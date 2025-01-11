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
    : OverlappedEx{ IOType::SEND }, buffer{ } { }

OverlappedSend::OverlappedSend(char* data, size_t len)
    : OverlappedEx{ IOType::SEND }, buffer{ } {
    ResetOverlapped();
    ::memcpy(buffer.data(), data, len);
    wsaBuf.buf = buffer.data();
    wsaBuf.len = static_cast<ULONG>(len);
}

OverlappedSend::OverlappedSend(const std::span<char>& span)
    : OverlappedEx{ IOType::SEND }, buffer{ } {
    ResetOverlapped();
    ::memcpy(buffer.data(), span.data(), span.size());
    wsaBuf.buf = buffer.data();
    wsaBuf.len = static_cast<ULONG>(span.size());
}

OverlappedSend::OverlappedSend(char* packet)
    : OverlappedEx{ IOType::SEND }, buffer{ } {
    ResetOverlapped();
    ::memcpy(buffer.data(), packet, static_cast<size_t>(packet[0]));
    wsaBuf.buf = buffer.data();
    wsaBuf.len = packet[0];
}