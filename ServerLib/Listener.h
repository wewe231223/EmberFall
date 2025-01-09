#pragma once

#include "INetworkObject.h"

class Listener : public INetworkObject {
public:
    Listener() = delete;
    Listener(const std::string& localIp, const unsigned short port);
    ~Listener();

public:
    virtual HANDLE GetHandle() const override;
    virtual void Close() override;

    virtual void ProcessOverlapped(OverlappedEx* overlapped, INT32 numOfBytes) override;
    void RegisterAccept();
    void ProcessAccept();

private:
    SOCKET mListenSocket{ INVALID_SOCKET };
    OverlappedAccept mOverlappedAccept{ };

    const std::string mLocalIp{ };
    const unsigned short mLocalPort{ };
};