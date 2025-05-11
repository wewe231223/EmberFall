#include "pch.h"
#include "FbsPacketProcessFn.h"
#include "PlayerScript.h"
#include "Input.h"
#include "ServerFrame.h"
#include "GameSession.h"
#include "ObjectManager.h"
#include "GameRoom.h"
#include "Sector.h"

// MultiThread Test
void ProcessPackets(std::shared_ptr<GameSession>& session, const uint8_t* const buffer, size_t bufSize) {
    const uint8_t* iter = buffer;
    while (iter < buffer + bufSize) {
        iter = ProcessPacket(session, iter);
    }
}

const uint8_t* ProcessPacket(std::shared_ptr<GameSession>& session, const uint8_t* buffer) {
    decltype(auto) header = FbsPacketFactory::GetHeaderPtrCS(buffer);
    if (nullptr == session) {
        return buffer + header->size;
    }

    Packets::PacketTypes enumType = static_cast<Packets::PacketTypes>(header->type);
    //gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Process: {}", Packets::EnumNamePacketTypes(enumType));
    switch (header->type) {
    case Packets::PacketTypes_PT_HEART_BEAT_CS:
    {
        decltype(auto) packetHeartBeat = FbsPacketFactory::GetDataPtrCS<Packets::HeartBeatCS>(buffer);
        ProcessHeartBeatCS(session, packetHeartBeat);
        break;
    }

    case Packets::PacketTypes_PT_PLAYER_INPUT_CS:
    {
        decltype(auto) packetInput = FbsPacketFactory::GetDataPtrCS<Packets::PlayerInputCS>(buffer);
        ProcessPlayerInputCS(session, packetInput);
        break;
    }

    case Packets::PacketTypes_PT_PLAYER_LOOK_CS:
    {
        decltype(auto) packetLook = FbsPacketFactory::GetDataPtrCS<Packets::PlayerLookCS>(buffer);
        ProcessPlayerLookCS(session, packetLook);
        break;
    }

    case Packets::PacketTypes_PT_PLAYER_ENTER_INGAME:
    {
        decltype(auto) packetEnter = FbsPacketFactory::GetDataPtrCS<Packets::PlayerEnterInGame>(buffer);
        ProcessPlayerEnterInGame(session, packetEnter);
        break;
    }

    case Packets::PacketTypes_PT_PLAYER_READY_IN_LOBBY_CS:
    {
        decltype(auto) packetReady = FbsPacketFactory::GetDataPtrCS<Packets::PlayerReadyInLobbyCS>(buffer);
        ProcessPlayerReadyInLobby(session, packetReady);
        break;
    }

    case Packets::PacketTypes_PT_PLAYER_CANCEL_READY_CS:
    {
        decltype(auto) packetCencelReady = FbsPacketFactory::GetDataPtrCS<Packets::PlayerCancelReadyCS>(buffer);
        ProcessPlayerCancelReady(session, packetCencelReady);
        break;
    }

    case Packets::PacketTypes_PT_PLAYER_ENTER_IN_LOBBY_CS:
    {
        decltype(auto) packetEnter = FbsPacketFactory::GetDataPtrCS<Packets::PlayerEnterInLobbyCS>(buffer);
        ProcessPlayerEnterInLobby(session, packetEnter);
        break;
    }

    case Packets::PacketTypes_PT_PLAYER_EXIT_CS:
    {
        decltype(auto) packetExit = FbsPacketFactory::GetDataPtrCS<Packets::PlayerExitCS>(buffer);
        ProcessPlayerExitCS(session, packetExit);
        break;
    }

    case Packets::PacketTypes_PT_PLAYER_SELECT_ROLE_CS:
    {
        decltype(auto) packetRoll = FbsPacketFactory::GetDataPtrCS<Packets::PlayerSelectRoleCS>(buffer);
        ProcessPlayerSelectRoleCS(session, packetRoll);
        break;
    }

    case Packets::PacketTypes_PT_LATENCY_CS:
    {
        decltype(auto) packetLatency = FbsPacketFactory::GetDataPtrCS<Packets::PacketLatencyCS>(buffer);
        ProcessLatencyCS(session, packetLatency);
        break;
    }


    case Packets::PacketTypes_PT_REQUEST_ATTACK_CS:
    {
        decltype(auto) packetAttack = FbsPacketFactory::GetDataPtrCS<Packets::RequestAttackCS>(buffer);
        ProcessRequestAttackCS(session, packetAttack);
        break;
    }

    case Packets::PacketTypes_PT_REQUEST_FIRE_CS:
    {
        decltype(auto) packetRequestFire = FbsPacketFactory::GetDataPtrCS<Packets::RequestFireCS>(buffer);
        ProcessRequestFireProjectileCS(session, packetRequestFire);
        break;
    }

    case Packets::PacketTypes_PT_REQUEST_USE_ITEM_CS:
    {
        decltype(auto) packetUseItem = FbsPacketFactory::GetDataPtrCS<Packets::RequestUseItemCS>(buffer);
        ProcessRequestUseItemCS(session, packetUseItem);
        break;
    }

    default:
    {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Client Sent Invalid PacketType - Close Session [{}]", session->GetId());
        gServerCore->GetSessionManager()->CloseSession(static_cast<SessionIdType>(session->GetId()));
        break;
    }
    }

    return buffer + header->size;
}

