#include "pch.h"
#include "Session.h"
#include "NetworkCore.h"

Session::Session(NetworkType networkType) 
    : INetworkObject{ }, mNetworkType{ networkType } {
    mSocket = NetworkUtil::CreateSocket();
    CrashExp(INVALID_SOCKET != mSocket, "");
}

Session::~Session() {
    if (IsClosed()) {
        return;
    }

    auto myId = GetId();
    gServerCore->GetSessionManager()->ReleaseSessionId(static_cast<SessionIdType>(myId));
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
    mConnected.exchange(false);

    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Close Session Success");
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
            HandleSocketError(errorCode);
        }
    }
}

void Session::RegisterSend(OverlappedSend* const overlappedSend) {
    if (false == mConnected.load()) {
        return;
    }

    if (nullptr == overlappedSend) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Get Overlapped Send Failure");
        return;
    }

    auto sharedThis = shared_from_this();
    overlappedSend->owner = sharedThis;
    if (nullptr == overlappedSend->owner) {
        gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "Overlapped Send's owner is Null");
        Crash("Overlapped Send's owner is Null");
    }

    DWORD sentBytes{ };
    DWORD dataSize = overlappedSend->wsaBuf.len;
    auto result = ::WSASend(
        mSocket,
        &overlappedSend->wsaBuf,
        1,
        &sentBytes,
        0,
        overlappedSend,
        nullptr
    );

    if (SOCKET_ERROR == result) {
        auto errorCode = ::WSAGetLastError();
        if (WSA_IO_PENDING != errorCode) {
            FbsPacketFactory::ReleasePacketBuf(overlappedSend);
            HandleSocketError(errorCode);
        }
    }

    if (sentBytes != dataSize) {
        Crash(true);
    }
}

void Session::ProcessRecv(INT32 numOfBytes) {
    mOverlappedRecv.owner.reset();
    if (0 >= numOfBytes) {
        gServerCore->GetSessionManager()->CloseSession(static_cast<SessionIdType>(GetId()));
        return;
    }

    auto dataBeg = mOverlappedRecv.buffer.begin();
    auto dataEnd = dataBeg + numOfBytes;
    auto remainBegin = ValidatePackets(dataBeg, dataEnd);
    mPrevRemainSize = std::distance(remainBegin, dataEnd);
    auto dataSize = numOfBytes - mPrevRemainSize;

    // 받아온 Recv 버퍼의 내용을 저장.
    auto coreService = gClientCore;
    if (0 == mPrevRemainSize) {
        coreService->GetPacketHandler()->Write(mOverlappedRecv.buffer.data(), dataSize);
        RegisterRecv();
        return;
    }

    std::move(remainBegin, dataEnd, dataBeg);
    RegisterRecv();
}

void Session::ProcessSend(INT32 numOfBytes, OverlappedSend* overlappedSend) {
    if (0 >= numOfBytes) {
        gServerCore->GetSessionManager()->CloseSession(static_cast<SessionIdType>(GetId()));
        return;
    }

    FbsPacketFactory::ReleasePacketBuf(overlappedSend);
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
    auto clientCore = gClientCore;
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

    RegisterRecv();
}

void Session::WaitTilSessionConn() {
    mConnected.wait(false);
}

void Session::NotifyingSessionConn() {
    mConnected.notify_all();
}

RecvBuf::iterator Session::ValidatePackets(RecvBuf::iterator iter, RecvBuf::iterator last) {
    while (iter != last) {
        if constexpr (sizeof(PacketSizeT) > sizeof(char)) {
            if (std::distance(iter, last) < sizeof(PacketSizeT)) {
                break;
            }
        }

        auto packetSize = NetworkUtil::GetPacketSizeFromIter(iter);
        if (sizeof(PacketHeaderSC) > packetSize) {
            MessageBoxA(nullptr, std::format("PacketSize is: {}", packetSize).c_str(), "", MB_OK | MB_ICONERROR);
            exit(-1);
        }

        iter += packetSize;
    }

    return iter;
}

void Session::OnConnect() { 
    decltype(auto) packetId = FbsPacketFactory::NotifyIdSC(static_cast<SessionIdType>(GetId()));
    RegisterSend(packetId);

    decltype(auto) packetProtocolVersion = FbsPacketFactory::ProtocolVersionSC();
    RegisterSend(packetProtocolVersion);
}

void Session::HandleSocketError(INT32 errorCore) {
    switch (errorCore) {
    case WSAECONNRESET: // 소프트웨어로 인해 연결 중단.
    case WSAECONNABORTED: // 피어별 연결 다시 설정. (원격 호스트에서 강제 중단.)
        if (NetworkType::CLIENT == mNetworkType) {
            gClientCore->CloseSession();
            MessageBoxA(nullptr, "Socket Error!", NetworkUtil::WSAErrorMessage().c_str(), MB_OK | MB_ICONERROR);
        }
        else {
            gServerCore->GetSessionManager()->CloseSession(static_cast<SessionIdType>(GetId()));
            gLogConsole->PushLog(DebugLevel::LEVEL_ERROR, "Socket Error: {}", NetworkUtil::WSAErrorMessage());
        }
        break;

    default:
        gLogConsole->PushLog(DebugLevel::LEVEL_ERROR, "Socket Error: {}", NetworkUtil::WSAErrorMessage());
        break;
    }
}
