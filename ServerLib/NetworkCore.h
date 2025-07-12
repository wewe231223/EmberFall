#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////
//
// NetworkCore.h
// 2025 - 01 - 19
//      김성준: 클라이언트도 IOCP를 이용하도록 함
//              전역객체가 너무 많아지는 문제, 서버 클라이언트 분리를 위한 인터페이스와 클래스 정의
// 
//          전역객체를 너무 많이 사용하는 문제를 해결하기 위해 Core 에 전역객체들을 몰아넣을 예정
//          
// 내부에서 shared_from_this를 사용하고 있으므로 전역객체를 만든다면 std::shared_ptr로 생성해줘야함.
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "IOCPCore.h"
#include "SendBuffers.h"
#include "PacketHandler.h"

class ClientCore {
public:
    ClientCore();
    ~ClientCore();

public:
    bool Start(const std::string& ip, const UINT16 port);
    void End();
    
    void InitSessionId(SessionIdType id);
    std::shared_ptr<class Session> GetSession() const;
    SessionIdType GetSessionId() const;
    bool IsClosedSession() const;
    std::shared_ptr<PacketHandler> GetPacketHandler() const;

    OverlappedConnect* GetOverlappedConnect();
    void Send(OverlappedSend* const overlappedSend);
    void CloseSession();

    bool PQCS(INT32 transfferdBytes, ULONG_PTR completionKey, OverlappedEx* overlapped);

private:
    std::thread mWorkerThread{ };
    std::shared_ptr<class Session> mSession{ nullptr };
    OverlappedConnect mOverlappedConnect{ };
    OverlappedDisconnect mOverlappedDisconnect{ };

    std::shared_ptr<IOCPCore> mIocpCore{ nullptr };
    std::shared_ptr<PacketHandler> mPacketHandler{ nullptr };
};