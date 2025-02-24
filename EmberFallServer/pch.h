#pragma once

#include <filesystem>

#include <random>
#include <map>
#include <deque>

#include <queue>

#include "../ServerLib/pch.h"
#include "../ServerLib/NetworkCore.h"

#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "../out/Debug/ServerLib.lib")
#else
#pragma comment(lib, "../out/Release/ServerLib.lib")
#endif

#include "Events.h"
#include "GameRandom.h"

using namespace GameUnitLiterals;

extern std::shared_ptr<class GameEventManager> gEventManager;