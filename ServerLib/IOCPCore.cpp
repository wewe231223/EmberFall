#include "pch.h"
#include "IOCPCore.h"

IOCPCore::IOCPCore() {
    mIocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    CrashExp(INVALID_HANDLE_VALUE != mIocpHandle, "IOCP Creation Failure");
}

IOCPCore::~IOCPCore() {
    ::CloseHandle(mIocpHandle);
    mIocpHandle = INVALID_HANDLE_VALUE;
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
