#include "pch.h"
#include "FbsPacketProcessFn.h"
#include "PlayerScript.h"
#include "Input.h"
#include "ServerFrame.h"
#include "GameSession.h"
#include "ObjectManager.h"
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
    //gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Process: {}", Packets::EnumNamePacketTypes(enumType));
    switch (header->type) {
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

    case Packets::PacketTypes_PT_PLAYER_EXIT_CS:
    {
        break;
    }

    case Packets::PacketTypes_PT_PLAYER_SELECT_WEAPON_CS:
    {
        decltype(auto) packetWeapon = FbsPacketFactory::GetDataPtrCS<Packets::PlayerSelectWeaponCS>(buffer);
        ProcessPlayerSelectWeaponCS(session, packetWeapon);
        break;
    }

    case Packets::PacketTypes_PT_PLAYER_SELECT_ROLE_CS:
    {
        decltype(auto) packetRoll = FbsPacketFactory::GetDataPtrCS<Packets::PlayerSelectRoleCS>(buffer);
        ProcessPlayerSelectRoleCS(session, packetRoll);
=========
    case Packets::PacketTypes_PT_PLAYER_SELECT_ROLE_CS:
    {
        decltype(auto) packetRoll = FbsPacketFactory::GetDataPtrCS<Packets::PlayerSelectRoleCS>(buffer);
        ProcessPlayerSelectRoleCS(session, packetRoll);
>>>>>>>>> Temporary merge branch 2
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
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Client Sent Invalid PacketType - Close Session [{}]", header->senderId);
        gServerCore->GetSessionManager()->CloseSession(header->senderId);
        break;
    }
    }

    return buffer + header->size;
}

void ProcessPlayerEnterInGame(std::shared_ptr<class GameSession>& session, const Packets::PlayerEnterInGame* const enter) {
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Player [{}] Enter In Game!", session->GetId());
    session->InitUserObject();
}

void ProcessPlayerInputCS(std::shared_ptr<GameSession>& session, const Packets::PlayerInputCS* const input) {
    const auto userObject = session->GetUserObject();
    if (nullptr == userObject) {
        return;
    }

    auto player = userObject->GetScript<PlayerScript>();
    if (nullptr == player) {
        return;
    }

    player->GetInput()->UpdateInput(input->key(), input->down());
    userObject->Update();
    userObject->LateUpdate();
}

void ProcessPlayerLookCS(std::shared_ptr<GameSession>& session, const Packets::PlayerLookCS* const look) {
    const auto userObject = session->GetUserObject();
    if (nullptr == userObject) {
        return;
    }

    auto lookVec = FbsPacketFactory::GetVector3(look->look());
    userObject->GetTransform()->SetLook(lookVec);
    userObject->Update();
void ProcessPlayerSelectRoleCS(std::shared_ptr<GameSession>& session, const Packets::PlayerSelectRoleCS* const roll) {
<<<<<<<<< Temporary merge branch 1
inline void ProcessPlayerSelectRoleCS(const Packets::PlayerSelectRoleCS* const roll, std::shared_ptr<GameObject>& player) {
    // TODO
=========
void ProcessPlayerSelectRoleCS(std::shared_ptr<GameSession>& session, const Packets::PlayerSelectRoleCS* const roll) {
>>>>>>>>> Temporary merge branch 2
}

void ProcessLatencyCS(std::shared_ptr<GameSession>& session, const Packets::PacketLatencyCS* const latency) {
    auto packetLatency = FbsPacketFactory::PacketLatencySC(latency->latency());
    session->RegisterSend(packetLatency);
}

void ProcessRequestAttackCS(std::shared_ptr<GameSession>& session, const Packets::RequestAttackCS* const attack) {

}

void ProcessRequestUseItemCS(std::shared_ptr<GameSession>& session, const Packets::RequestUseItemCS* const useItem) {
}

void ProcessRequestFireProjectileCS(std::shared_ptr<GameSession>& session, const Packets::RequestFireCS* const fire) {
}
