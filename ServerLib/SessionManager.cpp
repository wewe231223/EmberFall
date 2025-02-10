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

std::shared_ptr<Session> SessionManager::CreateSessionObject() {
    return std::make_shared<Session>(mCoreService);
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
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Session[{}]: add in session map\n", id);

    mCoreService->GetIOCPCore()->RegisterSocket(session);

    mOnSessionConnFn(id);

    return true;
}

void SessionManager::CloseSession(SessionIdType id) {
    Lock::SRWLockGuard sessionsGuard{ Lock::SRWLockMode::SRW_EXCLUSIVE, mSessionsLock };
    auto it = mSessions.find(id);
    if (it != mSessions.end()) {
        mSessionIdMap.push(id);
        mSessions.unsafe_erase(it);
        mSessionCount.fetch_sub(1);
        gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Session[{}]: erased from session map", id);

        mOnSessionDisconnFn(id);
    }
}

void SessionManager::RegisterOnSessionConnect(std::function<void(SessionIdType)>&& fn) {
    mOnSessionConnFn = fn;
}

void SessionManager::RegisterOnSessionDisconnect(std::function<void(SessionIdType)>&& fn) {
    mOnSessionDisconnFn = fn;
}

std::shared_ptr<Session> SessionManager::GetSession(SessionIdType id) {
    return mSessions[id];
}

void SessionManager::Send(SessionIdType to, void* packet) {
    Lock::SRWLockGuard sessionsGuard{ Lock::SRWLockMode::SRW_SHARED, mSessionsLock };
    auto it = mSessions.find(to);
    if (it == mSessions.end()) {
        return;
    }

    auto session = it->second;
    if (false == session->IsConnected()) {
        return;
    }

    session->RegisterSend(packet);
}

void SessionManager::SendAll(void* packet) {
    Lock::SRWLockGuard sessionsGuard{ Lock::SRWLockMode::SRW_SHARED, mSessionsLock };
    for (auto& [id, session] : mSessions) {
        if (false == session->IsConnected()) {
            continue;
        }

        session->RegisterSend(packet);
    }
}

void SessionManager::SendAll(void* data, size_t size) {
    Lock::SRWLockGuard sessionsGuard{ Lock::SRWLockMode::SRW_SHARED, mSessionsLock };
    for (auto& [id, session] : mSessions) {
        if (false == session->IsConnected()) {
            continue;
        }

        session->RegisterSend(data, size);
    }
}

