#include "pch.h"
#include "NetworkCore.h"
#include "Listener.h"
#include "Session.h"

ClientCore::ClientCore() {
    mIocpCore = std::make_shared<IOCPCore>();
    mPacketHandler = std::make_shared<PacketHandler>();
}

ClientCore::~ClientCore() { }

bool ClientCore::Start(const std::string& ip, const UINT16 port) {
    WSADATA data{ };
    if (0 != ::WSAStartup(MAKEWORD(2, 2), &data)) {
        return false;
    }

    mSession = std::make_shared<Session>(NetworkUtil::CreateSocket());

    mIocpCore->Init(1);
    mIocpCore->RegisterSocket(mSession.get());
    if (not mSession->Connect(ip, port)) {
        return false;
    }

    mWorkerThread = std::thread{ [=]() { mIocpCore->ClientIoThread(); } };

    std::cout << "test" << std::endl; 

    return true; 
}

void ClientCore::End() {
    CloseSession();
    PQCS(0, 0, &mOverlappedDisconnect);

    if (mWorkerThread.joinable()) {
        mWorkerThread.join();
    }

    ::WSACleanup();
}

void ClientCore::InitSessionId(SessionIdType id) {
    mSession->InitId(id);
}

std::shared_ptr<Session> ClientCore::GetSession() const {
    return mSession;
}

SessionIdType ClientCore::GetSessionId() const {
    return static_cast<SessionIdType>(mSession->GetId());
}

bool ClientCore::IsClosedSession() const {
    return mSession->IsClosed();
}

std::shared_ptr<PacketHandler> ClientCore::GetPacketHandler() const {
    return mPacketHandler;
}

OverlappedConnect* ClientCore::GetOverlappedConnect() {
    return &mOverlappedConnect;
}

void ClientCore::Send(OverlappedSend* const overlappedSend) {
    mSession->RegisterSend(overlappedSend);
}

void ClientCore::CloseSession() {
    mSession->Close();
}

bool ClientCore::PQCS(INT32 transfferdBytes, ULONG_PTR completionKey, OverlappedEx* overlapped) {
    return ::PostQueuedCompletionStatus(mIocpCore->GetHandle(), transfferdBytes, completionKey, overlapped->GetRawPtr());
}