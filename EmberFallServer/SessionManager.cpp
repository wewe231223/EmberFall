#include "pch.h"
#include "SessionManager.h"

SessionManager::SessionManager() { }

SessionManager::~SessionManager() { 
    mSessions.clear();
}

bool SessionManager::AddSession(OverlappedAccept* acceptInfo) {
    auto session = new GameSession{ acceptInfo->connectedSocket };
    auto id = mSessionIdCount.fetch_add(1);
    if (id == INVALID_SESSION_ID) {
        return false;
    }

    session->InitId(id);
    mSessionCount.fetch_add(1);

    mSessions.insert(std::make_pair(id, session));
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Session[{}]: add in session map", id);

    session->InitSessionNetAddress(acceptInfo->buffer.data());
    auto [ip, port] = session->GetAddress();

    session->OnConnect();

    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Client [IP: {}, PORT: {}] Connected", ip, port);

    return true;
}

bool SessionManager::AddSession(SOCKET socket) {
    auto session = new GameSession{ socket };
    auto id = mSessionIdCount.fetch_add(1);
    if (id == INVALID_SESSION_ID) {
        return false;
    }

    session->InitId(id);
    mSessionCount.fetch_add(1);

    mSessions.insert(std::make_pair(id, session));
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Session[{}]: add in session map", id);

    return true;
}

void SessionManager::CloseSession(SessionIdType id) {
    auto session = mSessions.at(id);
    if (nullptr == session) {
        return;
    }

    if (not session->IsClosed()) {
        session->Close();
        mSessions.at(id) = nullptr;

        mSessionCount.fetch_sub(1);
        gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Session[{}]: erased from session map", id);
    }
}

GameSession* SessionManager::GetSession(SessionIdType id) {
    return mSessions.at(id);
}

Concurrency::concurrent_unordered_map<SessionIdType, GameSession*>& SessionManager::GetSessionMap() {
    return mSessions;
}

void SessionManager::Send(SessionIdType to, OverlappedSend* const overlappedSend) {
    auto session = mSessions.at(to);
    if (nullptr == session or false == session->IsConnected()) {
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
    FbsPacketFactory::ReleasePacketBuf(packetHeartBeat);

    for (auto id : timeOutSessions) {
        CloseSession(id);
    }
}