void ProcessHeartBeatCS(std::shared_ptr<class GameSession>& session, const Packets::HeartBeatCS* const heartbeat) {
    session->mHeartBeat.fetch_sub(1);
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Player [{}] HeartBeat : {}", session->GetId(), session->mHeartBeat.load());
}

void ProcessPlayerEnterInLobby(std::shared_ptr<class GameSession>& session, const Packets::PlayerEnterInLobbyCS* const enter) {
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Player [{}] Enter In Lobby!", session->GetId());

    auto sessionId = static_cast<SessionIdType>(session->GetId());
    auto sessionGameRoom = session->GetMyRoomIdx();
    decltype(auto) sessionLock = gGameRoomManager->GetRoom(sessionGameRoom)->GetSessionLock();

    sessionLock.ReadLock();
    std::unordered_set<SessionIdType> sessionsInGameRoom = gGameRoomManager->GetSessionsInRoom(sessionGameRoom);
    sessionLock.ReadUnlock();

    auto packetEnter = FbsPacketFactory::PlayerEnterInLobbySC(sessionId, session->GetSlotIndex(),
        session->GetReadyState(), session->GetPlayerRole(), session->GetNameView());

    for (auto& otherSessionId : sessionsInGameRoom) {
        if (sessionId == otherSessionId) {
            continue;
        }

        auto otherSession = std::static_pointer_cast<GameSession>(gServerCore->GetSessionManager()->GetSession(otherSessionId));
        if (nullptr == otherSession or SESSION_INLOBBY != otherSession->GetSessionState()) {
            continue;
        }

        otherSession->RegisterSend(FbsPacketFactory::ClonePacket(packetEnter));
        auto oldUserEnter = FbsPacketFactory::PlayerEnterInLobbySC(otherSessionId, otherSession->GetSlotIndex(), 
            otherSession->GetReadyState(), otherSession->GetPlayerRole(), otherSession->GetNameView());

        session->RegisterSend(oldUserEnter);
    }

    session->RegisterSend(packetEnter);
    session->EnterLobby();
}

void ProcessPlayerReadyInLobby(std::shared_ptr<class GameSession>& session, const Packets::PlayerReadyInLobbyCS* const ready) {
    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Player[{}] Ready!", session->GetId());

    auto sessionId = static_cast<SessionIdType>(session->GetId());
    auto sessionGameRoom = session->GetMyRoomIdx();
    auto success = gGameRoomManager->ReadyPlayer(sessionGameRoom, sessionId);
    if (not success) {
        return;
    }

    auto packetReady = FbsPacketFactory::PlayerReadyInLobbySC(sessionId);
    gGameRoomManager->GetRoom(sessionGameRoom)->BroadCast(packetReady);

    gGameRoomManager->GetRoom(sessionGameRoom)->CheckAndStartGame();
}

void ProcessPlayerCancelReady(std::shared_ptr<class GameSession>& session, const Packets::PlayerCancelReadyCS* const cencelReady) {
    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Player[{}] Ready!", session->GetId());

    auto sessionId = static_cast<SessionIdType>(session->GetId());
    auto sessionGameRoom = session->GetMyRoomIdx();
    auto success = gGameRoomManager->CancelPlayerReady(sessionGameRoom, sessionId);
    if (not success) {
        return;
    }

    auto packetReady = FbsPacketFactory::PlayerCancelReadySC(sessionId);
    gGameRoomManager->GetRoom(sessionGameRoom)->BroadCast(packetReady);
}

