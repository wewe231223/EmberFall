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
#include "Events.h"
#include "GameRandom.h"

#include "../ServerLib/GameProtocol.h"

// Global Instance
extern std::unique_ptr<class ServerFrame> gServerFrame;
extern std::shared_ptr<class GameEventManager> gEventManager;
