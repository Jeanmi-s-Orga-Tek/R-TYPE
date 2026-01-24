/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** PlayerInfo
*/

#ifndef PLAYERINFO_HPP_
#define PLAYERINFO_HPP_

#include "Utils.hpp"

namespace Engine {
    namespace Components {
        typedef struct PlayerInfo_s {
            uint32_t player_id;
            int health;
            int max_health;
            int score;
        } PlayerInfo;
    };
};

#endif /* !PLAYERINFO_HPP_ */
