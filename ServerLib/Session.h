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
    void ProcessRecv(INT32 numOfBytes);
    void ProcessSend(INT32 numOfBytes);

    SessionState GetCurrentState() const;
    bool IsConnected() const;

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
    std::atomic_bool mConnected{ false }; // 01-14 클라이언트 연결여부를 atomic_bool로 수정
    size_t mPrevRemainSize{ };

    SOCKET mSocket{ INVALID_SOCKET };
    OverlappedRecv mOverlappedRecv{ };
};