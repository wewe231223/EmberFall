#include "pch.h"
#include "Session.h"
#include "NetworkCore.h"

Session::Session(std::shared_ptr<class INetworkCore> coreService) 
    : INetworkObject{ coreService } {
    mSocket = NetworkUtil::CreateSocket();
    CrashExp(INVALID_SOCKET == mSocket, "");
}

Session::~Session() {
    Close();
}

HANDLE Session::GetHandle() const {
    return reinterpret_cast<HANDLE>(mSocket);
}

bool Session::IsClosed() const {
    return INVALID_SOCKET == mSocket;
}

void Session::ProcessOverlapped(OverlappedEx* overlapped, INT32 numOfBytes) {
    switch (overlapped->type) {
    case IOType::SEND:
        ProcessSend(numOfBytes, reinterpret_cast<OverlappedSend*>(overlapped));
        break;

    case IOType::RECV:
        ProcessRecv(numOfBytes);
        break;

    case IOType::CONNECT:
        ProcessConnect(numOfBytes, reinterpret_cast<OverlappedConnect*>(overlapped));
        break;

    default:
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
    mOverlappedRecv.wsaBuf.buf = mOverlappedRecv.buffer.data() + mPrevRemainSize;
    mOverlappedRecv.wsaBuf.len = static_cast<UINT32>(mOverlappedRecv.buffer.size() - mPrevRemainSize);
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

void Session::RegisterSend(void* packet) {
    if (false == mConnected.load()) {
        return;
    }

    auto sendBufferFactory = GetCore()->GetSendBufferFactory();
    auto overlappedSend = sendBufferFactory->GetOverlapped(packet, reinterpret_cast<char*>(packet)[0]);
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
            sendBufferFactory->ReleaseOverlapped(overlappedSend);
            RegisterSend(packet);
        }
    }
}

void Session::RegisterSend(void* data, size_t size) {
    if (false == mConnected.load()) {
        return;
    }

    auto sendBufferFactory = GetCore()->GetSendBufferFactory();
    auto overlappedSend = sendBufferFactory->GetOverlapped(data, size);
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
            sendBufferFactory->ReleaseOverlapped(overlappedSend);
            RegisterSend(data, size);
        }
    }
}

void Session::ProcessRecv(INT32 numOfBytes) {
    auto coreService = GetCore();

    std::cout << std::format("RECV Len: {}\n", numOfBytes);
    mOverlappedRecv.owner.reset();
    if (0 >= numOfBytes) {
        if (NetworkType::SERVER == coreService->GetType()) {
            auto serverCore = std::static_pointer_cast<ServerCore>(coreService);
            serverCore->GetSessionManager()->CloseSession(GetId());
            return;
        }
        else {
            auto clientCore = std::static_pointer_cast<ClientCore>(coreService);
            clientCore->CloseSession();
            return;
        }
    }

    auto dataBeg = mOverlappedRecv.buffer.begin();
    auto dataEnd = dataBeg + numOfBytes;
    auto remainBegin = ValidatePackets(dataBeg, dataEnd);
    mPrevRemainSize = std::distance(remainBegin, dataEnd);
    auto dataSize = numOfBytes - mPrevRemainSize;

    // 받아온 Recv 버퍼의 내용을 저장.
    coreService->GetPacketHandler()->Write(mOverlappedRecv.buffer.data(), dataSize);
    std::move(remainBegin, dataEnd, dataBeg);
    RegisterRecv();
}

void Session::ProcessSend(INT32 numOfBytes, OverlappedSend* overlappedSend) {
    auto sendBufferFactory = GetCore()->GetSendBufferFactory();
    sendBufferFactory->ReleaseOverlapped(overlappedSend);
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

bool Session::Connect(const std::string& serverIp, const UINT16 port) {
    sockaddr_in serverAddr{ };
    NetworkUtil::InitSockAddr(serverAddr, 0);
    if (SOCKET_ERROR == ::bind(mSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr))) {
        return false;
    }

    if (false == NetworkUtil::InitSockAddr(serverAddr, port, serverIp.data())) {
        return false;
    }

    if (false == NetworkUtil::InitConnectExFunc(mSocket)) {
        return false;
    }

    DWORD bytes{ };
    auto clientCore = std::static_pointer_cast<ClientCore>(GetCore());
    auto overlappedConnect = clientCore->GetOverlappedConnect();
    overlappedConnect->owner = shared_from_this();
    auto result = NetworkUtil::ConnectEx(
        mSocket,
        reinterpret_cast<sockaddr*>(&serverAddr),
        sizeof(serverAddr),
        nullptr,
        NULL,
        &bytes,
        overlappedConnect
    );

    if (false == result) {
        auto errorCode = ::WSAGetLastError();
        if (WSA_IO_PENDING != errorCode) {
            return false;
        }
    }

    return true;
}

void Session::ProcessConnect(INT32 numOfBytes, OverlappedConnect* overlapped) {
    if (true == mConnected.exchange(true)) {
        return;
    }
}

RecvBuf::iterator Session::ValidatePackets(RecvBuf::iterator iter, RecvBuf::iterator last) {
    auto it = iter;
    while (it != last) {
        auto packetSize = NetworkUtil::GetPacketSizeFromIter(it);
        if (std::distance(it, last) < packetSize) {
            break;
        }
        it += packetSize;
    }

    return it;
}
