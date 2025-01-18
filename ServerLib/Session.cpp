#include "pch.h"
#include "Session.h"
#include "SessionManager.h"
#include "PacketHandler.h"

Session::Session() {
    mSocket = NetworkUtil::CreateSocket();
    CrashExp(INVALID_SOCKET == mSocket, "");
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
        delete overlapped;
        break;

    case IOType::RECV:
        ProcessRecv(numOfBytes);
        break;

    case IOType::CONNECT:
        break;

    default:
        /* Print Error Log */
        break;
    }
}

void Session::Close() {
    if (false == mConnected.exchange(false)) { // 원래 값이 false였다면 이미 다른 쓰레드가 이 함수에 접근한 것
        return; // 이경우 해당 쓰레드에서 별다른 작업을 할 필요는 없다.
    }

    ::closesocket(mSocket);
    mSocket = INVALID_SOCKET;
}

void Session::RegisterRecv() {
    if (false == mConnected.load()) {
        return;
    }

    DWORD receivedBytes{ };
    DWORD flag{ };

    mOverlappedRecv.ResetOverlapped();
    mOverlappedRecv.owner = shared_from_this();
    mOverlappedRecv.wsaBuf.buf = mOverlappedRecv.buffer.data();
    mOverlappedRecv.wsaBuf.len = static_cast<UINT32>(mOverlappedRecv.buffer.size());
    auto result = ::WSARecv(
        mSocket,
        &mOverlappedRecv.wsaBuf,
        1,
        &receivedBytes,
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
    if (false == mConnected.load()) {
        return;
    }

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

void Session::RegisterSend(void* data, size_t size) {
    if (false == mConnected.load()) {
        return;
    }

    OverlappedSend* overlappedSend = new OverlappedSend{ reinterpret_cast<char*>(data), size };
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
            RegisterSend(data, size);
        }
    }
}

void Session::ProcessRecv(INT32 numOfBytes) {
    // TODO
    std::cout << std::format("RECV Len: {}\n", numOfBytes);
    mOverlappedRecv.owner.reset();
    if (0 >= numOfBytes) {
        gSessionManager->CloseSession(GetId());
        return;
    }

    // Echo Test
    //RegisterSend(mOverlappedRecv.buffer.data(), numOfBytes);
    mOverlappedRecv.buffer[numOfBytes] = '\n'; // 출력 테스트용
    numOfBytes += 1;

    // 받아온 Recv 버퍼의 내용을 저장해야함.
    gPacketHandler->Write(mOverlappedRecv.buffer.data(), numOfBytes);
    RegisterRecv();
}

void Session::ProcessSend(INT32 numOfBytes) {
    // TODO
}

bool Session::IsConnected() const {
    return mConnected.load();
}

void Session::InitSessionNetAddress(char* addressBuffer) {
    if (true == mConnected.exchange(true)) {
        return;
    }

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

    RegisterRecv();
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
