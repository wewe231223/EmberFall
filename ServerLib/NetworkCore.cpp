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

std::shared_ptr<SendBufferFactory> INetworkCore::GetSendBufferFactory() const {
    return mSendBufferFactory;
}

void INetworkCore::Init() {
    mIocpCore = std::make_shared<IOCPCore>(shared_from_this());
    mPacketHandler = std::make_shared<PacketHandler>();
    mSendBufferFactory = std::make_shared<SendBufferFactory>();
}

ServerCore::ServerCore(size_t workerThreadNum) 
    : INetworkCore{ NetworkType::SERVER }, mWorkerThreadNum{ workerThreadNum } {
    mWorkerThreads.reserve(workerThreadNum);
}

ServerCore::~ServerCore() { }

std::shared_ptr<SessionManager> ServerCore::GetSessionManager() const {
    return mSessionManager;
}

bool ServerCore::Start(const std::string& ip, const UINT16 port) {
    WSADATA data{ };
    if (0 != ::WSAStartup(MAKEWORD(2, 2), &data)) {
        return false;
    }

    GetIOCPCore()->Init(mWorkerThreadNum);
    auto sharedPtrThis = std::static_pointer_cast<ServerCore>(shared_from_this());
    mSessionManager = std::make_shared<SessionManager>(sharedPtrThis);
    mListener = std::make_unique<Listener>(port, sharedPtrThis);

    GetIOCPCore()->RegisterSocket(mListener);
    mListener->RegisterAccept();

    for (size_t i = 0; i < mWorkerThreadNum; ++i) {
        mWorkerThreads.emplace_back(
            [=]()
            {
                GetIOCPCore()->IOWorker();
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

ClientCore::ClientCore() 
    : INetworkCore{ NetworkType::CLIENT } {
    mSession = std::make_shared<Session>(shared_from_this());
}

ClientCore::~ClientCore() { }

bool ClientCore::Start(const std::string& ip, const UINT16 port) {
    WSADATA data{ };
    if (0 != ::WSAStartup(MAKEWORD(2, 2), &data)) {
        return false;
    }

    mSession->Connect(ip, port);
    mWorkerThread = std::thread{ [=]() { GetIOCPCore()->IOWorker(); } };

    return true;
}

void ClientCore::End() {
    if (mWorkerThread.joinable()) {
        mWorkerThread.join();
    }

    mSession->Close();
    ::WSACleanup();
}

void ClientCore::Send(void* data, size_t dataSize) {
    mSession->RegisterSend(data, dataSize);
}
