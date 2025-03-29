#pragma once

#include "ProtocolEnums.h"

// Protocol version - ver 1.0
inline constexpr uint8_t PROTOCOL_VERSION_MAJOR = 1;
inline constexpr uint8_t PROTOCOL_VERSION_MINOR = 3;
inline constexpr PacketTypeT PACKET_CS_START = 0x00;
inline constexpr PacketTypeT PACKET_SC_START = 0x81;

#pragma pack(push, 1)
struct PacketHeader {
    PacketSizeT size;
    PacketTypeT type;
    SenderIdType id;
};

struct PacketProtocolVersion : public PacketHeader {
    uint8_t major{ PROTOCOL_VERSION_MAJOR };
    uint8_t minor{ PROTOCOL_VERSION_MINOR };
};

// if ObjectId >= 256 then Object Packet else Player Packet
namespace PacketSC { // Server To Client
    struct PacketNotifyId : public PacketHeader { };

    struct PacketObject : public PacketHeader {
        NetworkObjectIdType objId;
        float rotationYaw;
        SimpleMath::Vector3 position;
    };

    struct PacketObjectAppeared : public PacketHeader {
        NetworkObjectIdType objId;
        EntityType entity;
        float HP;
        float rotationYaw;
        SimpleMath::Vector3 position;
    };

    struct PacketAttacked : public PacketHeader {
        NetworkObjectIdType objId;
        float HP;
    };

    struct PacketObjectDisappeared : public PacketHeader {
        NetworkObjectIdType objId;
    };

    struct PacketObjectDead : public PacketHeader {
        NetworkObjectIdType objId;
    };

    struct PacketAcquireItem : public PacketHeader {
        EntityType item;
    };

    struct PacketUseItem : public PacketHeader {
        EntityType item;
    }; // remove UI

    struct PacketRestoreHP : public PacketHeader {
        float HP;
    };

    struct PacketInteractStart : public PacketHeader {
        EntityType interactionEntityType;
        NetworkObjectIdType playerId;
        NetworkObjectIdType interactObjId;
    };

    struct PacketInteractCancel : public PacketHeader { // 도중에 취소
        NetworkObjectIdType playerId;
    };

    struct PacketInteractFinished : public PacketHeader {
        EntityType interactionEntityType;
        NetworkObjectIdType playerId;
        NetworkObjectIdType interactObjId;
    };

    struct PacketAnimationState : public PacketHeader {
        NetworkObjectIdType objId;
        AnimationState animState;
    };

    struct PacketPlayerExit : public PacketHeader { };
}

namespace PacketCS { // Client To Server
    struct PacketKeyInput : public PacketHeader {
        uint8_t key;
        uint8_t down; // down 여부
    };

    struct PacketCamera : public PacketHeader {
        SimpleMath::Vector3 position;
        SimpleMath::Vector3 look;
    };

    struct PacketRequestAttack : public PacketHeader {
        SimpleMath::Vector3 dir;
    };

    struct PacketSelectRole : public PacketHeader {
        EntityType entity;
    };

    struct PacketSelectWeapon : public PacketHeader {
        Weapon weapon;
    };
    
    struct PacketExit : public PacketHeader { };
}
#pragma pack(pop)