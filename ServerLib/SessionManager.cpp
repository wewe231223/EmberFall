#include "pch.h"
#include "SessionManager.h"
#include "Session.h"

SessionManager::SessionManager() { }

SessionManager::~SessionManager() { }

std::shared_ptr<Session> SessionManager::CreateSessionObject() {
    return std::make_shared<Session>();
}

bool SessionManager::AddSession(SessionIdType id, std::shared_ptr<Session> session) {
    mSessions[id] = session;
    return true;
}

void SessionManager::CloseSession(SessionIdType id) {
    std::lock_guard sessionsGuard{ mSessionsLock };
    auto it = mSessions.find(id);
    if (it != mSessions.end()) {
        mSessions.unsafe_erase(it);
    }
}

std::shared_ptr<Session> SessionManager::GetSession(SessionIdType id) {
    return mSessions[id];
}