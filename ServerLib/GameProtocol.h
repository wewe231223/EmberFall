#pragma once

namespace GameProtocol {
    namespace Unit {
        decltype(auto) PLAYER_WALK_SPEED = 3.3mps;
        decltype(auto) PLAYER_RUN_SPEED = 5.5mps;

        decltype(auto) BOSS_PLAYER_WALK_SPEED = 5.5mps;
    }

    namespace Logic {
        constexpr uint8_t MAX_ITEM = 3;
        constexpr float PLAYER_HP = 100.0f;
    }

    namespace Map {
        decltype(auto) STAGE1_MAP_WIDTH = 1000.0m;
        decltype(auto) STAGE1_MAP_HEIGHT = 1000.0m;

        decltype(auto) STAGE2_MAP_WIDTH = 500.0m;
        decltype(auto) STAGE2_MAP_HEIGHT = 500.0m;

        decltype(auto) STAGE3_MAP_DIAMETER = 500.0m;
    }
}