#pragma once

inline constexpr SessionIdType INVALID_CLIENT_ID = std::numeric_limits<SessionIdType>::max();

inline constexpr size_t MAX_BUF_SIZE = std::numeric_limits<unsigned short>::max();
inline constexpr size_t BUF_RW_SIZE = 1024;
inline const UINT32 HARDWARE_CONCURRENCY = std::thread::hardware_concurrency();

enum class IOType : UINT32 {
    SEND,
    RECV,
    CONNECT,
    DISCONNECT,
    ACCEPT
};

extern std::unique_ptr<class IOCPCore> gIocpCore;
extern std::unique_ptr<class SessionManager> gSessionManager;