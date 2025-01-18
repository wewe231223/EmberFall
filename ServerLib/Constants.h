#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Constant.h
// 2025 - 01 - 14 (설명 추가 날짜)
//      김성준: 여러 상수, 전역 객체 선언
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
extern std::unique_ptr<class PacketHandler> gPacketHandler;
extern std::unique_ptr<class SendBufferFactory> gSendBufferFactory;