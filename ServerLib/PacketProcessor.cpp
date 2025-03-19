#include "pch.h"
#include "PacketProcessor.h"

ClientPacketProcessor::ClientPacketProcessor() {
    mProcessFuncArr.fill(NULL_PROCESS_FN);
}

ClientPacketProcessor::~ClientPacketProcessor() { }

void ClientPacketProcessor::RegisterProcessFn(PacketTypeT type, ProcessorFn&& function) {
    mProcessFuncArr[type - PACKET_SC_START] = std::move(function);
}

void ClientPacketProcessor::ProcessPackets(RecvBuffer& recvBuffer) {
    PacketHeader* header{ nullptr };
    while (not recvBuffer.IsReadEnd()) {
        header = recvBuffer.GetCurrPosToHeader();
        mProcessFuncArr[header->type - PACKET_SC_START](header);

        recvBuffer.AdvanceReadPos(header->size);
    }
}

void ClientPacketProcessor::ProcessPacket(PacketHeader* header) {
    mProcessFuncArr[header->type - PACKET_SC_START](header);
}

void ClientPacketProcessor::Clear() {
    mProcessFuncArr.fill(NULL_PROCESS_FN);
}

ServerPacketProcessor::ServerPacketProcessor() { }

ServerPacketProcessor::~ServerPacketProcessor() { }

void ServerPacketProcessor::RegisterProcessFn(PacketTypeT type, ProcessorFn&& function) {
    mProcessFuncArr[type - PACKET_CS_START] = std::move(function);
}

void ServerPacketProcessor::ProcessPackets(RecvBuffer& recvBuffer) {
    PacketHeader* header{ nullptr };
    while (not recvBuffer.IsReadEnd()) {
        header = recvBuffer.GetCurrPosToHeader();
        mProcessFuncArr[header->type - PACKET_CS_START](header);

        recvBuffer.AdvanceReadPos(header->size);
    }
}

void ServerPacketProcessor::ProcessPacket(PacketHeader* header) {
    mProcessFuncArr[header->type - PACKET_CS_START](header);
}

void ServerPacketProcessor::Clear() {
    mProcessFuncArr.fill(NULL_PROCESS_FN);
}