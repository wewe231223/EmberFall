#pragma once

#include "INetworkObject.h"

enum class SessionState : UINT {
    CONNECTED,
    DISCONNECTED,
    NONE,
};

class Session : public INetworkObject {
public:
    Session();
    ~Session();

public:
    void SetRemain(size_t remainSize);
    virtual HANDLE GetHandle() const override;

    virtual void ProcessOverlapped(OverlappedEx* overlapped, INT32 numOfBytes) override;

    virtual void Close() override;
    void RegisterRecv();
    void RegisterSend(void* packet);
    void ProcessRecv();
    void ProcessSend();

    SessionState GetCurrentState() const;

    /* std::lock_guard 사용을 위한 함수 정의 */
    void lock();
    void unlock();

private:
    std::mutex mLock{ };
    SOCKET mSocket{ INVALID_SOCKET };
    size_t mPrevRemainSize{ };
    SessionState mState{ SessionState::NONE };
    OverlappedRecv mOverlappedRecv{ };
};