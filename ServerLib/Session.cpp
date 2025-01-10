#include "pch.h"
#include "Session.h"

Session::Session() {
    mSocket = NetworkUtil::CreateSocket();
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
    auto result = ::WSARecv(
        mSocket, 
        &mOverlappedRecv.wsaBuf,
        1, 
        &numOfTransffered, 
        &flag, 
        &mOverlappedRecv, 
        nullptr
    );

    if (SOCKET_ERROR == result) {
        auto errorCode = ::WSAGetLastError();
        if (WSA_IO_PENDING != errorCode) {
            RegisterRecv();
        }
    }
}

void Session::SetRemain(size_t remainSize) {
    mPrevRemainSize = remainSize;
}

void Session::RegisterSend(void* packet) {
    OverlappedSend* overlappedSend = new OverlappedSend{ reinterpret_cast<char*>(packet) };
    overlappedSend->owner = shared_from_this();
    auto result = ::WSASend(
        mSocket, 
        &overlappedSend->wsaBuf, 
        1, 
        0, 
        0, 
        overlappedSend, 
        nullptr
    );

    if (SOCKET_ERROR == result) {
        auto errorCode = ::WSAGetLastError();
        if (WSA_IO_PENDING != errorCode) {
            delete overlappedSend;
            RegisterSend(packet);
        }
    }
}

void Session::ProcessRecv() {
}

void Session::ProcessSend() {
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
