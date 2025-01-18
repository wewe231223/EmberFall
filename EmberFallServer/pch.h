#pragma once

#include "../ServerLib/pch.h"
#include "../ServerLib/ServerCore.h"
#include "../ServerLib/PacketHandler.h"
#include "../ServerLib/SessionManager.h"

#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "../out/debug/ServerLib.lib")
#else
#pragma comment(lib, "../out/Release/ServerLib.lib")
#endif