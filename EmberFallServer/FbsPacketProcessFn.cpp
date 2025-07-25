#include "pch.h"
#include "FbsPacketProcessFn.h"
#include "PlayerScript.h"
#include "Input.h"
#include "ServerFrame.h"
#include "GameSession.h"
#include "ObjectManager.h"
#include "GameRoom.h"
#include "Sector.h"

void ProcessPackets(GameSession* session, const uint8_t* const buffer, size_t bufSize) {
    const uint8_t* iter = buffer;
    while (iter < buffer + bufSize) {
        iter = ProcessPacket(session, iter);
    }
}

const uint8_t* ProcessPacket(GameSession* session, const uint8_t* buffer) {
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
    }
    break;

    case Packets::PacketTypes_PT_PLAYER_INPUT_CS:
    {
        decltype(auto) packetInput = FbsPacketFactory::GetDataPtrCS<Packets::PlayerInputCS>(buffer);
        ProcessPlayerInputCS(session, packetInput);
    }
    break;

    case Packets::PacketTypes_PT_PLAYER_LOOK_CS:
    {
        decltype(auto) packetLook = FbsPacketFactory::GetDataPtrCS<Packets::PlayerLookCS>(buffer);
        ProcessPlayerLookCS(session, packetLook);
    }
    break;

    case Packets::PacketTypes_PT_PLAYER_ENTER_INGAME:
    {
        decltype(auto) packetEnter = FbsPacketFactory::GetDataPtrCS<Packets::PlayerEnterInGame>(buffer);
        ProcessPlayerEnterInGame(session, packetEnter);
    }
    break;

    case Packets::PacketTypes_PT_PLAYER_READY_IN_LOBBY_CS:
    {
        decltype(auto) packetReady = FbsPacketFactory::GetDataPtrCS<Packets::PlayerReadyInLobbyCS>(buffer);
        ProcessPlayerReadyInLobby(session, packetReady);
    }
    break;

    case Packets::PacketTypes_PT_PLAYER_CANCEL_READY_CS:
    {
        decltype(auto) packetCencelReady = FbsPacketFactory::GetDataPtrCS<Packets::PlayerCancelReadyCS>(buffer);
        ProcessPlayerCancelReady(session, packetCencelReady);
    }
    break;

    case Packets::PacketTypes_PT_PLAYER_ENTER_IN_LOBBY_CS:
    {
        decltype(auto) packetEnter = FbsPacketFactory::GetDataPtrCS<Packets::PlayerEnterInLobbyCS>(buffer);
        ProcessPlayerEnterInLobby(session, packetEnter);
    }
    break;

    case Packets::PacketTypes_PT_PLAYER_EXIT_CS:
    {
        decltype(auto) packetExit = FbsPacketFactory::GetDataPtrCS<Packets::PlayerExitCS>(buffer);
        ProcessPlayerExitCS(session, packetExit);
    }
    break;

    case Packets::PacketTypes_PT_PLAYER_SELECT_ROLE_CS:
    {
        decltype(auto) packetRoll = FbsPacketFactory::GetDataPtrCS<Packets::PlayerSelectRoleCS>(buffer);
        ProcessPlayerSelectRoleCS(session, packetRoll);
    }
    break;

    case Packets::PacketTypes_PT_LATENCY_CS:
    {
        decltype(auto) packetLatency = FbsPacketFactory::GetDataPtrCS<Packets::PacketLatencyCS>(buffer);
        ProcessLatencyCS(session, packetLatency);
    }
    break;

    case Packets::PacketTypes_PT_REQUEST_ATTACK_CS:
    {
        decltype(auto) packetAttack = FbsPacketFactory::GetDataPtrCS<Packets::RequestAttackCS>(buffer);
        ProcessRequestAttackCS(session, packetAttack);
    }
    break;

    case Packets::PacketTypes_PT_REQUEST_FIRE_CS:
    {
        decltype(auto) packetRequestFire = FbsPacketFactory::GetDataPtrCS<Packets::RequestFireCS>(buffer);
        ProcessRequestFireProjectileCS(session, packetRequestFire);
    }
    break;

    case Packets::PacketTypes_PT_REQUEST_USE_ITEM_CS:
    {
        decltype(auto) packetUseItem = FbsPacketFactory::GetDataPtrCS<Packets::RequestUseItemCS>(buffer);
        ProcessRequestUseItemCS(session, packetUseItem);
    }
    break;

    case Packets::PacketTypes_PT_TEST_CHANGE_TO_NEXT_SCENE_CS:
    {
        ProcessTestChangeToNextSceneCS(session);
    }
    break;

    default:
    {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Client Sent Invalid PacketType - Close Session [{}]", session->GetId());
        gServerFrame->CloseSession(static_cast<SessionIdType>(session->GetId()));
    }
    break;
    }

    return buffer + header->size;
}

