#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////
//
// INetworkObject
// 2025 - 01 - 14 (설명 추가 날짜)
//      김성준: IOCP 핸들 확장, WorkerThread
//          
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "INetworkObject.h"

class IOCPCore {
public:
    IOCPCore(std::shared_ptr<class INetworkCore> coreService);
    ~IOCPCore();

    IOCPCore(const IOCPCore&) = delete;
    IOCPCore(IOCPCore&&) noexcept = delete;
    IOCPCore& operator=(const IOCPCore&) = delete;
    IOCPCore& operator=(IOCPCore&&) noexcept = delete;

public:
    void Init(size_t workerThreadNum);

    HANDLE GetHandle() const;

    void RegisterSocket(SOCKET socket, ULONG_PTR registerKey);
    void RegisterSocket(const std::shared_ptr<INetworkObject>& networkObject);

    void IOWorker();

private:
    HANDLE mIocpHandle{ INVALID_HANDLE_VALUE };
    std::shared_ptr<class INetworkCore> mCoreService{ nullptr };
};