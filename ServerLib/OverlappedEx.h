#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////
//
// OverlappedEx.h
// 2025 - 01 - 14 (설명 추가 날짜)
//      김성준: Overlapped 구조체 확장
// 
// 각 I/O 이벤트마다 다른 구조체 사용
// 
//      OverlappedConnect : Client
//      OverlappedAccept : Server
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "INetworkObject.h"

struct OverlappedEx : public WSAOVERLAPPED {
    // method
    OverlappedEx(IOType type);

    OverlappedEx(const OverlappedEx&) = delete;
    OverlappedEx(OverlappedEx&&) noexcept = delete;
    OverlappedEx& operator=(const OverlappedEx&) = delete;
    OverlappedEx& operator=(OverlappedEx&&) noexcept = delete;

    WSAOVERLAPPED* GetRawPtr();

    void ResetOverlapped();

public:
    WSABUF wsaBuf;
    IOType type;
    std::shared_ptr<INetworkObject> owner;
};

inline constexpr size_t ADDR_BUF_SIZE = (sizeof(sockaddr_in) + 16) * 2;
struct OverlappedAccept : public OverlappedEx {
    OverlappedAccept();

    OverlappedAccept(const OverlappedAccept&) = delete;
    OverlappedAccept(OverlappedAccept&&) noexcept = delete;
    OverlappedAccept& operator=(const OverlappedAccept&) = delete;
    OverlappedAccept& operator=(OverlappedAccept&&) noexcept = delete;  

    std::shared_ptr<class Session> GetSession();

public:
    NetworkBuf<ADDR_BUF_SIZE> buffer;
    std::shared_ptr<INetworkObject> session;
};

struct OverlappedConnect : public OverlappedEx {
    OverlappedConnect();

    OverlappedConnect(const OverlappedConnect&) = delete;
    OverlappedConnect(OverlappedConnect&&) noexcept = delete;
    OverlappedConnect& operator=(const OverlappedConnect&) = delete;
    OverlappedConnect& operator=(OverlappedConnect&&) noexcept = delete;
};

struct OverlappedDisconnect : public OverlappedEx {
    OverlappedDisconnect();

    OverlappedDisconnect(const OverlappedDisconnect&) = delete;
    OverlappedDisconnect(OverlappedDisconnect&&) noexcept = delete;
    OverlappedDisconnect& operator=(const OverlappedDisconnect&) = delete;
    OverlappedDisconnect& operator=(OverlappedDisconnect&&) noexcept = delete;
};

struct OverlappedRecv : public OverlappedEx {
    OverlappedRecv();

    OverlappedRecv(const OverlappedRecv&) = delete;
    OverlappedRecv(OverlappedRecv&&) noexcept = delete;
    OverlappedRecv& operator=(const OverlappedRecv&) = delete;
    OverlappedRecv& operator=(OverlappedRecv&&) noexcept = delete;

public:
    NetworkBuf<BUF_RW_SIZE> buffer;
};

struct OverlappedSend : public OverlappedEx {
    OverlappedSend();

    OverlappedSend(const OverlappedSend&) = delete;
    OverlappedSend(OverlappedSend&&) noexcept = delete;
    OverlappedSend& operator=(const OverlappedSend&) = delete;
    OverlappedSend& operator=(OverlappedSend&&) noexcept = delete;
};
