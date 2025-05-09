#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Crash.h
// 2025 - 01 - 05 김성준 - Release, Debug 모드에서 디버깅 용으로 사용할 매크로 정의
//     별다른 일은 하지 않으며 의도적으로 잘못된 메모리 접근을 일으켜 프로그램을 강제 종료함
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef Crash 
#pragma region MACRO_CRASH
#include <Windows.h>
#include <string>
#include <cassert>
#include <source_location>

inline void CrashMessageBox(const char* cause, const std::source_location& location = std::source_location::current()) {
    std::string message = "Crash Occurred!\n";
    message += "Cause: ";       message += cause; message += "\n";
    message += "File: ";        message += location.file_name(); message += "\n";
    message += "Function: ";    message += location.function_name(); message += "\n";
    message += "Line: ";        message += std::to_string(location.line());
    MessageBoxA(nullptr, message.c_str(), "Crash Handler", MB_OK | MB_ICONERROR);
}

inline void CheckCrash(bool condition, const std::source_location& location = std::source_location::current()) {
    if (!condition) {
        CrashMessageBox("Condition failed", location);
        int* p = nullptr;
        __analysis_assume(p != nullptr);
        *p = 0;
    }
}

inline void CheckCrashMsg(bool condition, const char* cause, const std::source_location& location = std::source_location::current()) {
    if (!condition) {
        CrashMessageBox(cause, location);
        int* p = nullptr;
        __analysis_assume(p != nullptr);
        *p = 0;
    }
}

#define CrashRelease(condition) CheckCrash((condition))
#define CrashExpRelease(expression, cause) CheckCrashMsg((expression), (cause))

#if defined(_DEBUG) || defined(DEBUG)
#define Crash(condition) assert((condition) && "Crash")
#define CrashExp(expression, cause) if (!(expression)) assert(false && cause)
#else
#define Crash(condition) CrashRelease((condition))
#define CrashExp(expression, cause) CrashExpRelease((expression), (cause))
#endif

#endif
#pragma endregion
