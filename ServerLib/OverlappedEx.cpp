#include "pch.h"
#include "OverlappedEx.h"
#include "Session.h"

OverlappedEx::OverlappedEx(IOType type)
    : buffer{ }, wsaBuf{ 0, nullptr }, type{ type } {
    ResetOverlapped();
}

OverlappedEx::OverlappedEx(IOType type, char* data, size_t len)
    : buffer{ }, wsaBuf{ static_cast<UINT32>(len), buffer.data() }, type{ type } {
    ResetOverlapped();
    ::memcpy(buffer.data(), data, len);
}

OverlappedEx::OverlappedEx(IOType type, const std::span<char>& span)
    : buffer{ }, wsaBuf{ static_cast<UINT32>(span.size()), buffer.data() }, type{ type } {
    ResetOverlapped();
    ::memcpy(buffer.data(), span.data(), span.size());
}

OverlappedEx::OverlappedEx(IOType type, char* packet)
    : buffer{ }, wsaBuf{ static_cast<UINT32>(packet[0]), buffer.data() }, type{ type } {
    ResetOverlapped();
    ::memcpy(buffer.data(), packet, static_cast<size_t>(packet[0]));
}

WSAOVERLAPPED* OverlappedEx::GetRawPtr() {
    return static_cast<WSAOVERLAPPED*>(this);
}

void OverlappedEx::ResetOverlapped() {
    ::memset(this, 0, sizeof(OVERLAPPED));
}

OverlappedAccept::OverlappedAccept() 
    : OverlappedEx{ IOType::ACCEPT } { }

std::shared_ptr<Session> OverlappedAccept::GetSession() {
    return std::static_pointer_cast<Session>(session);
}

OverlappedConnect::OverlappedConnect() 
    : OverlappedEx{ IOType::CONNECT } { }

OverlappedRecv::OverlappedRecv() 
    :OverlappedEx{ IOType::RECV } { }

OverlappedSend::OverlappedSend() 
    : OverlappedEx{ IOType::SEND } { }

OverlappedSend::OverlappedSend(char* data, size_t len) 
    : OverlappedEx{ IOType::SEND, data, len } { }

OverlappedSend::OverlappedSend(const std::span<char>& span)
    : OverlappedEx{ IOType::SEND, span} { }

OverlappedSend::OverlappedSend(char* packet)
    : OverlappedEx{ IOType::SEND, packet } { }
