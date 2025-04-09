#pragma once
#include "PacketProtocol_generated.h"

using PacketSizeT = PacketSizeType;
using PacketTypeT = uint8_t;
using SenderIdType = SessionIdType;

inline constexpr uint8_t PROTOCOL_VERSION_MAJOR = 2; 
inline constexpr uint8_t PROTOCOL_VERSION_MINOR = 0;


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
