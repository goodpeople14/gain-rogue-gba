#ifndef COMBAT_MELEE_SWORDSMAN_ATTACK_DATA_H
#define COMBAT_MELEE_SWORDSMAN_ATTACK_DATA_H

#include "combat/attack_frame_data.h"
#include "game/direction.h"

constexpr int swordsman_attack_game_frames = 6;

[[nodiscard]] const AttackFrameData& swordsman_attack_frame_data(Direction direction, int game_frame);

#endif
