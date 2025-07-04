#pragma once
#include "PacketProtocol_generated.h"

using PacketSizeT = uint8_t;
using PacketTypeT = uint8_t;
using SenderIdType = uint8_t;

inline constexpr uint16_t SERVER_PORT = 7777;

inline constexpr uint8_t PROTOCOL_VERSION_MAJOR = 2; 
inline constexpr uint8_t PROTOCOL_VERSION_MINOR = 0;

constexpr int32_t SYSTEM_ID = -1;

#pragma pack(push, 1)

struct PacketHeaderSC {
    PacketSizeT size;
    PacketTypeT type;
};

struct PacketHeaderCS {
    PacketSizeT size;
    PacketTypeT type;
    SenderIdType senderId;
};

#pragma pack(pop)
