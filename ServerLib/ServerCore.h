#pragma once

class ServerCore {
public:
    ServerCore();
    ~ServerCore();

public:
    void Start(const std::string& ip, const unsigned short port);
    void IOWorker();

    std::shared_ptr<Session> CreateSession();
    bool AddSession(SessionIdType id, std::shared_ptr<Session> session);
    std::shared_ptr<Session> GetSession(SessionIdType id);

private:
    //std::unique_ptr<class Listener> mListener{ nullptr };
    //std::unique_ptr<class PacketHandler> mPacketHandler{ nullptr };
    std::unique_ptr<class SessionManager> mSessionManager{ nullptr };
};