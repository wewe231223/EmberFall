#pragma once

class ServerCore {
public:
    ServerCore();
    ~ServerCore();

public:
    void Start(const std::string& ip, const unsigned short port);
    void IOWorker();

private:
    //std::unique_ptr<class Listener> mListener{ nullptr };
    //std::unique_ptr<class PacketHandler> mPacketHandler{ nullptr };
    //std::unique_ptr<class SessionManager> mSessionManager{ nullptr };
};