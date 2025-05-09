#include "pch.h"
#include "IOCPCore.h"
#include "NetworkCore.h"

IOCPCore::IOCPCore(std::shared_ptr<INetworkCore> coreService) 
    : mCoreService{ coreService } { }

IOCPCore::~IOCPCore() {
    ::CloseHandle(mIocpHandle);
    mIocpHandle = INVALID_HANDLE_VALUE;
}

void IOCPCore::Init(size_t workerThreadNum) {
    mIocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, static_cast<DWORD>(workerThreadNum));
    CrashExp(NULL != mIocpHandle, "IOCP Creation Failure");
}

HANDLE IOCPCore::GetHandle() const {
    return mIocpHandle;
}

void IOCPCore::RegisterSocket(SOCKET socket, ULONG_PTR registerKey) {
    auto result = ::CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket), mIocpHandle, registerKey, 0);
}

void IOCPCore::RegisterSocket(const std::shared_ptr<INetworkObject>& networkObject) {
    auto result = ::CreateIoCompletionPort(
        networkObject->GetHandle(),
        mIocpHandle,
        static_cast<ULONG_PTR>(networkObject->GetId()),
        0
    );

    if (NULL == result) {
        return;
    }
}

void IOCPCore::IOWorker(int32_t threadId) {
    static SessionIdType lastErrorClient{ INVALID_SESSION_ID };
    DWORD receivedByte{ };
    ULONG_PTR completionKey{ };
    OVERLAPPED* overlapped{ nullptr };

    while (true) {
        auto success = ::GetQueuedCompletionStatus(
            GetHandle(),
            &receivedByte,
            &completionKey,
            &overlapped,
            INFINITE
        );

        OverlappedEx* overlappedEx = reinterpret_cast<OverlappedEx*>(overlapped);
        SessionIdType clientId = static_cast<SessionIdType>(completionKey);

        if (not success) {
            overlappedEx->owner.reset();
            if (IOType::ACCEPT == overlappedEx->type) {
                gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "Accept Error!!");
                gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "{}", NetworkUtil::WSAErrorMessage());
                Crash("Accept Error");
            }
            else if (IOType::CONNECT == overlappedEx->type) {
                gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "Connect Error!!");
                MessageBoxA(nullptr, NetworkUtil::WSAErrorMessage().c_str(), "", MB_OK);
                Crash("Connect Error");
            }

            // IOType::SEND or IOType::RECV
            if (NetworkType::SERVER == mCoreService->GetType()) {
                auto serverCore = std::static_pointer_cast<ServerCore>(mCoreService);
                if (IOType::SEND == overlappedEx->type) {
                    FbsPacketFactory::ReleasePacketBuf(reinterpret_cast<OverlappedSend*>(overlappedEx));

                    if (lastErrorClient != clientId) {
                        gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "Client[{}] Error Send", static_cast<INT32>(clientId));
                    }
                    lastErrorClient = clientId;
                }

                serverCore->GetSessionManager()->CloseSession(clientId);
                continue;
            }
            else {
                auto clientCore = std::static_pointer_cast<ClientCore>(mCoreService);
                if (IOType::SEND == overlappedEx->type) {
                    FbsPacketFactory::ReleasePacketBuf(reinterpret_cast<OverlappedSend*>(overlappedEx));
                    gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "Error Send");
                }

                clientCore->CloseSession();
                break;
            }
        }

        if (IOType::DISCONNECT == overlappedEx->type) {
            break;
        }

        if (nullptr == overlappedEx->owner) {
            auto header = FbsPacketFactory::GetHeaderPtrSC(reinterpret_cast<const uint8_t* const>(overlappedEx->wsaBuf.buf));
            gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "OverlappedEx's owner is Null");
            gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "Error Overlapped Info: ID: {}", clientId);
            gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "Error PacketType: {}", Packets::EnumNamePacketTypes(static_cast<Packets::PacketTypes>(header->type)));
            Crash("");
        }

        overlappedEx->owner->ProcessOverlapped(overlappedEx, receivedByte);
    }
}
