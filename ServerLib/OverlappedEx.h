#pragma once

#include "INetworkObject.h"

struct OverlappedEx : public WSAOVERLAPPED {
    NetworkBuf<BUF_RW_SIZE> buffer;
    WSABUF wsaBuf;
    IOType type;
    std::shared_ptr<INetworkObject> mOwner;

    // method
    OverlappedEx(IOType type);
    OverlappedEx(IOType type, char* data, size_t len);
    OverlappedEx(IOType type, const std::span<char>& span);
    OverlappedEx(IOType type, char* packet);

    OverlappedEx(const OverlappedEx&) = delete;
    OverlappedEx(OverlappedEx&&) noexcept = delete;
    OverlappedEx& operator=(const OverlappedEx&) = delete;
    OverlappedEx& operator=(OverlappedEx&&) noexcept = delete;

    WSAOVERLAPPED* GetRawPtr();

    void ResetOverlapped();
};

struct OverlappedAccept : public OverlappedEx {  
    OverlappedAccept();

    OverlappedAccept(const OverlappedAccept&) = delete;
    OverlappedAccept(OverlappedAccept&&) noexcept = delete;
    OverlappedAccept& operator=(const OverlappedAccept&) = delete;
    OverlappedAccept& operator=(OverlappedAccept&&) noexcept = delete;
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
    OverlappedSend();
    OverlappedSend(char* data, size_t len);
    OverlappedSend(const std::span<char>& span);
    OverlappedSend(char* packet);

    OverlappedSend(const OverlappedSend&) = delete;
    OverlappedSend(OverlappedSend&&) noexcept = delete;
    OverlappedSend& operator=(const OverlappedSend&) = delete;
    OverlappedSend& operator=(OverlappedSend&&) noexcept = delete;
};
