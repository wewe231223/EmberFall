#pragma once

#include "INetworkObject.h"

struct OverlappedEx : public WSAOVERLAPPED {
    WSABUF wsaBuf;
    IOType type;
    std::shared_ptr<INetworkObject> owner;

    // method
    OverlappedEx(IOType type);

    OverlappedEx(const OverlappedEx&) = delete;
    OverlappedEx(OverlappedEx&&) noexcept = delete;
    OverlappedEx& operator=(const OverlappedEx&) = delete;
    OverlappedEx& operator=(OverlappedEx&&) noexcept = delete;

    WSAOVERLAPPED* GetRawPtr();

    void ResetOverlapped();
};

inline constexpr size_t ADDR_BUF_SIZE = (sizeof(sockaddr_in) + 16) * 2;
struct OverlappedAccept : public OverlappedEx {
    NetworkBuf<ADDR_BUF_SIZE> buffer;
    std::shared_ptr<INetworkObject> session;

    OverlappedAccept();

    OverlappedAccept(const OverlappedAccept&) = delete;
    OverlappedAccept(OverlappedAccept&&) noexcept = delete;
    OverlappedAccept& operator=(const OverlappedAccept&) = delete;
    OverlappedAccept& operator=(OverlappedAccept&&) noexcept = delete;

    std::shared_ptr<class Session> GetSession();
};

struct OverlappedConnect : public OverlappedEx {
    OverlappedConnect();

    OverlappedConnect(const OverlappedConnect&) = delete;
    OverlappedConnect(OverlappedConnect&&) noexcept = delete;
    OverlappedConnect& operator=(const OverlappedConnect&) = delete;
    OverlappedConnect& operator=(OverlappedConnect&&) noexcept = delete;
};

struct OverlappedRecv : public OverlappedEx {
    OverlappedRecv();

    OverlappedRecv(const OverlappedRecv&) = delete;
    OverlappedRecv(OverlappedRecv&&) noexcept = delete;
    OverlappedRecv& operator=(const OverlappedRecv&) = delete;
    OverlappedRecv& operator=(OverlappedRecv&&) noexcept = delete;
};

struct OverlappedSend : public OverlappedEx {
    NetworkBuf<BUF_RW_SIZE> buffer;

    OverlappedSend();
    OverlappedSend(char* data, size_t len);
    OverlappedSend(const std::span<char>& span);
    OverlappedSend(char* packet);

    OverlappedSend(const OverlappedSend&) = delete;
    OverlappedSend(OverlappedSend&&) noexcept = delete;
    OverlappedSend& operator=(const OverlappedSend&) = delete;
    OverlappedSend& operator=(OverlappedSend&&) noexcept = delete;
};
