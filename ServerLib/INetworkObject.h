#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////
//
// INetworkObject
// 2025 - 01 - 14 (설명 추가 날짜)
//      김성준: Session, Listener 클래스의 부모 클래스
//              작업자 쓰레드에 전달되는 Overlapped 구조체가 소유주를 기록할 수 있도록 하기위한 장치
//          
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct OverlappedSend;

class INetworkObject abstract : public std::enable_shared_from_this<INetworkObject> {
public:
    INetworkObject();
    virtual ~INetworkObject();

public:
    void InitId(NetworkObjectIdType id);
    NetworkObjectIdType GetId() const;
    virtual bool IsClosed() const { return false;  }

    void StorePacket(OverlappedSend* sendBuf);
    Concurrency::concurrent_queue<OverlappedSend*>& GetSendBuf();

    virtual void Close() { }
    virtual HANDLE GetHandle() const { return INVALID_HANDLE_VALUE; }
    virtual void ProcessOverlapped(struct OverlappedEx* overlapped, int32_t numOfBytes) abstract;

private:
    NetworkObjectIdType mId{ INVALID_SESSION_ID };
    Concurrency::concurrent_queue<OverlappedSend*> mSendBuf{ };
};