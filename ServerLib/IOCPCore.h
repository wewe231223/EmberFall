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
    void Init(size_t workerThreadNum);

    HANDLE GetHandle() const { return mIocpHandle; }

    void RegisterSocket(SOCKET socket, ULONG_PTR registerKey);
    void RegisterSocket(const std::shared_ptr<INetworkObject>& networkObject);

    void IOWorker();

private:
    HANDLE mIocpHandle{ INVALID_HANDLE_VALUE };
};