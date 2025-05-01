#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////
//
// SessionManager
// 2025 - 01 - 14 (설명 추가 날짜)
//      김성준: Session객체 관리
//              Send, SendAll을 통해 Send 작업 등록
// 
// ID
//      Session의 ID는 MAX_CLIENT 만큼 concurrent_queue에 미리 저장
//      try_pop이 실패하면 모든 Session이 연결되어 있는 것 -> 더이상 연결 X
// 
// 01 - 14 
//      Session Map에 대한 Lock을 SRWLock으로 변경
//      Send 함수에서는 Write 연산 X 동시에 여러 Send 작업이 수행될 수 있도록 변경
// 
// 02 - 03
//      SessionManager에서 새로운 Session이 추가/삭제될 때 추가로 수행할 함수를 등록하고
//      실행될 수 있도록 함.
//      등록된 함수에서는 멀티 쓰레드와 관련해서 어떤 안전장치도 기대 X
//      단, 추가와 삭제 그리고 Send 연산은 동시에 일어나지 않는다는 것은 보장됨 (Locking 하고 있으므로)
// 
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Session;

//const std::function<std::shared_ptr<Session>()> DEFAULT_CREATE_SESSION_FN = []() { return std::make_shared<Session>(); };

class SessionManager {
    inline constexpr static size_t MAX_SESSION_VAL = std::numeric_limits<SessionIdType>::max();
    inline constexpr static size_t MAX_CLIENT_SIZE = 10;

public:
    SessionManager(std::shared_ptr<class ServerCore> coreService);
    ~SessionManager();

    SessionManager(const SessionManager&) = delete;
    SessionManager(SessionManager&&) noexcept = delete;
    SessionManager& operator=(const SessionManager&) = delete;
    SessionManager& operator=(SessionManager&&) noexcept = delete;

public:
    void RegisterCreateSessionFn(std::function<std::shared_ptr<Session>()>&& fn);
    std::shared_ptr<Session> CreateSessionObject();

    bool AddSession(std::shared_ptr<Session> session);
    void CloseSession(SessionIdType id);

    void ReleaseSessionId(SessionIdType id);

    std::shared_ptr<Session> GetSession(SessionIdType id);
    Concurrency::concurrent_unordered_map<SessionIdType, std::shared_ptr<Session>>& GetSessionMap();

    void Send(SessionIdType to, OverlappedSend* const overlappedSend);

private:
    std::function<std::shared_ptr<Session>()> mCreateSessionFn{ };

    std::shared_ptr<class ServerCore> mCoreService{ nullptr };
    Lock::SRWLock mSessionsLock{ }; // 01-14 std::mutex -> SRWLock으로 변경
    std::atomic<SessionIdType> mSessionCount{ };
    Concurrency::concurrent_unordered_map<SessionIdType, std::shared_ptr<Session>> mSessions{ };

    /* 세션에 대한 Id 들을 미리 담아놓기 위한 Queue */
    Concurrency::concurrent_queue<SessionIdType> mSessionIdMap{ };
};