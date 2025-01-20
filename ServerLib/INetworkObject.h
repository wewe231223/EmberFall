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

class INetworkObject abstract : public std::enable_shared_from_this<INetworkObject> {
public:
    INetworkObject(std::shared_ptr<class INetworkCore> coreService);
    virtual ~INetworkObject();

public:
    void InitId(SessionIdType id);
    SessionIdType GetId() const;
    std::shared_ptr<class INetworkCore> GetCore() const;
    virtual bool IsClosed() const abstract;

    virtual void Close() abstract;
    virtual HANDLE GetHandle() const abstract;
    virtual void ProcessOverlapped(class OverlappedEx* overlapped, INT32 numOfBytes) abstract;

private:
    SessionIdType mId{ INVALID_CLIENT_ID };
    std::shared_ptr<class INetworkCore> mCoreService{ nullptr };
};