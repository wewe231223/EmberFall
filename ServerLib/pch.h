#pragma once

// Windows && C headers
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")

#include <Windows.h>

#undef max
#undef min

// I/O header
#include <iostream>
#include <fstream>

// container
#include <array>
#include <vector>
#include <unordered_map>

// utils
#include <chrono>
#include <variant>
#include <concepts>
#include <ranges>
#include <numeric>
#include <algorithm>

// concurrency support
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

// concurrency container
#include <concurrent_unordered_map.h>
#include <concurrent_queue.h>

#include "../External/Include/DirectXTK12/SimpleMath.h"

using namespace std::literals;
namespace SimpleMath = DirectX::SimpleMath;

#include "Types.h"
#include "Constants.h"
#include "OverlappedEx.h"
#include "Crash.h"
#include "Lock.h"
#include "Protocol.h"
#include "NetworkUtils.h"