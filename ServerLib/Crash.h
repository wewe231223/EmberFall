#pragma once

#pragma region MACRO_CRASH
#define CrashExp(expression, cause)         \
{                                           \
    if (expression) {                       \
        int* p = nullptr;                   \
        __analysis_assume(p != nullptr);    \
        * p = 0;                            \
    }                                       \
}

#define Crash(cause)                        \
{                                           \
    int* p = nullptr;                       \
    __analysis_assume(p != nullptr);        \
    *p = 0;                                 \
}

#pragma endregion