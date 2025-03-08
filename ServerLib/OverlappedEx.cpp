#include "pch.h"
#include "OverlappedEx.h"
#include "Session.h"

OverlappedEx::OverlappedEx(IOType type)
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
    : OverlappedEx{ IOType::ACCEPT }, buffer{ } { }

std::shared_ptr<Session> OverlappedAccept::GetSession() {
    return std::static_pointer_cast<Session>(session);
}

OverlappedConnect::OverlappedConnect()
    : OverlappedEx{ IOType::CONNECT } { }

OverlappedRecv::OverlappedRecv()
    :OverlappedEx{ IOType::RECV }, buffer{ } { 
    wsaBuf.buf = buffer.data();
    wsaBuf.len = static_cast<ULONG>(buffer.size());
}

OverlappedSend::OverlappedSend()
    : OverlappedEx{ IOType::SEND } { }

OverlappedDisconnect::OverlappedDisconnect()
    : OverlappedEx{ IOType::DISCONNECT } { }
