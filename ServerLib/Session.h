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

    void InitSessionNetAddress(char* addressBuffer);
    std::pair<std::string, UINT16> GetAddress() const;

    /* std::lock_guard 사용을 위한 함수 정의 */
    void lock();
    void unlock();

private:
    std::mutex mLock{ };

    std::string mIP{ };
    UINT16 mPort{ };

    SessionState mState{ SessionState::NONE };
    size_t mPrevRemainSize{ };

    SOCKET mSocket{ INVALID_SOCKET };
    OverlappedRecv mOverlappedRecv{ };
};