#include "pch.h"
#include "SessionManager.h"
#include "Session.h"
#include "IOCPCore.h"

SessionManager::SessionManager() { 
    for (size_t i = 0; i < MAX_SESSION_VAL; ++i) {
        mSessionIdMap.push(i); // Initialize Id
    }
}

SessionManager::~SessionManager() { 
    mSessions.clear();
}

std::shared_ptr<Session> SessionManager::CreateSessionObject() {
    return std::make_shared<Session>();
}

bool SessionManager::AddSession(std::shared_ptr<Session> session) {
    SessionIdType id{ };
    if (not mSessionIdMap.try_pop(id)) {
        return false;
    }

    mSessionCount.fetch_add(1);

    mSessions[id] = session;
    session->InitId(id);
    std::cout << std::format("Session[{}]: add in session map\n", id);

    gIocpCore->RegisterSocket(session);
    session->RegisterRecv();

    return true;
}

void SessionManager::CloseSession(SessionIdType id) {
    std::lock_guard sessionsGuard{ mSessionsLock };
    auto it = mSessions.find(id);
    if (it != mSessions.end()) {
        mSessionIdMap.push(id);
        mSessions.unsafe_erase(it);
        mSessionCount.fetch_sub(1);
        std::cout << std::format("Session[{}]: erased from session map\n", id);
    }
}

std::shared_ptr<Session> SessionManager::GetSession(SessionIdType id) {
    return mSessions[id];
}