#include "pch.h"
#include "FbsPacketProcessFn.h"
#include "ServerGameScene.h"
#include "PlayerScript.h"
#include "Input.h"
#include "ServerFrame.h"
#include "GameSession.h"

void ProcessPackets(std::shared_ptr<IServerGameScene>& gameScene, const uint8_t* const buffer, size_t bufSize) {
    const uint8_t* iter = buffer;
    while (iter < buffer + bufSize) {
        iter = ProcessPacket(gameScene, iter);
    }
}

const uint8_t* ProcessPacket(std::shared_ptr<IServerGameScene>& gameScene, const uint8_t* buffer) {
    decltype(auto) header = FbsPacketFactory::GetHeaderPtrCS(buffer);
    decltype(auto) sender = gameScene->GetObjectFromId(header->senderId);
    if (nullptr == sender) {
        return buffer + header->size;
    }

    switch (header->type) {
    case Packets::PacketTypes_PT_PLAYER_INPUT_CS:
    {
        decltype(auto) packetInput = FbsPacketFactory::GetDataPtrCS<Packets::PlayerInputCS>(buffer);
        ProcessPlayerInputCS(packetInput, sender);
        break;
    }

    case Packets::PacketTypes_PT_PLAYER_LOOK_CS:
    {
        decltype(auto) packetLook = FbsPacketFactory::GetDataPtrCS<Packets::PlayerLookCS>(buffer);
        ProcessPlayerLookCS(packetLook, sender);
        break;
    }

    case Packets::PacketTypes_PT_PLAYER_EXIT_CS:
    {
        gameScene->ExitPlayer(header->senderId);
        break;
    }

    case Packets::PacketTypes_PT_PLAYER_SELECT_WEAPON_CS:
    {
        decltype(auto) packetWeapon = FbsPacketFactory::GetDataPtrCS<Packets::PlayerSelectWeaponCS>(buffer);
        ProcessPlayerSelectWeaponCS(packetWeapon, sender);
        break;
    }

    case Packets::PacketTypes_PT_PLAYER_SELECT_ROLE_CS:
    {
        decltype(auto) packetRoll = FbsPacketFactory::GetDataPtrCS<Packets::PlayerSelectRoleCS>(buffer);
        ProcessPlayerSelectRoleCS(packetRoll, sender);
        break;
    }

    case Packets::PacketTypes_PT_LATENCY_CS:
    {
        decltype(auto) packetLatency = FbsPacketFactory::GetDataPtrCS<Packets::PacketLatencyCS>(buffer);
        ProcessLatencyCS(packetLatency);
        break;
    }


    case Packets::PacketTypes_PT_REQUEST_ATTACK_CS:
    {
        decltype(auto) packetAttack = FbsPacketFactory::GetDataPtrCS<Packets::RequestAttackCS>(buffer);
        ProcessRequestAttackCS(packetAttack, sender);
        break;
    }

    case Packets::PacketTypes_PT_REQUEST_FIRE_CS:
    {
        decltype(auto) packetRequestFire = FbsPacketFactory::GetDataPtrCS<Packets::RequestFireCS>(buffer);
        ProcessRequestFireProjectileCS(packetRequestFire, sender);
        break;
    }

    case Packets::PacketTypes_PT_REQUEST_USE_ITEM_CS:
    {
        decltype(auto) packetUseItem = FbsPacketFactory::GetDataPtrCS<Packets::RequestUseItemCS>(buffer);
        ProcessRequestUseItemCS(packetUseItem, sender);
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

void ProcessPlayerInputCS(const Packets::PlayerInputCS* const input, std::shared_ptr<class GameObject>& player) {
    auto id = player->GetId();
    auto inputObj = gServerFrame->GetInputManager()->GetInput(id);
    inputObj->UpdateInput(input->key(), input->down());
}

void ProcessPlayerLookCS(const Packets::PlayerLookCS* const look, std::shared_ptr<GameObject>& player) {
    auto lookVec = FbsPacketFactory::GetVector3(look->look());
    player->GetTransform()->SetLook(lookVec);
}

void ProcessPlayerSelectWeaponCS(const Packets::PlayerSelectWeaponCS* const weapon, std::shared_ptr<GameObject>& player) {
    //player->ChangeWeapon(weapon->weapon());
}

void ProcessPlayerSelectRoleCS(const Packets::PlayerSelectRoleCS* const roll, std::shared_ptr<GameObject>& player) {
    // TODO
}

void ProcessLatencyCS(const Packets::PacketLatencyCS* const latency) {

}

void ProcessRequestAttackCS(const Packets::RequestAttackCS* const attack, std::shared_ptr<GameObject>& player) {
    player->Attack();
}

void ProcessRequestUseItemCS(const Packets::RequestUseItemCS* const useItem, std::shared_ptr<GameObject>& player) {
}

void ProcessRequestFireProjectileCS(const Packets::RequestFireCS* const fire, std::shared_ptr<GameObject>& player) {
}


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

    case Packets::PacketTypes_PT_PLAYER_EXIT_CS:
    {
        gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Process: {}", Packets::EnumNamePacketTypes(enumType));
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
        break;
    }

    case Packets::PacketTypes_PT_LATENCY_CS:
    {
        decltype(auto) packetLatency = FbsPacketFactory::GetDataPtrCS<Packets::PacketLatencyCS>(buffer);
        ProcessLatencyCS(packetLatency);
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

void ProcessPlayerInputCS(std::shared_ptr<GameSession>& session, const Packets::PlayerInputCS* const input) {
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Process: Player");
}

void ProcessPlayerLookCS(std::shared_ptr<GameSession>& session, const Packets::PlayerLookCS* const look) {
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Process: PlayerLook");
}

void ProcessPlayerSelectWeaponCS(std::shared_ptr<GameSession>& session, const Packets::PlayerSelectWeaponCS* const weapon) {
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Process: PlayerSelectWeapon");
}

void ProcessPlayerSelectRoleCS(std::shared_ptr<GameSession>& session, const Packets::PlayerSelectRoleCS* const roll) {
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Process: PlayerSelectRoll");
}

void ProcessLatencyCS(std::shared_ptr<GameSession>& session, const Packets::PacketLatencyCS* const latency) {
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Process: PlayerLatency");
}

void ProcessRequestAttackCS(std::shared_ptr<GameSession>& session, const Packets::RequestAttackCS* const attack) {
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Process: PlayerAttack");
}

void ProcessRequestUseItemCS(std::shared_ptr<GameSession>& session, const Packets::RequestUseItemCS* const useItem) {
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Process: PlayerItem");
}

void ProcessRequestFireProjectileCS(std::shared_ptr<GameSession>& session, const Packets::RequestFireCS* const fire) {
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Process: PlayerFire");
}
