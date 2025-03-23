#pragma once

namespace GameProtocol {
    namespace Unit {
        inline decltype(auto) PLAYER_WALK_SPEED = 3.3mps;
        inline decltype(auto) PLAYER_RUN_SPEED = 5.5mps;

        inline decltype(auto) BOSS_PLAYER_WALK_SPEED = 5.5mps;

        inline decltype(auto) DEFAULT_PROJECTILE_SPEED = 33.3mps;
    }

    namespace Logic {
        inline constexpr uint8_t MAX_ITEM = 3;
        inline constexpr float PLAYER_HP = 100.0f;

        inline constexpr float DEFAULT_DAMAGE = 10.0f;
        inline constexpr float DEFAULT_HEALTH = 100.0f;
        //inline constexpr float 
    }

    namespace Map {
        inline decltype(auto) STAGE1_MAP_WIDTH = 1000.0m;
        inline decltype(auto) STAGE1_MAP_HEIGHT = 1000.0m;

        inline decltype(auto) STAGE2_MAP_WIDTH = 500.0m;
        inline decltype(auto) STAGE2_MAP_HEIGHT = 500.0m;

        inline decltype(auto) STAGE3_MAP_DIAMETER = 500.0m;
    }
}