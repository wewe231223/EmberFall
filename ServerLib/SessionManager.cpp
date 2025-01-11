#include "pch.h"
#include "SessionManager.h"
#include "Session.h"
#include "IOCPCore.h"

SessionManager::SessionManager() { }

SessionManager::~SessionManager() { }

std::shared_ptr<Session> SessionManager::CreateSessionObject() {
    return std::make_shared<Session>();
}

bool SessionManager::AddSession(std::shared_ptr<Session> session) {
    auto id = mSessionCount.fetch_add(1); // temp code

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
        mSessions.unsafe_erase(it);
        std::cout << std::format("Session[{}]: erased from session map\n", id);
    }
}

std::shared_ptr<Session> SessionManager::GetSession(SessionIdType id) {
    return mSessions[id];
}