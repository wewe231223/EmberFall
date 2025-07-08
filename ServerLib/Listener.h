#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Listener
// 2025 - 01 - 14 (설명 추가 날짜)
//      김성준: 클라이언트들의 연결요청을 처리할 클래스
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "INetworkObject.h"

class Listener {
public:
    Listener() = delete;
    Listener(const UINT16 port);
    ~Listener();

public:
    HANDLE GetHandle() const;
    SOCKET GetListenSocket() const;
    bool IsClosed() const;

    bool Init();
    void Close();

    void ProcessOverlapped(OverlappedEx* overlapped, INT32 numOfBytes);
    void RegisterAccept();

private:
    SOCKET mListenSocket{ INVALID_SOCKET };
    SOCKET mClientSocket{ INVALID_SOCKET };
    OverlappedAccept mOverlappedAccept{ };

    const unsigned short mLocalPort{ };
};