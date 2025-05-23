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
        session->Close();
        it->second.reset();

        mSessions.unsafe_erase(it);
        mSessionCount.fetch_sub(1);
        gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Session[{}]: erased from session map", id);
    }
}

void SessionManager::ReleaseSessionId(SessionIdType id) {
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Release Session Id: {}", id);
    mSessionIdMap.push(id);
}

std::shared_ptr<Session> SessionManager::GetSession(SessionIdType id) {
    Lock::SRWLockGuard sessionGuard{ Lock::SRWLockMode::SRW_SHARED, mSessionsLock };
    auto it = mSessions.find(id);
    if (it == mSessions.end()) {
        return nullptr;
    }

    auto session = it->second;

    return session;
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

void SessionManager::CheckSessionsHeartBeat(const std::vector<SessionIdType>& sessionsId) {
    std::vector<SessionIdType> timeOutSessions{ };

    auto packetHeartBeat = FbsPacketFactory::HeartBeatSC();
    for (auto id : sessionsId) {
        auto session = GetSession(id);
        if (nullptr == session) {
            continue;
        }

        if (session->mHeartBeat >= MAX_SESSION_HEART_BEAT_CNT) {
            timeOutSessions.push_back(id);
        }

        session->mHeartBeat.fetch_add(1);
        session->RegisterSend(FbsPacketFactory::ClonePacket(packetHeartBeat));
    }
    FbsPacketFactory::ReleasePacketBuf(packetHeartBeat);

    for (auto id : timeOutSessions) {
        CloseSession(id);
    }
}

void SessionManager::CheckSessionsHeartBeat() {
    std::vector<SessionIdType> timeOutSessions{ };

    auto packetHeartBeat = FbsPacketFactory::HeartBeatSC();
    mSessionsLock.ReadLock();
    for (auto [id, session] : mSessions) {
        if (nullptr == session) {
            continue;
        }

        if (session->mHeartBeat >= MAX_SESSION_HEART_BEAT_CNT) {
            timeOutSessions.push_back(id);
        }

        session->mHeartBeat.fetch_add(1);
        session->RegisterSend(packetHeartBeat);
    }
    mSessionsLock.ReadUnlock();
    FbsPacketFactory::ReleasePacketBuf(packetHeartBeat);

    for (auto id : timeOutSessions) {
        CloseSession(id);
    }
}