void ProcessHeartBeatCS(GameSession* session, const Packets::HeartBeatCS* const heartbeat) {
    session->mHeartBeat.fetch_sub(1);
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Player [{}] HeartBeat : {}", session->GetId(), session->mHeartBeat.load());
}

void ProcessPlayerEnterInLobby(GameSession* session, const Packets::PlayerEnterInLobbyCS* const enter) {
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

        auto otherSession = gServerFrame->GetSession(otherSessionId);
        if (nullptr == otherSession or SESSION_INLOBBY != otherSession->GetSessionState()) {
            continue;
        }

        if (false == otherSession->RegisterSend(FbsPacketFactory::ClonePacket(packetEnter))) {
            gServerFrame->CloseSession(otherSessionId);
            continue;
        }

        auto oldUserEnter = FbsPacketFactory::PlayerEnterInLobbySC(otherSessionId, otherSession->GetSlotIndex(),
            otherSession->GetReadyState(), otherSession->GetPlayerRole(), otherSession->GetNameView());

        if (false == session->RegisterSend(oldUserEnter)) {
            gServerFrame->CloseSession(sessionId);
            break;
        }
    }

    if (false == session->RegisterSend(packetEnter)) {
        gServerFrame->CloseSession(sessionId);
        return;
    }

    session->EnterLobby();
}

void ProcessPlayerReadyInLobby(GameSession* session, const Packets::PlayerReadyInLobbyCS* const ready) {
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

void ProcessPlayerCancelReady(GameSession* session, const Packets::PlayerCancelReadyCS* const cencelReady) {
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

void ProcessPlayerExitCS(GameSession* session, const Packets::PlayerExitCS* const exit) {
    auto state = session->GetSessionState();
    if (SESSION_INLOBBY != state) {
        return;
    }

    auto sessionId = static_cast<SessionIdType>(session->GetId());
    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Player [{}] Exit In Game!", sessionId);

    gServerFrame->CloseSession(sessionId);
}

void ProcessPlayerEnterInGame(GameSession* session, const Packets::PlayerEnterInGame* const enter) {
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Player [{}] Enter In Game!", session->GetId());
    auto stage = gGameRoomManager->GetRoom(session->GetMyRoomIdx())->GetStage().GetStageIdx();
    session->EnterInGame(stage);
}

void ProcessPlayerInputCS(GameSession* session, const Packets::PlayerInputCS* const input) {
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

void ProcessPlayerLookCS(GameSession* session, const Packets::PlayerLookCS* const look) {
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

void ProcessPlayerSelectRoleCS(GameSession* session, const Packets::PlayerSelectRoleCS* const role) {
    auto sessionId = static_cast<SessionIdType>(session->GetId());
    auto sessionGameRoom = session->GetMyRoomIdx();
    auto success = gGameRoomManager->GetRoom(sessionGameRoom)->ChangeRolePlayer(sessionId, role->role());
    if (not success) {
        auto packetRejectSelection = FbsPacketFactory::RejectSelectionRoleSC();
        if (false == session->RegisterSend(packetRejectSelection)) {
            gServerFrame->CloseSession(session->GetId());
        }
        return;
    }

    auto packetConfirmSelection = FbsPacketFactory::ConfirmSelectoinRoleSC();
    if (false == session->RegisterSend(packetConfirmSelection)) {
        gServerFrame->CloseSession(session->GetId());
        return;
    }

    auto packetChangeRole = FbsPacketFactory::PlayerChangeRoleSC(sessionId, role->role());

    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Player [{}] Change Role: {}!", session->GetId(), Packets::EnumNamePlayerRole(role->role()));
    gGameRoomManager->GetRoom(sessionGameRoom)->BroadCast(sessionId, packetChangeRole);
}

void ProcessLatencyCS(GameSession* session, const Packets::PacketLatencyCS* const latency) {
    auto packetLatency = FbsPacketFactory::PacketLatencySC(latency->latency());
    if (false == session->RegisterSend(packetLatency)) {
        gServerFrame->CloseSession(session->GetId());
    }
}

void ProcessRequestAttackCS(GameSession* session, const Packets::RequestAttackCS* const attack) {
    auto sessionState = session->GetSessionState();
    if (SESSION_INGAME != sessionState) {
        return;
    }

    const auto userObject = session->GetUserObject();
    if (nullptr == userObject) {
        return;
    }

    auto dir = FbsPacketFactory::GetVector3(attack->dir());
    userObject->Attack(-dir);
}

void ProcessRequestUseItemCS(GameSession* session, const Packets::RequestUseItemCS* const useItem) {

}

void ProcessRequestFireProjectileCS(GameSession* session, const Packets::RequestFireCS* const fire) {

}

void ProcessTestChangeToNextSceneCS(GameSession* session) {
    decltype(auto) room = gGameRoomManager->GetRoom(session->GetMyRoomIdx());

    if (GameRoomState::GAME_ROOM_STATE_TRANSITION != room->GetGameRoomState()) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Process Change Next Scene");
        room->DebugChangeToNextStage();
    }
}
