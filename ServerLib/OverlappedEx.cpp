#include "pch.h"
#include "OverlappedEx.h"
#include "Session.h"

OverlappedEx::OverlappedEx() 
    : wsaBuf{ 0, nullptr }, type{ } { 
    ResetOverlapped();
}

OverlappedEx::OverlappedEx(IoType type)
    : wsaBuf{ 0, nullptr }, type{ type } {
    ResetOverlapped();
}

WSAOVERLAPPED* OverlappedEx::GetRawPtr() {
    return static_cast<WSAOVERLAPPED*>(this);
}

void OverlappedEx::ResetOverlapped() {
    ::memset(this, 0, sizeof(OVERLAPPED));
}

OverlappedAccept::OverlappedAccept()
    : OverlappedEx{ IoType::ACCEPT }, buffer{ } { }

OverlappedConnect::OverlappedConnect()
    : OverlappedEx{ IoType::CONNECT } { }

OverlappedRecv::OverlappedRecv()
    :OverlappedEx{ IoType::RECV }, buffer{ } { 
    wsaBuf.buf = buffer.data();
    wsaBuf.len = static_cast<ULONG>(buffer.size());
}

OverlappedSend::OverlappedSend()
    : OverlappedEx{ IoType::SEND } { }

OverlappedDisconnect::OverlappedDisconnect()
    : OverlappedEx{ IoType::DISCONNECT } { }