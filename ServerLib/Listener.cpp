#include "pch.h"
#include "Listener.h"
#include "Session.h"
#include "SessionManager.h"
#include "NetworkCore.h"

Listener::Listener(const UINT16 port, std::shared_ptr<INetworkCore> coreService)
    : INetworkObject{ coreService }, mLocalPort{ port } {
    mListenSocket = NetworkUtil::CreateSocket();
    if (INVALID_SOCKET == mListenSocket) {
        gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "Create Socket Failure: {}", NetworkUtil::WSAErrorMessage());
        Crash("");
    }

    sockaddr_in sockAddr{ };
    NetworkUtil::InitSockAddr(sockAddr, port);
    NetworkUtil::SetSocketOpt(mListenSocket, SOL_SOCKET, SO_REUSEADDR, 1);

    if (SOCKET_ERROR == ::bind(mListenSocket, reinterpret_cast<sockaddr*>(&sockAddr), sizeof(sockAddr))) {
        gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "::bind Error: {}", NetworkUtil::WSAErrorMessage());
        Crash("");
    }

    if (SOCKET_ERROR == ::listen(mListenSocket, SOMAXCONN)) {
        gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "::listen Error: {}", NetworkUtil::WSAErrorMessage());
        Crash("");
    }
}

Listener::~Listener() {
    Close();
}

HANDLE Listener::GetHandle() const {
    return reinterpret_cast<HANDLE>(mListenSocket);
}

void Listener::Close() {
    ::closesocket(mListenSocket);
    mListenSocket = INVALID_SOCKET;
}

bool Listener::IsClosed() const {
    return mListenSocket == INVALID_SOCKET;
}

void Listener::ProcessOverlapped(OverlappedEx* overlapped, INT32 numOfBytes) {
    if (IOType::ACCEPT != overlapped->type) {
        /* Print Error Log */
        return;
    }

    ProcessAccept();

    RegisterAccept();
}

void Listener::RegisterAccept() {
    auto sessionManager = std::static_pointer_cast<ServerCore>(GetCore())->GetSessionManager();
    std::shared_ptr<Session> session = sessionManager->CreateSessionObject();

    mOverlappedAccept.ResetOverlapped();
    mOverlappedAccept.owner = shared_from_this();
    mOverlappedAccept.session = session;

    SOCKET clientSocket = reinterpret_cast<SOCKET>(session->GetHandle());

    INT addrSize{ sizeof(sockaddr_in) + 16 }; // addrsize는 내부 구현상 사용하는 주소체게 구조체 크기 + 16이 되어야함
    DWORD received{ };
    auto registSuccess = ::AcceptEx(
        mListenSocket,
        clientSocket,
        mOverlappedAccept.buffer.data(),
        0,
        addrSize,
        addrSize,
        &received,
        &mOverlappedAccept
    );

    if (not registSuccess) {
        int errorCode = ::WSAGetLastError();
        if (WSA_IO_PENDING != errorCode) {
            RegisterAccept();
        }
    }
}

void Listener::ProcessAccept() {
    auto session = mOverlappedAccept.GetSession();

    auto sessionManager = std::static_pointer_cast<ServerCore>(GetCore())->GetSessionManager();
    if (true == sessionManager->AddSession(session)) {
        session->InitSessionNetAddress(mOverlappedAccept.buffer.data());
        auto [ip, port] = session->GetAddress();
        
        decltype(auto) packetId = FbsPacketFactory::NotifyIdSC(session->GetId());
        sessionManager->Send(session->GetId(), packetId);

        decltype(auto) packetProtocolVersion = FbsPacketFactory::ProtocolVersionSC();
        sessionManager->Send(session->GetId(), packetProtocolVersion);

        session->OnConnect();
        
        gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Client [IP: {}, PORT: {}] Connected", ip, port);
    }
    else {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Client Connect Failure");
    }

    mOverlappedAccept.owner.reset();
    mOverlappedAccept.session.reset();
}
