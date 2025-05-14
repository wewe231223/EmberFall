#pragma once

// Windows && C headers
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")

#include <Windows.h>


#define DEV_MODE // 디버그 모드 설정 여부
//#undef DEV_MODE

#ifdef DEV_MODE
#define PRINT_DEBUG_LOG // 디버깅용 로그 출력여부 설정
#undef PRINT_DEBUG_LOG
#endif

#define DEF_DEBUG_OR_DEV (defined(DEBUG) || defined(DEV_MODE) || defined(_DEBUG))
#define DEF_DEBUG_OR_PRINT_LOG defined(DEBUG) || defined(PRINT_DEBUG_LOG) || defined(_DEBUG)

#undef max
#undef min

// I/O header
#include <iostream>
#include <fstream>
#include <format>

// container
#include <array>
#include <vector>
#include <unordered_map>

// utils
#include <string>
#include <chrono>
#include <variant>
#include <concepts>
#include <ranges>
#include <numeric>
#include <algorithm>
#include <functional>

#include <source_location>

// concurrency support
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

// concurrency container
#include <concurrent_unordered_map.h>
#include <concurrent_unordered_set.h>
#include <concurrent_queue.h>
#include <concurrent_priority_queue.h>

#include "../Utility/Crash.h"
#include "../External/Include/DirectXTK12/SimpleMath.h"

namespace SimpleMath = DirectX::SimpleMath;

#include "LogConsole.h"

#include "Types.h"
#include "Constants.h"
#include "OverlappedEx.h"
#include "Lock.h"
#include "Protocol.h"
#include "NetworkUtils.h"

#include "GameUnits.h"
#include "GameMath.h"
#include "FbsPacketFactory.h"
#include "NetworkCore.h"

using namespace GameUnitLiterals;
using namespace std::literals;

extern std::shared_ptr<class ClientCore> gClientCore;
extern std::shared_ptr<class ServerCore> gServerCore;