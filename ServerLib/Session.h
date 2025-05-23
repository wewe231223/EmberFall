#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Session.h
// 
// 2025 - 01 - 14 (설명 추가 날짜)
//      김성준: 연결된 클라이언트를 추상화한 클래스
//              실질적인 Socket I/O 작업은 이 클래스에서 이루어짐
// 
//      mOverlappedRecv는 매번 Recv 버퍼를 새로 생성할 필요가 없으므로 멤버로 선언
//      
//      모든 I/O 작업은 Register와 Process 작업으로 나뉨
//          Register는 IO 작업을 등록하는 함수
//          Process는 완료된 I/O 작업에 대한 후처리를 위한 함수
//      
//      RegisterSend, ProcessSend 함수에서 OverlappedSend 객체가 new delete됨. (쓰레드간 데이터 전송에 필요)
// 
//      Recv
//          RegisterRecv -> I/O -> WorkerThread -> ProcessRecv -> RegisterRecv (반복)
//      Send
//          RegisterSend -> I/O -> WorkerThread -> ProcessSend
// 
//      01 - 15 : 연결 여부를 Enum 값에서 atomic_bool로 설정 연결여부 확인에 lock은 필요 없도록 함
//      01 - 20 : std::mutex mLock 변수 삭제, lock, unlock 함수 삭제
// 
//      02 - 10 : SocketError가 발생했을 때 다시 재귀로 IO함수를 호출하고 있는데 에러를 관리하는 함수를 따로 만들어
//                그 함수를 호출하는 방식으로 다시 만들었다.
//                정상적으로 종료되지 않거나 호스트가 강제로 종료되면 StackOverflow문제가 발생되는 현상 해결
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "INetworkObject.h"

inline constexpr int32_t MAX_SESSION_HEART_BEAT_CNT = 5;

using RecvBuf = NetworkBuf<BUF_NETWORK_RECV_SIZE>;

class Session : public INetworkObject {
public:
    Session(NetworkType networkType=NetworkType::CLIENT);
    ~Session();

public:
    virtual HANDLE GetHandle() const override;
    virtual bool IsClosed() const override;

    virtual void ProcessOverlapped(OverlappedEx* overlapped, INT32 numOfBytes) override;
    virtual void Close() override;

    void RegisterRecv();
    void RegisterSend(OverlappedSend* const overlapped);
    virtual void ProcessRecv(INT32 numOfBytes);
    void ProcessSend(INT32 numOfBytes, OverlappedSend* overlappedSend);

    bool IsConnected() const;

    void InitSessionNetAddress(char* addressBuffer);
    std::pair<std::string, UINT16> GetAddress() const;
    RecvBuf::iterator ValidatePackets(RecvBuf::iterator iter, RecvBuf::iterator last);

    virtual void OnConnect();

    // 에러 발생으로 인한 연결 종료 처리
    void HandleSocketError(INT32 errorCore);

    // For Client
    bool Connect(const std::string& serverIp, const UINT16 port);
    void ProcessConnect(INT32 numOfBytes, OverlappedConnect* overlapped);
    void WaitTilSessionConn();
    void NotifyingSessionConn();
    
public:
    std::atomic_int32_t mHeartBeat{ };

private:
    std::string mIP{ };
    UINT16 mPort{ };
    NetworkType mNetworkType{ NetworkType::CLIENT };

    std::atomic_bool mConnected{ false }; // 01-14 클라이언트 연결여부를 atomic_bool로 수정
    SOCKET mSocket{ INVALID_SOCKET };

protected:
    size_t mPrevRemainSize{ };
    OverlappedRecv mOverlappedRecv{ };
};