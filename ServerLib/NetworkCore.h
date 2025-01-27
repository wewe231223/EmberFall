#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////
//
// PacketHandler
// 2025 - 01 - 19
//      김성준: 클라이언트도 IOCP를 이용하도록 함
//              전역객체가 너무 많아지는 문제, 서버 클라이언트 분리를 위한 인터페이스와 클래스 정의
// 
//          전역객체를 너무 많이 사용하는 문제를 해결하기 위해 Core 에 전역객체들을 몰아넣을 예정
//          
// 내부에서 shared_from_this를 사용하고 있으므로 전역객체를 만든다면 std::shared_ptr로 생성해줘야함.
// 
// 01-19 ClientCore는 완성 X
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "IOCPCore.h"
#include "SessionManager.h"
#include "SendBuffers.h"
#include "PacketHandler.h"

enum class NetworkType : BYTE {
    SERVER,
    CLIENT
};

class INetworkCore abstract : public std::enable_shared_from_this<INetworkCore> {
public:
    INetworkCore(NetworkType type);
    virtual ~INetworkCore();

public:
    NetworkType GetType() const;
    std::shared_ptr<IOCPCore> GetIOCPCore() const;
    std::shared_ptr<PacketHandler> GetPacketHandler() const;
    std::shared_ptr<SendBufferFactory> GetSendBufferFactory() const;

    virtual void Init();
    virtual bool Start(const std::string& ip, const UINT16 port) abstract;
    virtual void End() abstract;

    bool PQCS(INT32 transfferdBytes, ULONG_PTR completionKey, OverlappedEx* overlapped);

private:
    NetworkType mType{ };
    std::shared_ptr<IOCPCore> mIocpCore{ nullptr };
    std::shared_ptr<PacketHandler> mPacketHandler{ nullptr };
    std::shared_ptr<SendBufferFactory> mSendBufferFactory{ nullptr };
};

class ServerCore : public INetworkCore {
public:
    ServerCore(size_t workerThreadNum=HARDWARE_CONCURRENCY);
    virtual ~ServerCore();

public:
    bool IsListenerClosed() const;
    std::shared_ptr<SessionManager> GetSessionManager() const;

    virtual bool Start(const std::string& ip, const UINT16 port) override;
    virtual void End() override;

private:
    std::shared_ptr<class Listener> mListener{ nullptr };
    std::shared_ptr<SessionManager> mSessionManager{ nullptr };
    std::vector<std::thread> mWorkerThreads{ };
    size_t mWorkerThreadNum{ };
};

class ClientCore : public INetworkCore {
public:
    ClientCore();
    virtual ~ClientCore();

public:
    virtual bool Start(const std::string& ip, const UINT16 port) override;
    virtual void End() override;
    
    void InitSessionId(SessionIdType id);
    SessionIdType GetSessionId() const;
    bool IsClosedSession() const;
    OverlappedConnect* GetOverlappedConnect();
    void Send(void* packet);
    void Send(void* data, size_t dataSize);
    void CloseSession();

private:
    std::thread mWorkerThread{ };
    std::shared_ptr<class Session> mSession{ nullptr };
    OverlappedConnect mOverlappedConnect{ };
    OverlappedDisconnect mOverlappedDisconnect{ };
};