void ProcessPlayerExitCS(std::shared_ptr<class GameSession>& session, const Packets::PlayerExitCS* const exit) {
    auto state = session->GetSessionState();
    if (SESSION_INLOBBY != state) {
        return;
    }

    auto sessionId = static_cast<SessionIdType>(session->GetId());
    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Player [{}] Exit In Game!", sessionId);

    gServerCore->GetSessionManager()->CloseSession(sessionId);
}

void ProcessPlayerEnterInGame(std::shared_ptr<class GameSession>& session, const Packets::PlayerEnterInGame* const enter) {
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Player [{}] Enter In Game!", session->GetId());
    session->EnterInGame();
}

void ProcessPlayerInputCS(std::shared_ptr<GameSession>& session, const Packets::PlayerInputCS* const input) {
    auto sessionState = session->GetSessionState();
    if (SESSION_INGAME != sessionState) {
        return;
    }

    const auto userObject = session->GetUserObject();
    if (nullptr == userObject) {
        return;
    }

    auto player = userObject->GetScript<PlayerScript>();
    if (nullptr == player) {
        return;
    }

    player->GetInput()->UpdateInput(input->key(), input->down());
  //  userObject->Update();
   // userObject->LateUpdate();
}

void ProcessPlayerLookCS(std::shared_ptr<GameSession>& session, const Packets::PlayerLookCS* const look) {
    auto sessionState = session->GetSessionState();
    if (SESSION_INGAME != sessionState) {
        return;
    }

    const auto userObject = session->GetUserObject();
    if (nullptr == userObject) {
        return;
    }

    auto lookVec = FbsPacketFactory::GetVector3(look->look());
    userObject->GetTransform()->SetLook(lookVec);
    userObject->Update();
    userObject->LateUpdate();
}

void ProcessPlayerSelectRoleCS(std::shared_ptr<GameSession>& session, const Packets::PlayerSelectRoleCS* const role) {
    auto sessionId = static_cast<SessionIdType>(session->GetId());
    auto sessionGameRoom = session->GetMyRoomIdx();
    auto success = gGameRoomManager->GetRoom(sessionGameRoom)->ChangeRolePlayer(sessionId, role->role());
    if (not success) {
        gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Player [{}] Fail Changing Role: {}", session->GetId(), Packets::EnumNamePlayerRole(role->role()));
        auto packetRejectSelection = FbsPacketFactory::RejectSelectionRoleSC();
        session->RegisterSend(packetRejectSelection);
        return;
    }

    auto packetConfirmSelection = FbsPacketFactory::ConfirmSelectoinRoleSC();
    session->RegisterSend(packetConfirmSelection);

    auto packetChangeRole = FbsPacketFactory::PlayerChangeRoleSC(sessionId, role->role());

    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Player [{}] Change Role: {}!", session->GetId(), Packets::EnumNamePlayerRole(role->role()));
    gGameRoomManager->GetRoom(sessionGameRoom)->BroadCast(sessionId, packetChangeRole);
}

void ProcessLatencyCS(std::shared_ptr<GameSession>& session, const Packets::PacketLatencyCS* const latency) {
    auto packetLatency = FbsPacketFactory::PacketLatencySC(latency->latency());
    session->RegisterSend(packetLatency);
}

void ProcessRequestAttackCS(std::shared_ptr<GameSession>& session, const Packets::RequestAttackCS* const attack) {
    auto sessionState = session->GetSessionState();
    if (SESSION_INGAME != sessionState) {
        return;
    }

    const auto userObject = session->GetUserObject();
    if (nullptr == userObject) {
        return;
    }

    auto dir = FbsPacketFactory::GetVector3(attack->dir());
    userObject->Attack(dir);
    userObject->Update();
    userObject->LateUpdate();
}

void ProcessRequestUseItemCS(std::shared_ptr<GameSession>& session, const Packets::RequestUseItemCS* const useItem) {

}

void ProcessRequestFireProjectileCS(std::shared_ptr<GameSession>& session, const Packets::RequestFireCS* const fire) {

}
