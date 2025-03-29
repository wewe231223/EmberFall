#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////
//
// PacketFactory.h
// 2025 - 03 - 13 : PacketFactory
// 
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "MemoryPool.h"

template <typename PacketT> requires std::derived_from<PacketT, PacketHeader> 
inline constexpr auto ConvertPacketTypeToEnum() noexcept
{
    // Client To Server
    if constexpr (std::is_same_v<PacketT, PacketCS::PacketCamera>) {
        return PacketType::PacketCS::PACKET_CAMERA;
    }
    else if constexpr (std::is_same_v<PacketT, PacketCS::PacketExit>) {
        return PacketType::PacketCS::PACKET_EXIT;
    }
    else if constexpr (std::is_same_v<PacketT, PacketCS::PacketKeyInput>) {
        return PacketType::PacketCS::PACKET_KEYINPUT;
    }
    else if constexpr (std::is_same_v<PacketT, PacketCS::PacketRequestAttack>) {
        return PacketType::PacketCS::PACKET_REQUEST_ATTACK;
    }
    else if constexpr (std::is_same_v<PacketT, PacketCS::PacketSelectRole>) {
        return PacketType::PacketCS::PACKET_SELECT_ROLE;
    }
    else if constexpr (std::is_same_v<PacketT, PacketCS::PacketSelectWeapon>) {
        return PacketType::PacketCS::PACKET_SELECT_WEAPON;
    }
    // Server To Client
    else if constexpr (std::is_same_v<PacketT, PacketSC::PacketAttacked>) {
        return PacketType::PacketSC::PACKET_ATTACKED;
    }
    else if constexpr (std::is_same_v<PacketT, PacketSC::PacketPlayerExit>) {
        return PacketType::PacketSC::PACKET_PLAYER_EXIT;
    }
    else if constexpr (std::is_same_v<PacketT, PacketSC::PacketNotifyId>) {
        return PacketType::PacketSC::PACKET_NOTIFY_ID;
    }
    else if constexpr (std::is_same_v<PacketT, PacketSC::PacketObject>) {
        return PacketType::PacketSC::PACKET_OBJECT;
    }
    else if constexpr (std::is_same_v<PacketT, PacketSC::PacketObjectAppeared>) {
        return PacketType::PacketSC::PACKET_OBJECT_APPEARED;
    }
    else if constexpr (std::is_same_v<PacketT, PacketSC::PacketObjectDisappeared>) {
        return PacketType::PacketSC::PACKET_OBJECT_DISAPPEARED;
    }
    else if constexpr (std::is_same_v<PacketT, PacketProtocolVersion>) {
        return PacketType::PacketSC::PACKET_PROTOCOL_VERSION;
    }
    else if constexpr (std::is_same_v<PacketT, PacketSC::PacketRestoreHP>) {
        return PacketType::PacketSC::PACKET_RESTORE_HEALTH;
    }
    else if constexpr (std::is_same_v<PacketT, PacketSC::PacketUseItem>) {
        return PacketType::PacketSC::PACKET_USE_ITEM;
    }
    else if constexpr (std::is_same_v<PacketT, PacketSC::PacketObjectDead>) {
        return PacketType::PacketSC::PACKET_OBJECT_DEAD;
    }
    else if constexpr (std::is_same_v<PacketT, PacketSC::PacketAcquireItem>) {
        return PacketType::PacketSC::PACKET_ACQUIRED_ITEM;
    }
    else if constexpr (std::is_same_v<PacketT, PacketSC::PacketInteractStart>) {
        return PacketType::PacketSC::PACKET_INTERACTION_START;
    }
    else if constexpr (std::is_same_v<PacketT, PacketSC::PacketInteractCancel>) {
        return PacketType::PacketSC::PACKET_INTERACTION_CANCEL;
    }
    else if constexpr (std::is_same_v<PacketT, PacketSC::PacketInteractFinished>) {
        return PacketType::PacketSC::PACKET_INTERACTION_FINISH;
    }
    else if constexpr (std::is_same_v<PacketT, PacketSC::PacketAnimationState>) {
        return PacketType::PacketSC::PACKET_ANIM_STATE;
    }
    else {
        static_assert(false, "Packet Type is not exists!!!");
    }
}

//template <typename PacketType, typename... Args> requires std::derived_from<PacketType, PacketHeader>
//inline PacketType GetPacket(SessionIdType id, Args&&... args) 
//{
//    return PacketType{ sizeof(PacketType), ConvertPacketTypeToEnum<PacketType>(), id, std::forward<Args>(args)... };
//}
