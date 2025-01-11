#include "pch.h"
#include "Session.h"

Session::Session() {
    mSocket = NetworkUtil::CreateSocket();
    CrashExp(mSocket == INVALID_SOCKET, "");

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
    // TODO
}

void Session::ProcessSend() {
    // TODO
}

SessionState Session::GetCurrentState() const {
    return mState;
}

void Session::InitSessionNetAddress(char* addressBuffer) {
    sockaddr_in* localAddr{ nullptr };
    sockaddr_in* remoteAddr{ nullptr };
    int localAddrLen = 0;
    int remoteAddrLen = 0;

    GetAcceptExSockaddrs(
        addressBuffer,
        0,
        sizeof(sockaddr_in) + 16,
        sizeof(sockaddr_in) + 16,
        reinterpret_cast<sockaddr**>(&localAddr),
        &localAddrLen,
        reinterpret_cast<sockaddr**>(&remoteAddr),
        &remoteAddrLen
    );

    mIP.resize(INET_ADDRSTRLEN);
    ::inet_ntop(AF_INET, &remoteAddr->sin_addr, mIP.data(), INET_ADDRSTRLEN);
    mPort = ::ntohs(remoteAddr->sin_port);
}

std::pair<std::string, UINT16> Session::GetAddress() const {
    return std::make_pair(mIP, mPort);
}

void Session::lock() {
    mLock.lock();
}

void Session::unlock() {
    mLock.unlock();
}
