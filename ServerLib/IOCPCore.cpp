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
    CrashExp(NULL == mIocpHandle, "IOCP Creation Failure");
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

void IOCPCore::IOWorker() {
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
            if (IOType::ACCEPT == overlappedEx->type) {
                gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "Accept Error!!");
                Crash("Accept Error");
            }
            else if (IOType::CONNECT == overlappedEx->type) {
                gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "Connect Error!!");
                Crash("Connect Error");
            }

            // IOType::SEND or IOType::RECV
            gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "Client[{}] Error", static_cast<INT32>(clientId));
            if (NetworkType::SERVER == mCoreService->GetType()) {
                auto serverCore = std::static_pointer_cast<ServerCore>(mCoreService);
                serverCore->GetSessionManager()->CloseSession(clientId);
                continue;
            }
            else {
                auto clientCore = std::static_pointer_cast<ClientCore>(mCoreService);
                clientCore->CloseSession();
                break;
            }
        }

        if (IOType::DISCONNECT == overlappedEx->type) {
            break;
        }

        overlappedEx->owner->ProcessOverlapped(overlappedEx, receivedByte);
    }
}
