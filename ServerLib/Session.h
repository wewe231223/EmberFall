#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////
//
// PacketHandler
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
//      01 - 15 연결 여부를 Enum 값에서 atomic_bool로 설정 연결여부 확인에 lock은 필요 없도록 함
//      01 - 20 std::mutex mLock 변수 삭제, lock, unlock 함수 삭제
//          
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "INetworkObject.h"

using RecvBuf = NetworkBuf<BUF_RW_SIZE>;

class Session : public INetworkObject {
public:
    Session(std::shared_ptr<class INetworkCore> coreService);
    ~Session();

public:
    virtual HANDLE GetHandle() const override;
    virtual bool IsClosed() const override;

    virtual void ProcessOverlapped(OverlappedEx* overlapped, INT32 numOfBytes) override;

    virtual void Close() override;
    void RegisterRecv();
    void RegisterSend(void* packet);
    void RegisterSend(void* data, size_t size);
    void ProcessRecv(INT32 numOfBytes);
    void ProcessSend(INT32 numOfBytes, OverlappedSend* overlappedSend);

    bool IsConnected() const;

    void InitSessionNetAddress(char* addressBuffer);
    std::pair<std::string, UINT16> GetAddress() const;

    // For Client
    bool Connect(const std::string& serverIp, const UINT16 port);
    void ProcessConnect(INT32 numOfBytes, OverlappedConnect* overlapped);
    RecvBuf::iterator ValidatePackets(RecvBuf::iterator iter, RecvBuf::iterator last);

private:
    std::string mIP{ };
    UINT16 mPort{ };

    std::atomic_bool mConnected{ false }; // 01-14 클라이언트 연결여부를 atomic_bool로 수정
    size_t mPrevRemainSize{ };

    SOCKET mSocket{ INVALID_SOCKET };
    OverlappedRecv mOverlappedRecv{ };
};