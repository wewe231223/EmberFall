#include "pch.h"
#include "IOCPCore.h"
#include "NetworkCore.h"
#include "Session.h"

IOCPCore::IOCPCore() { }

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

void IOCPCore::RegisterSocket(const IServerEntity* const networkObject) {
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

void IOCPCore::ClientIoThread() {
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
            if (IoType::CONNECT == overlappedEx->type) {
                gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "Connect Error!!");
                MessageBoxA(nullptr, NetworkUtil::WSAErrorMessage().c_str(), "", MB_OK);
                Crash("Connect Error");
            }
            else {
                auto clientCore = gClientCore;
                if (IoType::SEND == overlappedEx->type) {
                    FbsPacketFactory::ReleasePacketBuf(reinterpret_cast<OverlappedSend*>(overlappedEx));
                    gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "Error Send");
                }

                clientCore->CloseSession();
                break;
            }
        }

        if (IoType::DISCONNECT == overlappedEx->type) {
            break;
        }

        auto session = gClientCore->GetSession();
        if (nullptr == session) {
            break;
        }

        session->ProcessOverlapped(overlappedEx, receivedByte);
    }
}
