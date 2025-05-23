#include "pch.h"
#include "NetworkCore.h"
#include "Listener.h"
#include "Session.h"

INetworkCore::INetworkCore(NetworkType type) 
    : mType{ type } { }

INetworkCore::~INetworkCore() { }

NetworkType INetworkCore::GetType() const {
    return mType;
}

std::shared_ptr<IOCPCore> INetworkCore::GetIOCPCore() const {
    return mIocpCore;
}

std::shared_ptr<PacketHandler> INetworkCore::GetPacketHandler() const {
    return mPacketHandler;
}

void INetworkCore::Init() {
    mIocpCore = std::make_shared<IOCPCore>(shared_from_this());
    mPacketHandler = std::make_shared<PacketHandler>();
}

bool INetworkCore::PQCS(INT32 transfferdBytes, ULONG_PTR completionKey, OverlappedEx* overlapped) {
    return ::PostQueuedCompletionStatus(mIocpCore->GetHandle(), transfferdBytes, completionKey, overlapped->GetRawPtr());
}

ServerCore::ServerCore(size_t workerThreadNum) 
    : INetworkCore{ NetworkType::SERVER }, mWorkerThreadNum{ workerThreadNum } {
    mWorkerThreads.reserve(workerThreadNum);
}

ServerCore::~ServerCore() { }

bool ServerCore::IsListenerClosed() const {
    return mListener->IsClosed();
}

std::shared_ptr<SessionManager> ServerCore::GetSessionManager() const {
    return mSessionManager;
}

void ServerCore::Init() {
    auto sharedPtrThis = std::static_pointer_cast<ServerCore>(shared_from_this());
    mSessionManager = std::make_shared<SessionManager>(sharedPtrThis);
}

bool ServerCore::Start(const std::string& ip, const UINT16 port) {
    WSADATA data{ };
    if (0 != ::WSAStartup(MAKEWORD(2, 2), &data)) {
        return false;
    }

    INetworkCore::Init();
    GetIOCPCore()->Init(mWorkerThreadNum);
    auto sharedPtrThis = std::static_pointer_cast<ServerCore>(shared_from_this());
    mListener = std::make_unique<Listener>(port, sharedPtrThis);

    GetIOCPCore()->RegisterSocket(mListener);
    mListener->RegisterAccept();

    for (size_t threadId = 0; threadId < mWorkerThreadNum; ++threadId) {
        mWorkerThreads.emplace_back(
            [=]() {
                GetIOCPCore()->IOWorker(static_cast<int32_t>(threadId));
            }
        );
    }

    return true;
}

void ServerCore::End() {
    for (auto& thread : mWorkerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    mListener->Close();

    ::WSACleanup();
}

void ServerCore::Send(SessionIdType to, OverlappedSend* overlappedSend) {
    mSessionManager->Send(to, overlappedSend);
}

ClientCore::ClientCore() 
    : INetworkCore{ NetworkType::CLIENT } {
}

ClientCore::~ClientCore() { }

bool ClientCore::Start(const std::string& ip, const UINT16 port) {
    WSADATA data{ };
    if (0 != ::WSAStartup(MAKEWORD(2, 2), &data)) {
        return false;
    }

    GetIOCPCore()->Init(1);
    mSession = std::make_shared<Session>();
    GetIOCPCore()->RegisterSocket(mSession);
    if (not mSession->Connect(ip, port)) {
        return false;
    }

    mWorkerThread = std::thread{ [=]() { GetIOCPCore()->IOWorker(0); } };

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

SessionIdType ClientCore::GetSessionId() const {
    return static_cast<SessionIdType>(mSession->GetId());
}

bool ClientCore::IsClosedSession() const {
    return mSession->IsClosed();
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
