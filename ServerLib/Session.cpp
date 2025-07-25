#include "pch.h"
#include "Session.h"
#include "NetworkCore.h"

Session::Session(SOCKET socket, SessionNetType type) : mSocket{ socket }, mNetType{ type } {
    CrashExp(INVALID_SOCKET != mSocket, "");
}

Session::~Session() {
    if (IsClosed()) {
        return;
    }

    auto myId = GetId();
    Close();
}

HANDLE Session::GetHandle() const {
    return reinterpret_cast<HANDLE>(mSocket);
}

bool Session::IsClosed() const {
    return false == mConnected;
}

void Session::ProcessOverlapped(OverlappedEx* overlapped, INT32 numOfBytes) {
    switch (overlapped->type) {
    case IoType::SEND:
        ProcessSend(numOfBytes, reinterpret_cast<OverlappedSend*>(overlapped));
        break;

    case IoType::RECV:
        ProcessRecv(numOfBytes);
        break;

    default:
        break;
    }
}

void Session::Close() {
    bool expected = true;
    if (false == mConnected.compare_exchange_strong(expected, false)) {
        return;
    }

    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Close Session Success");
    ::closesocket(mSocket);
    mSocket = INVALID_SOCKET;
}

bool Session::RegisterRecv() {
    if (false == mConnected.load()) {
        return false;
    }

    DWORD receivedBytes{ };
    DWORD flag{ };

    //mOverlappedRecv.ResetOverlapped();
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

    if (SOCKET_ERROR != result) {
        return true;
    }

    auto errorCode = ::WSAGetLastError();
    if (WSA_IO_PENDING != errorCode) {
        if (SessionNetType::CLIENT == mNetType) {
            HandleSocketError(errorCode);
            return false;
        }
        else {
            return false;
        }
    }

    return true;
}

bool Session::RegisterSend(OverlappedSend* const overlappedSend) {
    if (false == mConnected.load()) {
        return false;
    }

    if (nullptr == overlappedSend) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Get Overlapped Send Failure");
        return false;
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

    if (SOCKET_ERROR != result) {
        return true;
    }

    auto errorCode = ::WSAGetLastError();
    if (WSA_IO_PENDING != errorCode) {
        FbsPacketFactory::ReleasePacketBuf(overlappedSend);
        if (SessionNetType::CLIENT == mNetType) {
            HandleSocketError(errorCode);
            return false;
        }
        else {
            gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "Socket Error, Send!: ", NetworkUtil::WSAErrorMessage());
            return false;
        }
    }

    return true;
}

void Session::ProcessRecv(INT32 numOfBytes) { }

void Session::ProcessSend(INT32 numOfBytes, OverlappedSend* overlappedSend) {
    if (0 >= numOfBytes) {
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
}

std::pair<std::string, UINT16> Session::GetAddress() const {
    return std::make_pair(mIP, mPort);
}

void Session::WaitTilSessionConn() {
    mConnected.wait(false);
}

void Session::NotifyingSessionConn() {
    mConnected.notify_all();
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
    {
    }
    break;

    default:
    {
        MessageBoxA(nullptr, "Socket Error!", NetworkUtil::WSAErrorMessage().c_str(), MB_OK | MB_ICONERROR);
    }
    break;
    }
}
