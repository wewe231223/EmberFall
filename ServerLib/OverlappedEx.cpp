#include "pch.h"
#include "OverlappedEx.h"

OverlappedEx::OverlappedEx()
    : buffer{ }, wsaBuf{ 0, nullptr }, type{ IOType::RECV } {
    ResetOverlapped();
}

OverlappedEx::OverlappedEx(char* data, size_t len)
    : buffer{ }, wsaBuf{ static_cast<UINT32>(len), buffer.data() }, type{ IOType::SEND } {
    ResetOverlapped();
    ::memcpy(buffer.data(), data, len);
}

OverlappedEx::OverlappedEx(const std::span<char>& span)
    : buffer{ }, wsaBuf{ static_cast<UINT32>(span.size()), buffer.data() }, type{ IOType::SEND } {
    ResetOverlapped();
    ::memcpy(buffer.data(), span.data(), span.size());
}

OverlappedEx::OverlappedEx(char* packet)
    : buffer{ }, wsaBuf{ static_cast<UINT32>(packet[0]), buffer.data() }, type{ IOType::SEND} {
    ResetOverlapped();
    ::memcpy(buffer.data(), packet, static_cast<size_t>(packet[0]));
}

WSAOVERLAPPED* OverlappedEx::GetRawPtr() {
    return static_cast<WSAOVERLAPPED*>(this);
}

void OverlappedEx::ResetOverlapped() {
    ::memset(this, 0, sizeof(OVERLAPPED));
}
