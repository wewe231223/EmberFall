#pragma once

inline std::atomic_int32_t gThreadId{ };
inline thread_local int32_t lThreadId{ };

void InitTls();
void ClearTls();