#pragma once

using PacketSizeT = uint16_t; // 65535
using PacketTypeT = uint8_t;  // 255
using SenderIdType = SessionIdType;

// Protocol version - ver 1.0
inline constexpr uint8_t PROTOCOL_VERSION_MAJOR = 1;
inline constexpr uint8_t PROTOCOL_VERSION_MINOR = 0;
inline constexpr PacketTypeT PACKET_CS_START = 0x00;
inline constexpr PacketTypeT PACKET_SC_START = 0x81;

// 0x00 - Packet Protocol Version
// 0x01 ~ Client To Server
// 0x81 ~ Server To Client
namespace PacketType {
    enum PacketCS : PacketTypeT {
        PACKET_KEYINPUT = 0x00,
        PACKET_CAMERA,
        PACKET_REQUEST_ATTACK,
        PACKET_SELECT_ROLE,
        PACKET_SELECT_WEAPON,
        PACKET_EXIT,
        PACKET_CS_COUNT // Count Enum
    };

    enum PacketSC : PacketTypeT {
        PACKET_PROTOCOL_VERSION = 0x81,
        PACKET_NOTIFY_ID,
        PACKET_PLAYER,
        PACKET_OBJECT,
        PACKET_OBJECT_APPEARED,
        PACKET_OBJECT_DISAPPEARED,
        PACKET_ATTACKED,
        PACKET_OBJECT_DEAD,
        PAKCET_ACQUIRED_ITEM,
        PACKET_USE_ITEM,
        PACKET_RESTORE_HEALTH,
        PACKET_PLAYER_EXIT,
        PACKET_SC_COUNT // Count Enum
    };
}

#pragma pack(push, 1)
enum Weapon : uint8_t {
    STAFF,
    SWORD,
    BOW,
    SPEAR,
    NONE
};

enum EntityType : uint8_t {
    BOSS,
    PLAYER,
    MONSTER1,
    MONSTER2,
    MONSTER3,
    ITEM_HOLYWATER,
    ITEM_CROSS,
    ITEM_POTION
};

struct PacketHeader {
    PacketSizeT size;
    PacketTypeT type;
    SenderIdType id;
};

struct PacketProtocolVersion : public PacketHeader {
    uint8_t major{ PROTOCOL_VERSION_MAJOR };
    uint8_t minor{ PROTOCOL_VERSION_MINOR };
};

namespace PacketSC { // Server To Client
    struct PacketNotifyId : public PacketHeader { };

    struct PacketPlayer : public PacketHeader { 
        float rotationYaw;
        SimpleMath::Vector3 position;
    };

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