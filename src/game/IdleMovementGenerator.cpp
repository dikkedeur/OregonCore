/*
 * This file is part of the OregonCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "IdleMovementGenerator.h"
#include "CreatureAI.h"
#include "Creature.h"

IdleMovementGenerator si_idleMovement;

// StopMoving is needed to make unit stop if its last movement generator expires
// But it should not be sent otherwise there are many redundent packets
void IdleMovementGenerator::Initialize(Unit& owner)
{
    if (owner.HasUnitState(UNIT_STATE_MOVE))
        owner.StopMoving();
}

void
IdleMovementGenerator::Reset(Unit& owner)
{
    if (owner.HasUnitState(UNIT_STATE_MOVE))
        owner.StopMoving();
}

void RotateMovementGenerator::Initialize(Unit& owner)
{
    if (owner.HasUnitState(UNIT_STATE_MOVE))
        owner.StopMoving();

    if (owner.getVictim())
        owner.SetInFront(owner.getVictim());

    owner.AddUnitState(UNIT_STATE_ROTATING);

    owner.AttackStop();
}

bool RotateMovementGenerator::Update(Unit& owner, const uint32& diff)
{
    float angle = owner.GetOrientation();
    if (m_direction == ROTATE_DIRECTION_LEFT)
    {
        angle += (float)diff * M_PI * 2 / m_maxDuration;
        while (angle >= M_PI * 2) angle -= M_PI * 2;
    }
    else
    {
        angle -= (float)diff * M_PI * 2 / m_maxDuration;
        while (angle < 0) angle += M_PI * 2;
    }

    owner.SetFacingTo(angle);

    if (m_duration > diff)
        m_duration -= diff;
    else
        return false;

    return true;
}

void RotateMovementGenerator::Finalize(Unit& unit)
{
    unit.ClearUnitState(UNIT_STATE_ROTATING);
    if (unit.GetTypeId() == TYPEID_UNIT)
        unit.ToCreature()->AI()->MovementInform(ROTATE_MOTION_TYPE, 0);
}

void
DistractMovementGenerator::Initialize(Unit& owner)
{
    // Distracted creatures stand up if not standing
    if (!owner.IsStandState())
        owner.SetStandState(UNIT_STAND_STATE_STAND);

    owner.AddUnitState(UNIT_STATE_DISTRACTED);
}

void
DistractMovementGenerator::Finalize(Unit& owner)
{
    owner.ClearUnitState(UNIT_STATE_DISTRACTED);

    // If this is a creature, then return orientation to original position (for idle movement creatures)
    if (owner.GetTypeId() == TYPEID_UNIT && owner.ToCreature())
    {
        float angle = owner.ToCreature()->GetHomePosition().GetOrientation();
        owner.SetFacingTo(angle);
    }
}

bool
DistractMovementGenerator::Update(Unit& /*owner*/, const uint32& time_diff)
{
    if (time_diff > m_timer)
        return false;

    m_timer -= time_diff;
    return true;
}

void
AssistanceDistractMovementGenerator::Finalize(Unit& unit)
{
    unit.ClearUnitState(UNIT_STATE_DISTRACTED);
    unit.ToCreature()->SetReactState(REACT_AGGRESSIVE);
}

