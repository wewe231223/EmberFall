#pragma once

class Session;

class SessionManager {
    inline constexpr static size_t MAX_CLIENT_SIZE = 10;

public:
    SessionManager();
    ~SessionManager();

    SessionManager(const SessionManager&) = delete;
    SessionManager(SessionManager&&) noexcept = delete;
    SessionManager& operator=(const SessionManager&) = delete;
    SessionManager& operator=(SessionManager&&) noexcept = delete;

public:
    std::shared_ptr<Session> CreateSessionObject();
    bool AddSession(std::shared_ptr<Session> session);
    void CloseSession(SessionIdType id);

    std::shared_ptr<Session> GetSession(SessionIdType id);

private:
    std::mutex mSessionsLock;
    std::atomic<SessionIdType> mSessionCount{ };
    Concurrency::concurrent_unordered_map<SessionIdType, std::shared_ptr<Session>> mSessions;
};