#pragma once

namespace GameProtocol {
    namespace Unit {
        inline decltype(auto) MONSTER_WALK_SPEED = 1.5mps;
        inline decltype(auto) PLAYER_WALK_SPEED = 2.3mps;
        inline decltype(auto) PLAYER_RUN_SPEED = 4.0mps;

        inline decltype(auto) BOSS_PLAYER_WALK_SPEED = 3.3mps;
        inline decltype(auto) BOSS_PLAYER_RUN_SPEED = 5.5mps;

        inline decltype(auto) DEFAULT_PROJECTILE_SPEED = 33.3mps;
        inline decltype(auto) ARROW_SPEED = 30.0mps;
    }

    namespace Key {
        inline std::array<std::pair<char, SimpleMath::Vector3>, 4> KEY_MOVE_DIR{
            std::pair<char, SimpleMath::Vector3>{ 'W', { SimpleMath::Vector3::Forward } },
            std::pair<char, SimpleMath::Vector3>{ 'A', { SimpleMath::Vector3::Right } },
            std::pair<char, SimpleMath::Vector3>{ 'S', { SimpleMath::Vector3::Backward } },
            std::pair<char, SimpleMath::Vector3>{ 'D', { SimpleMath::Vector3::Left } },
        };
    }

    namespace Logic {
        inline constexpr uint8_t MAX_ITEM = 3;
        inline constexpr float MAX_HP = 100.0f;

        inline constexpr float DEFAULT_DAMAGE = 10.0f;
        //inline constexpr float 
    }

    namespace Map {
        inline decltype(auto) STAGE1_MAP_WIDTH = 1000.0m;
        inline decltype(auto) STAGE1_MAP_HEIGHT = 1000.0m;

        inline decltype(auto) STAGE2_MAP_WIDTH = 500.0m;
        inline decltype(auto) STAGE2_MAP_HEIGHT = 500.0m;

        inline decltype(auto) STAGE3_MAP_DIAMETER = 500.0m;
    }

    enum EnvironmentType : UINT {
        Tree1 = 0,
        Tree2 = 1,
        Tree3 = 2,
        Rock1 = 3,
        Rock2 = 4,
        Rock3 = 5,
        Rock4 = 6,
        LargeRock1 = 7,
        LargeRock2 = 8,
        Fern = 9,
        Mountain1 = 10,
        Mountain2 = 11,
        TimberHouse = 12,
        StoneHouse = 13,
        LogHouse = 14,
        LogHouseDoor = 15,
        WindMill = 16,
        WindMillBlade = 17,
        Well = 18,
    };
}