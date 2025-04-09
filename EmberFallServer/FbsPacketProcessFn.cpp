#include "pch.h"
#include "FbsPacketProcessFn.h"
#include "ServerGameScene.h"

#include "PlayerScript.h"

inline void ProcessPackets(std::shared_ptr<IServerGameScene>& gameScene, const uint8_t* buffer, size_t bufSize) {
    const uint8_t* iter = buffer;
    while (iter < buffer + bufSize) {
        iter = ProcessPacket(gameScene, buffer);
    }
}

inline const uint8_t* ProcessPacket(std::shared_ptr<IServerGameScene>& gameScene, const uint8_t* buffer) {
    decltype(auto) header = FbsPacketFactory::GetHeaderPtrCS(buffer);
    decltype(auto) sender = gameScene->GetObjectFromId(header->senderId);

    switch (header->type) {
    case Packets::PacketTypes::PacketTypes_PT_PLAYER_LOOK_CS:
    {
        decltype(auto) packetLook = FbsPacketFactory::GetDataPtrCS<Packets::PlayerLookCS>(buffer);
        ProcessPlayerLookCS(packetLook, sender);
        break;
    }

    case Packets::PacketTypes::PacketTypes_PT_PLAYER_EXIT_CS:
    {
        gameScene->ExitPlayer(header->senderId);
        break;
    }

    case Packets::PacketTypes::PacketTypes_PT_PLAYER_SELECT_WEAPON_CS:
    {
        decltype(auto) packetWeapon = FbsPacketFactory::GetDataPtrCS<Packets::PlayerSelectWeaponCS>(buffer);
        ProcessPlayerSelectWeaponCS(packetWeapon, sender);
        break;
    }

    case Packets::PacketTypes::PacketTypes_PT_PLAYER_SELECT_ROLL_CS:
    {
        decltype(auto) packetRoll = FbsPacketFactory::GetDataPtrCS<Packets::PlayerSelectRollCS>(buffer);
        ProcessPlayerSelectRollCS(packetRoll, sender);
        break;
    }

    case Packets::PacketTypes::PacketTypes_PT_LATENCY_CS:
    {
        decltype(auto) packetLatency = FbsPacketFactory::GetDataPtrCS<Packets::PacketLatencyCS>(buffer);
        ProcessLatencyCS(packetLatency);
        break;
    }


    case Packets::PacketTypes::PacketTypes_PT_REQUEST_ATTACK_CS:
    {
        decltype(auto) packetAttack = FbsPacketFactory::GetDataPtrCS<Packets::RequestAttackCS>(buffer);
        ProcessRequestAttackCS(packetAttack, sender);
        break;
    }

    case Packets::PacketTypes::PacketTypes_PT_REQUEST_FIRE_CS:
    {
        decltype(auto) packetRequestFire = FbsPacketFactory::GetDataPtrCS<Packets::RequestFireCS>(buffer);
        ProcessRequestFireProjectileCS(packetRequestFire, sender);
        break;
    }

    case Packets::PacketTypes::PacketTypes_PT_REQUEST_USE_ITEM_CS:
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

inline void ProcessPlayerLookCS(const Packets::PlayerLookCS* const look, std::shared_ptr<GameObject>& player) {
    auto lookVec = FbsPacketFactory::GetVector3(look->look());
    player->GetTransform()->SetLook(lookVec);
}

inline void ProcessPlayerSelectWeaponCS(const Packets::PlayerSelectWeaponCS* const weapon, std::shared_ptr<GameObject>& player) {
    //player->ChangeWeapon(weapon->weapon());
}

inline void ProcessPlayerSelectRollCS(const Packets::PlayerSelectRollCS* const roll, std::shared_ptr<GameObject>& player) {
    // TODO
}

inline void ProcessLatencyCS(const Packets::PacketLatencyCS* const latency) {

}

inline void ProcessRequestAttackCS(const Packets::RequestAttackCS* const attack, std::shared_ptr<GameObject>& player) {
    player->Attack();
}

inline void ProcessRequestUseItemCS(const Packets::RequestUseItemCS* const useItem, std::shared_ptr<GameObject>& player) {
}

inline void ProcessRequestFireProjectileCS(const Packets::RequestFireCS* const fire, std::shared_ptr<GameObject>& player) {
}
