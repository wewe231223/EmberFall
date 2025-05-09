#include "pch.h"
#include "SessionManager.h"
#include "Session.h"
#include "NetworkCore.h"

SessionManager::SessionManager(std::shared_ptr<ServerCore> coreService) 
    : mCoreService{ coreService } {
    for (size_t i = 0; i < MAX_SESSION_VAL; ++i) {
        mSessionIdMap.push(static_cast<SessionIdType>(i)); // Initialize Id
    }
}

SessionManager::~SessionManager() { 
    mSessions.clear();
}

void SessionManager::RegisterCreateSessionFn(std::function<std::shared_ptr<Session>()>&& fn) {
    mCreateSessionFn = fn;
}

std::shared_ptr<Session> SessionManager::CreateSessionObject() {
    return mCreateSessionFn();
}

bool SessionManager::AddSession(std::shared_ptr<Session> session) {
    SessionIdType id{ };
    if (not mSessionIdMap.try_pop(id)) {
        return false;
    }

    Lock::SRWLockGuard sessionsGuard{ Lock::SRWLockMode::SRW_EXCLUSIVE, mSessionsLock };
    mSessionCount.fetch_add(1);

    mSessions[id] = session;
    session->InitId(id);
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Session[{}]: add in session map", id);

    mCoreService->GetIOCPCore()->RegisterSocket(session);

    return true;
}

void SessionManager::CloseSession(SessionIdType id) {
    Lock::SRWLockGuard sessionsGuard{ Lock::SRWLockMode::SRW_EXCLUSIVE, mSessionsLock };
    auto it = mSessions.find(id);
    if (it == mSessions.end()) {
        return;
    }

    auto session = it->second;
    if (nullptr == session) {
        mSessions.unsafe_erase(it);
        return;
    }

    if (not session->IsClosed()) {
        it->second->Close();
        mSessions.unsafe_erase(it);
        mSessionCount.fetch_sub(1);
        gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Session[{}]: erased from session map", id);
    }
}

void SessionManager::ReleaseSessionId(SessionIdType id) {
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Release Session Id: {}", id);
    mSessionIdMap.push(id);
}

Lock::SRWLock& SessionManager::GetSessionLock() {
    return mSessionsLock;
}

std::shared_ptr<Session> SessionManager::GetSession(SessionIdType id) {
    return mSessions[id];
}

Concurrency::concurrent_unordered_map<SessionIdType, std::shared_ptr<Session>>& SessionManager::GetSessionMap() {
    return mSessions;
}

void SessionManager::Send(SessionIdType to, OverlappedSend* const overlappedSend) {
    Lock::SRWLockGuard sessionsGuard{ Lock::SRWLockMode::SRW_SHARED, mSessionsLock };
    auto it = mSessions.find(to);
    if (it == mSessions.end()) {
        return;
    }

    auto session = it->second;
    if (false == session->IsConnected()) {
        FbsPacketFactory::ReleasePacketBuf(overlappedSend);
        return;
    }

    session->RegisterSend(overlappedSend);
}
