#pragma once

using PacketTypeIdx = BYTE;

// 0x81 ~ Client To Server
// 0x00 ~ Server To Client
namespace PacketType {
    inline constexpr PacketTypeIdx PT_INPUT_CS = 0x00;   // TEST
    inline constexpr PacketTypeIdx PT_GAMEOBJ_CS = 0x01;
    inline constexpr PacketTypeIdx PT_NOTIFYING_ID_SC = 0x81;
    inline constexpr PacketTypeIdx PT_GAMEOBJ_SC = 0x82;
    inline constexpr PacketTypeIdx PT_COLLISION_SC = 0x83;
}

#pragma pack(push, 1)
struct PacketHeader {
    PacketSizeType size;
    PacketTypeIdx type;
    SessionIdType id;
};

struct PacketNotifyId : public PacketHeader { };

struct PacketChat : public PacketHeader {
    char chatdata[256];
};

struct PacketGameObj : public PacketHeader {
    SimpleMath::Vector3 color;
    SimpleMath::Vector3 position;
    SimpleMath::Quaternion rotation;
    SimpleMath::Vector3 scale;
};

struct PacketGameObjCS : public PacketHeader {
    SimpleMath::Vector3 color;
    SimpleMath::Quaternion rotation;
};

struct PacketInput : public PacketHeader {
    Key key;
};
#pragma pack(pop)