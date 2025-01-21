#pragma once

#pragma pack(push, 1)
struct PacketHeader {
    PacketSizeType size;
    BYTE type;
    SessionIdType id;
};

struct PacketChat : public PacketHeader {
    char chatdata[256];
};
#pragma pack(pop)