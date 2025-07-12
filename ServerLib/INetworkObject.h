#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////
//
// INetworkObject
// 2025 - 01 - 14 (설명 추가 날짜)
//      김성준: Session, Listener 클래스의 부모 클래스
//              작업자 쓰레드에 전달되는 Overlapped 구조체가 소유주를 기록할 수 있도록 하기위한 장치
// 
// 2025 - 06 - 18
//       더이상 std::shared_ptr로 관리되지 않으며 Overlapped 구조체가 포인터를 가지지 않도록 변경
//          
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct OverlappedSend;

class IServerEntity abstract {
public:
    IServerEntity();
    IServerEntity(uint16_t mGameRoomIdx);
    virtual ~IServerEntity();

public:
    void InitId(NetworkObjectIdType id);
    NetworkObjectIdType GetId() const;
    virtual bool IsClosed() const { return false;  }
    uint16_t GetMyRoomIdx() const;

    void SetRoomIdx(uint16_t roomIdx);
    void StorePacket(OverlappedSend* sendBuf);
    Concurrency::concurrent_queue<OverlappedSend*>& GetSendBuf();

    virtual void Close() { }
    virtual HANDLE GetHandle() const { return INVALID_HANDLE_VALUE; }

private:
    uint16_t mGameRoomIdx{ };
    NetworkObjectIdType mId{ INVALID_SESSION_ID };
    Concurrency::concurrent_queue<OverlappedSend*> mSendBuf{ };
};