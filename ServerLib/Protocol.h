#pragma once

using PacketTypeIdx = BYTE;

// 0x00 ~ Client To Server
// 0x81 ~ Server To Client
namespace PacketType {
    inline constexpr PacketTypeIdx PT_INPUT_CS = 0x00;
    inline constexpr PacketTypeIdx PT_PLAYER_INFO_CS = 0x01;
    inline constexpr PacketTypeIdx PT_NOTIFYING_ID_SC = 0x81;
    inline constexpr PacketTypeIdx PT_PLAYER_EXIT_SC = 0x82;
    inline constexpr PacketTypeIdx PT_PLAYER_INFO_SC = 0x83;
    inline constexpr PacketTypeIdx PT_GAME_OBJECT_SC = 0x84;
}

#pragma pack(push, 1)
struct PacketHeader {
    PacketSizeType size;
    PacketTypeIdx type;
    SessionIdType id;
};

struct PacketNotifyId : public PacketHeader { };

struct PacketPlayerExit : public PacketHeader { };

struct PacketChat : public PacketHeader {
    char chatdata[256];
};

struct PacketPlayerInfoSC : public PacketHeader {
    SimpleMath::Vector3 color;
    SimpleMath::Vector3 position;
    SimpleMath::Quaternion rotation;
    SimpleMath::Vector3 scale;
};

struct PacketGameObject : public PacketHeader {
    NetworkObjectIdType objectId;
    bool state;
    SimpleMath::Vector3 color;
    SimpleMath::Vector3 position;
    SimpleMath::Quaternion rotation;
    SimpleMath::Vector3 scale;
};

struct PacketPlayerInfoCS : public PacketHeader {
    SimpleMath::Vector3 color;
    SimpleMath::Quaternion rotation;
};

struct PacketInput : public PacketHeader {
    Key key;
};
#pragma pack(pop)