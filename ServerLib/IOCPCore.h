#pragma once

#include "INetworkObject.h"

class IOCPCore {
public:
    IOCPCore();

    ~IOCPCore();

    IOCPCore(const IOCPCore&) = delete;
    IOCPCore(IOCPCore&&) noexcept = delete;
    IOCPCore& operator=(const IOCPCore&) = delete;
    IOCPCore& operator=(IOCPCore&&) noexcept = delete;

public:
    HANDLE GetHandle() const { return mIocpHandle; }

    void RegisterSocket(SOCKET socket, ULONG_PTR registerKey);
    void RegisterSocket(const INetworkObject* session);

    void IOWorker();

private:
    HANDLE mIocpHandle{ INVALID_HANDLE_VALUE };
};