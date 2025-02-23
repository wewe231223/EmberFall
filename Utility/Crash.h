#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Crash.h
// 2025 - 01 - 05 김성준 - Release, Debug 모드에서 디버깅 용으로 사용할 매크로 정의
//     별다른 일은 하지 않으며 의도적으로 잘못된 메모리 접근을 일으켜 프로그램을 강제 종료함
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <cassert>

#pragma region MACRO_CRASH
#define CrashExpRelease(expression, cause)  \
{                                           \
    if (false == (expression)) {            \
        int* p = nullptr;                   \
        __analysis_assume(p != nullptr);    \
		*p = 0;                             \
    }                                       \
}

#define CrashRelease(cause)                 \
{                                           \
    int* p = nullptr;                       \
    __analysis_assume(p != nullptr);        \
    *p = 0;                                 \
}

#if defined(_DEBUG) || defined(DEBUG)
#define Crash(cause) assert(false && cause)
#define CrashExp(expression, cause) if (false == (expression)) assert(false && cause)
#else
#define Crash(cause) CrashRelease((cause))
#define CrashExp(expression, cause) CrashExpRelease(expression, cause)
#endif

#pragma endregion
