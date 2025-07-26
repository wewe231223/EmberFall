#pragma once

inline constexpr auto CHECK_SESSION_HEART_BEAT_DELAY = 50s;

namespace GameProtocol {
    namespace Unit {
        inline decltype(auto) MONSTER_WALK_SPEED = 1.5mps;
        inline decltype(auto) PLAYER_WALK_SPEED = 2.3mps;
        inline decltype(auto) PLAYER_RUN_SPEED = 5.0mps;

        inline decltype(auto) BOSS_PLAYER_WALK_SPEED = 3.3mps;
        inline decltype(auto) BOSS_PLAYER_RUN_SPEED = 7.0mps;

        inline decltype(auto) DEFAULT_PROJECTILE_SPEED = 33.3mps;
        inline decltype(auto) ARROW_SPEED = 15.0mps;
    }

    namespace Key {
        inline constexpr char KEY_MOVE_FORWARD = 'W';
        inline constexpr char KEY_MOVE_LEFT = 'A';
        inline constexpr char KEY_MOVE_BACKWARD = 'S';
        inline constexpr char KEY_MOVE_RIGHT = 'D';
        inline constexpr char KEY_INTERACTION = 'F';
        inline constexpr char KEY_USE_ITEM = 'R';

        inline constexpr char KEY_CHANGE_TARGET_ITEM = VK_TAB;
        inline constexpr char KEY_JUMP = VK_SPACE;
        inline constexpr char KEY_ATTACK = VK_LBUTTON;

        inline std::array<std::pair<char, SimpleMath::Vector3>, 4> KEY_MOVE_DIR{
            std::pair<char, SimpleMath::Vector3>{ KEY_MOVE_FORWARD, { SimpleMath::Vector3::Forward } },
            std::pair<char, SimpleMath::Vector3>{ KEY_MOVE_LEFT, { SimpleMath::Vector3::Right } },
            std::pair<char, SimpleMath::Vector3>{ KEY_MOVE_BACKWARD, { SimpleMath::Vector3::Backward } },
            std::pair<char, SimpleMath::Vector3>{ KEY_MOVE_RIGHT, { SimpleMath::Vector3::Left } },
        };
    }

    namespace Logic {
        inline constexpr auto GAME_ROOM_CHECK_GAME_END_DELAY = 1s;
        inline constexpr auto MONSTER_UPDATE_DELAY = 100ms;
        inline constexpr auto ITEM_LIFE_TIME = 15s;
        inline constexpr auto ITEM_SPAWN_DELAY = 5s;
        inline constexpr auto PROJECTILE_LIFE_TIME = 7s;

        inline constexpr float PLAYER_VIEW_RANGE = 100.0f;
        inline constexpr float SECTOR_SIZE = 200.0f;

        inline constexpr uint8_t MAX_ITEM = 3;
        inline constexpr float MAX_HP = 100.0f;

        // Attack
        inline constexpr float DEFAULT_DAMAGE = 10.0f;

        inline constexpr float MONSTER_KNOCK_BACK_POWER = (15000.0N).Count();
        inline constexpr float LONGSWORD_KNOCK_BACK_POWER = (5000.0N).Count();
        inline constexpr float SWORD_KNOCK_BACK_POWER = (3000.0N).Count();
        inline constexpr float BOSS_SWORD_KNOCK_BACK_POWER = (10000.0N).Count();

        inline const SimpleMath::Vector3 PLAYER_TRIGGER_SIZE{ 1.0f, 3.0f, 1.0f };
       
        // Spawn
        inline constexpr auto MONSTER_SPAWN_COUNT = 200;
        inline constexpr auto TEST_ITEM_SPAWN_COUNT = 10;

        inline const std::pair<SimpleMath::Vector3, SimpleMath::Vector3> PLAYER_SPAWN_AREA{
            SimpleMath::Vector3{ -10.0f, 0.0f, -10.0f },
            SimpleMath::Vector3{ 10.0f, 0.0f, 10.0f }
        };

