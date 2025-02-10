#pragma once

#include <filesystem>
#include <map>

#include "../ServerLib/pch.h"
#include "../ServerLib/NetworkCore.h"

#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "../out/Debug/ServerLib.lib")
#else
#pragma comment(lib, "../out/Release/ServerLib.lib")
#endif

#include "Events.h"