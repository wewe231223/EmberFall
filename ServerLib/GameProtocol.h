#pragma once

namespace GameProtocol {
    namespace Unit {

        decltype(auto) GAME_MAP_WIDTH = 1000.0m;
        decltype(auto) GAME_MAP_HEIGHT = 1000.0m;

        decltype(auto) PLAYER_WALK_SPEED = 3.3mps;
        decltype(auto) PLAYER_RUN_SPEED = 5.5mps;
    }

    namespace Logic {
        constexpr uint8_t MAX_ITEM = 3;
        constexpr float DEFAULT_PLAYER_HP = 100.0f;
        constexpr float DEFAULT_PLAYER_MP = 100.0f;
    }
}