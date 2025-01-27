#pragma once

using PacketTypeIdx = BYTE;

// 0x81 ~ Client To Server
// 0x00 ~ Server To Client
namespace PacketType {
    inline constexpr PacketTypeIdx PT_INPUT_CS = 0x00;   // TEST
    inline constexpr PacketTypeIdx PT_NOTIFYING_ID = 0x81;
    inline constexpr PacketTypeIdx PT_GAMEOBJ_SC = 0x82; // TEST
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
    SimpleMath::Vector3 look; // look 벡터
    SimpleMath::Matrix world; // 4x4 행렬 보내기
};

struct PacketInput : public PacketHeader {
    Key key;
};
#pragma pack(pop)