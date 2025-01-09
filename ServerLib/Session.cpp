#include "pch.h"
#include "Session.h"

Session::Session() {
    mSocket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, NULL, WSA_FLAG_OVERLAPPED);
    CrashExp(mSocket != INVALID_SOCKET, "");

    mState = SessionState::DISCONNECTED;
}

Session::~Session() {
    Close();
}

HANDLE Session::GetHandle() const {
    return reinterpret_cast<HANDLE>(mSocket);
}

void Session::ProcessOverlapped(OverlappedEx* overlapped, INT32 numOfBytes) {
    switch (overlapped->type) {
    case IOType::SEND:
        break;

    case IOType::RECV:
        break;

    case IOType::CONNECT:
        break;

    default:
        /* Print Error Log */
        break;
    }
}

void Session::Close() {
    if (INVALID_SOCKET == mSocket) {
        return;
    }

    ::shutdown(mSocket, SD_BOTH);
    ::closesocket(mSocket);
    mSocket = INVALID_SOCKET;
    mState = SessionState::CONNECTED;
}

void Session::RegisterRecv() {
    DWORD numOfTransffered{ };
    DWORD flag{ };
    ::WSARecv(mSocket, &mRecvOverlappedEx.wsaBuf, 1, &numOfTransffered, &flag, &mRecvOverlappedEx, nullptr);
}

void Session::SetRemain(size_t remainSize) {
    mPrevRemainSize = remainSize;
}

void Session::RegisterSend(void* packet) {
    OverlappedSend* overlappedSend = new OverlappedSend{ reinterpret_cast<char*>(packet) };
    overlappedSend->mOwner = shared_from_this();
    ::WSASend(mSocket, &overlappedSend->wsaBuf, 1, 0, 0, overlappedSend, nullptr);
}

SessionState Session::GetCurrentState() const {
    return mState;
}

void Session::lock() {
    mLock.lock();
}

void Session::unlock() {
    mLock.unlock();
}
