#pragma once

#include "BT_Decider.h"
#include "BT_MonsterAttack.h"
#include "BT_MonsterChase.h"
#include "BT_MonsterRandomMove.h"

using BT_Monster = BT::BT_Decider<BT::BT_MonsterRandomMove, BT::BT_MonsterChase>;