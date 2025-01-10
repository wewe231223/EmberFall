#include "pch.h"
#include "IOCPCore.h"

IOCPCore::IOCPCore() { }

IOCPCore::~IOCPCore() {
    ::CloseHandle(mIocpHandle);
    mIocpHandle = INVALID_HANDLE_VALUE;
}

void IOCPCore::Init(size_t workerThreadNum) {
    mIocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, static_cast<DWORD>(workerThreadNum));
    CrashExp(INVALID_HANDLE_VALUE != mIocpHandle, "IOCP Creation Failure");
}

void IOCPCore::RegisterSocket(SOCKET socket, ULONG_PTR registerKey) {
    auto result = ::CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket), mIocpHandle, registerKey, 0);
}

void IOCPCore::RegisterSocket(const INetworkObject* session) {
    auto result = ::CreateIoCompletionPort(
        session->GetHandle(),
        mIocpHandle,
        static_cast<ULONG_PTR>(session->GetId()),
        0
    );

    if (INVALID_HANDLE_VALUE == result) {
        return;
    }
}

void IOCPCore::IOWorker() {
    DWORD receivedByte{ };
    ULONG_PTR completionKey{ };
    OVERLAPPED* overlapped{ nullptr };

    while (true) {
        auto success = ::GetQueuedCompletionStatus(
            INVALID_HANDLE_VALUE,
            &receivedByte,
            &completionKey,
            &overlapped,
            INFINITE
        );

        OverlappedEx* overlappedEx = reinterpret_cast<OverlappedEx*>(overlapped);
        SessionIdType clientId = static_cast<SessionIdType>(completionKey);

        if (not success) {
            if (IOType::ACCEPT == overlappedEx->type) {
                if (overlappedEx->mOwner) {
                    // TODO
                }
                continue;
            }
            else {
                // TODO
                //mClientManager->CloseSession(clientId);
                continue;
            }
        }

        std::shared_ptr<INetworkObject> owener = overlappedEx->mOwner;
    }
}