        inline const std::pair<SimpleMath::Vector3, SimpleMath::Vector3> ITEM_SPAWN_AREA{
            SimpleMath::Vector3{ -10.0f, 0.0f, -10.0f },
            SimpleMath::Vector3{ 10.0f, 0.0f, 10.0f }
        };

        inline const std::pair<SimpleMath::Vector3, SimpleMath::Vector3> MONSTER_SPAWN_AREA{
            SimpleMath::Vector3{ -200.0f, 0.0f, -200.0f },
            SimpleMath::Vector3{ 200.0f, 0.0f, 200.0f }
        };

        inline const std::pair<SimpleMath::Vector3, SimpleMath::Vector3> GEM_SPAWN_AREA{
            SimpleMath::Vector3{ -50.0f, 0.0f, -50.0f },
            SimpleMath::Vector3{ 50.0f, 0.0f, 50.0f }
        };

        inline const std::pair<SimpleMath::Vector3, SimpleMath::Vector3> MONSTER_PATROL_AREA{
            SimpleMath::Vector3{ -10.0f, 0.0f, -10.0f },
            SimpleMath::Vector3{ 10.0f, 0.0f, 10.0f }
        };

        inline const SimpleMath::Vector3 MAGIC_ARROW_EXPLOSION_EXTENTS{
            SimpleMath::Vector3{ 2.5f, 100.0f, 2.5f },
        };
    }

    namespace Map {
        inline decltype(auto) STAGE1_MAP_WIDTH = 1000.0m;
        inline decltype(auto) STAGE1_MAP_HEIGHT = 1000.0m;

        inline decltype(auto) STAGE2_MAP_WIDTH = 500.0m;
        inline decltype(auto) STAGE2_MAP_HEIGHT = 500.0m;

        inline decltype(auto) STAGE3_MAP_DIAMETER = 500.0m;

        inline std::array<std::pair<SimpleMath::Vector3, SimpleMath::Vector3>, Packets::GameStage_MAX - 1> STAGE_MAP_SIZE{
            std::pair<SimpleMath::Vector3, SimpleMath::Vector3>{ { -240.f, -1000.0f, -240.f }, {240.f, 1000.0f, 240.f} },
            std::pair<SimpleMath::Vector3, SimpleMath::Vector3>{ { -240.f, -1000.0f, -240.f }, {240.f, 1000.0f, 240.f} },
            std::pair<SimpleMath::Vector3, SimpleMath::Vector3>{ { -25.f, -1000.0f, -25.f }, {25.f, 1000.0f, 25.f} }
        }; 

        inline constexpr const char* BASE_TERRAIN_PATH = "../Resources/Binarys/Terrain/NTerrain.bin";
        inline constexpr const char* LAST_STAGE_TERRAIN_PATH = "";

        inline const std::pair<SimpleMath::Vector3, SimpleMath::Vector3>& GetStageMapSize(Packets::GameStage stage)
        {
            if (Packets::GameStage_LOBBY > stage or Packets::GameStage_MAX < stage) {
                return { };
            }

            return STAGE_MAP_SIZE.at(static_cast<size_t>(stage) - 2);
        }
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


    enum class EnvironmentType1 : UINT {
        Tree1 = 0,
        Tree2 = 1,
        Tree3 = 2, 
        Tree4 = 3,
		Tree5 = 4,
        Tree6 = 5,
        Tree7 = 6, 
        Tree8 = 7,
        SRock1 = 8,
        Fern1 = 9,
        LogHouse = 10,
        TimberHouse = 11, 
        StoneHouse = 12, 
		Cliff1 = 13,
        Cliff2 = 14,
        Cliff3 = 15,
		Tower = 16,
        Wall = 17,
		Wall1 = 18,
		DoorR = 19,
        DoorL = 20, 

    };
}