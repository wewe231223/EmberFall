#include "pch.h"
#include "Listener.h"
#include "Session.h"
#include "SessionManager.h"
#include "NetworkCore.h"

Listener::Listener(const UINT16 port, std::shared_ptr<INetworkCore> coreService)
    : INetworkObject{ coreService }, mLocalPort{ port } {
    mListenSocket = NetworkUtil::CreateSocket();
    CrashExp(INVALID_SOCKET == mListenSocket, "");

    sockaddr_in sockAddr{ };
    NetworkUtil::InitSockAddr(sockAddr, port);

    if (SOCKET_ERROR == ::bind(mListenSocket, reinterpret_cast<sockaddr*>(&sockAddr), sizeof(sockAddr))) {
        Crash("");
    }

    if (SOCKET_ERROR == ::listen(mListenSocket, SOMAXCONN)) {
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
        if (ERROR_IO_PENDING != errorCode) {
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
        
        PacketNotifyId notifyingId{ sizeof(PacketNotifyId), PacketType::PT_NOTIFYING_ID_SC, session->GetId() };
        sessionManager->Send(session->GetId(), &notifyingId);

        std::cout << std::format("Client [IP: {}, PORT: {}] Connected\n", ip, port);
    }
    else {
        std::cout << "Client Connect Failure\n";
    }

    mOverlappedAccept.owner.reset();
    mOverlappedAccept.session.reset();
}
