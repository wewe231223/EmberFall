#pragma once

// Cpp Util
#include <filesystem>

#include <random>

// Cpp Container
#include <map>
#include <deque>
#include <queue>
#include <unordered_set>

// Server Library
#include "../ServerLib/pch.h"
#include "../ServerLib/NetworkCore.h"

#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "../out/Debug/ServerLib.lib")
#else
#pragma comment(lib, "../out/Release/ServerLib.lib")
#endif

// Util & Global Include
#include "GameRandom.h"
#include "GameTimer.h"
#include "GameEventFactory.h"

#include "../ServerLib/GameProtocol.h"

// Global Instance
extern std::unique_ptr<class ServerFrame> gServerFrame;

extern std::unique_ptr<class GameRoomManager> gGameRoomManager;
extern std::unique_ptr<class SectorSystem> gSectorSystem;
extern std::unique_ptr<class ObjectManager> gObjectManager;
extern std::unique_ptr<class CollisionManager> gCollisionManager;

enum class GameStage : uint8_t {
    LOBBY,
    STAGE1,
    STAGE2,
    STAGE3,
};

// Add std::hash for std::pair
namespace std {
    template <typename T1, typename T2>
    struct hash<std::pair<T1, T2>> {
        std::size_t operator()(const std::pair<T1, T2>& p) const noexcept {
            std::size_t h1 = std::hash<T1>{}(p.first);
            std::size_t h2 = std::hash<T2>{}(p.second);
            return h1 ^ (h2 << 1); // boost::hash_combine
        }
    };
}