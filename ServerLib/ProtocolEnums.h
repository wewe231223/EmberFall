#pragma once

using PacketSizeT = uint16_t; // 65535
using PacketTypeT = uint8_t;  // 255
using SenderIdType = SessionIdType;

// 0x00 ~ Client To Server
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
        PACKET_OBJECT,
        PACKET_OBJECT_APPEARED,
        PACKET_OBJECT_DISAPPEARED,
        PACKET_ANIM_STATE,
        PACKET_ATTACKED,
        PACKET_OBJECT_DEAD,
        PACKET_ACQUIRED_ITEM,
        PACKET_USE_ITEM,
        PACKET_RESTORE_HEALTH,
        PACKET_PLAYER_EXIT,
        PACKET_INTERACTION_START,
        PACKET_INTERACTION_CANCEL,
        PACKET_INTERACTION_FINISH,
        PACKET_SC_COUNT // Count Enum
    };
}

enum AnimationState : uint8_t {
    IDLE,
    MOVE_FORWARD,
    MOVE_BACKWARD,
    MOVE_LEFT,
    MOVE_RIGHT,
    JUMP,
    ATTACKED,
    ATTACK,
    INTERACTION,
    DEAD
};

enum Weapon : uint8_t {
    STAFF,
    SWORD,
    BOW,
    SPEAR,
    NONE
};

enum EntityType : uint8_t {
    ENV,
    BOSS,
    PLAYER,
    CORRUPTED_GEM,
    MONSTER1,
    MONSTER2,
    MONSTER3,
    ITEM_HOLYWATER,
    ITEM_CROSS,
    ITEM_POTION,
    PROJECTILE_ARROW,
};