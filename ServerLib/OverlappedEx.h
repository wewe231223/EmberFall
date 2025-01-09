#pragma once

#include "INetworkObject.h"

struct OverlappedEx : public WSAOVERLAPPED {
    NetworkBuf<BUF_RW_SIZE> buffer;
    WSABUF wsaBuf;
    IOType type;
    std::weak_ptr<INetworkObject> mOwner;

    // method
    OverlappedEx();
    OverlappedEx(char* data, size_t len);
    OverlappedEx(const std::span<char>& span);
    OverlappedEx(char* packet);

    OverlappedEx(const OverlappedEx&) = delete;
    OverlappedEx(OverlappedEx&&) noexcept = delete;
    OverlappedEx& operator=(const OverlappedEx&) = delete;
    OverlappedEx& operator=(OverlappedEx&&) noexcept = delete;

    WSAOVERLAPPED* GetRawPtr();

    void ResetOverlapped();
};

struct OverlappedAccept : public OverlappedEx { };
struct OverlappedConnect : public OverlappedEx { };
struct OverlappedRecv : public OverlappedEx { };
struct OverlappedSend : public OverlappedEx { };
