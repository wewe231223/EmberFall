#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////
//
// SessionManager
// 2025 - 01 - 14 (설명 추가 날짜)
//      김성준: Session객체 관리
//              Send, SendAll을 통해 Send 작업 등록
// 
// 02 - 03
//      SessionManager에서 새로운 Session이 추가/삭제될 때 추가로 수행할 함수를 등록하고
//      실행될 수 있도록 함.
//      등록된 함수에서는 멀티 쓰레드와 관련해서 어떤 안전장치도 기대 X
//      단, 추가와 삭제 그리고 Send 연산은 동시에 일어나지 않는다는 것은 보장됨 (Locking 하고 있으므로)
// 
// 06 - 18
//      더이상 세션을 std::shared_ptr로 관리하지 않도록 수정, 그에 따른 동시성 문제, 삭제문제는
//      ebr 재사용 적용과 unsafe_erase를 호출하지 않음으로써 해결
// 
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "GameSession.h"

class SessionManager {
    inline constexpr static size_t MAX_SESSION_VAL = std::numeric_limits<SessionIdType>::max();
    inline constexpr static size_t MAX_CLIENT_SIZE = 10;

public:
    SessionManager();
    ~SessionManager();

    SessionManager(const SessionManager&) = delete;
    SessionManager(SessionManager&&) noexcept = delete;
    SessionManager& operator=(const SessionManager&) = delete;
    SessionManager& operator=(SessionManager&&) noexcept = delete;

public:
    bool AddSession(OverlappedAccept* acceptInfo);
    bool AddSession(SOCKET socket);
    void CloseSession(SessionIdType id);

    GameSession* GetSession(SessionIdType id);
    Concurrency::concurrent_unordered_map<SessionIdType, GameSession*>& GetSessionMap();

    void Send(SessionIdType to, OverlappedSend* const overlappedSend);

    void CheckSessionsHeartBeat(const std::vector<SessionIdType>& sessionsId);
    void CheckSessionsHeartBeat();

private:
    std::atomic<SessionIdType> mSessionCount{ };
    std::atomic<SessionIdType> mSessionIdCount{ };
    Concurrency::concurrent_unordered_map<SessionIdType, GameSession*> mSessions{ };
};