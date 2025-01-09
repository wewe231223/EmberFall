#pragma once

class INetworkObject abstract : public std::enable_shared_from_this<INetworkObject> {
public:
    INetworkObject();
    virtual ~INetworkObject();

public:
    SessionIdType GetId() const;
    virtual void Close() abstract;

    virtual HANDLE GetHandle() const abstract;

    virtual void ProcessOverlapped(class OverlappedEx* overlapped, INT32 numOfBytes) abstract;

    void PostQueuedCompletionStatus();

private:
    SessionIdType mId{ INVALID_CLIENT_ID };
};