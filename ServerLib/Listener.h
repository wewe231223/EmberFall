#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Listener
// 2025 - 01 - 14 (설명 추가 날짜)
//      김성준: 클라이언트들의 연결요청을 처리할 클래스
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "INetworkObject.h"

class Listener : public INetworkObject {
public:
    Listener() = delete;
    Listener(const UINT16 port, std::shared_ptr<class INetworkCore> coreService);
    ~Listener();

public:
    virtual HANDLE GetHandle() const override;
    virtual void Close() override;
    virtual bool IsClosed() const override;

    virtual void ProcessOverlapped(OverlappedEx* overlapped, INT32 numOfBytes) override;
    void RegisterAccept();
    void ProcessAccept();

private:
    SOCKET mListenSocket{ INVALID_SOCKET };
    OverlappedAccept mOverlappedAccept{ };

    const unsigned short mLocalPort{ };
};