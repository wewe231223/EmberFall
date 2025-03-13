#pragma once

using PacketSizeT = uint16_t; // 65535
using PacketTypeT = uint8_t;  // 255
using SenderIdType = SessionIdType;

// Protocol version - ver 1.0
inline uint8_t PROTOCOL_VERSION_MAJOR = 1;
inline uint8_t PROTOCOL_VERSION_MINOR = 0;

// 0x00 - Packet Protocol Version
// 0x01 ~ Client To Server
// 0x81 ~ Server To Client
namespace PacketType {
    enum PacketCommon : PacketTypeT {
        PACKET_PROTOCOL_VERSION = 0x00
    };

    enum PacketCS : PacketTypeT {
        PACKET_KEYINPUT = 0x01,
        PACKET_CAMERA,
        PACKET_REQUEST_ATTACK,
        PACKET_SELECT_ROLE,
        PACKET_SELECT_WEAPON,
    };

    enum PacketSC : PacketTypeT {
        PACKET_NOTIFY_ID = 0x81,
        PACKET_OBJECT,
        PACKET_OBJECT_APPEARED,
        PACKET_OBJECT_DISAPPEARED,
        PACKET_ATTACKED,
        PACKET_OBJECT_DEAD,
        PAKCET_ACQUIRED_ITEM,
        PACKET_USE_ITEM,
        PACKET_RESTORE_HEALTH,
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
    Boss,
    Player,
    Monster1,
    Monster2,
    Monster3,
    Item_HolyWater,
    Item_Cross,
    Item_Potion
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
}
#pragma pack(pop)