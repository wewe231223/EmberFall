#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////
//
// PacketProcessor.h
// 2025 - 03 - 13 : Packet Type에 따른 처리 함수를 등록하고 패킷 버퍼를 받아 자동으로 처리하는 클래스를 만들기 위함.
//                  Static? Instance?
// 
//                  Packet 처리에는 여러가지 인자가 필요할 수 있음.
//                  모든 타입의 인자를 처리하는 함수를 등록하는 것은 포기... type deduction 때문에 머리아프다..
//                  애초에 type을 지정해주면 타입추론으로 인한 오류는 해결가능하나 처음의 목적과는 맞지 않는다.
// 
//                  그냥 등록할 함수의 타입을 지정해두자
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "PacketHandler.h"
#include <any>

template<typename T>
struct FunctionType {
    using type = void;
};

template<typename Ret, typename Class, typename... Args>
struct FunctionType<Ret(Class::*)(Args...) const> {
    using type = std::function<Ret(Args...)>;
};

template <typename Lambda>
using LambdaFunction = typename FunctionType<decltype(&Lambda::operator())>::type;

template <typename Lambda>
LambdaFunction<Lambda> LambdaToFunction(Lambda lambda)
{
    return lambda;
}

using ProcessorFn = std::function<void(PacketHeader*)>;

inline const ProcessorFn NULL_PROCESS_FN = [](PacketHeader*){ };

inline constexpr size_t SERVER_PROCESSOR_COUNT = PacketType::PacketCS::PACKET_CS_COUNT - PACKET_CS_START;
inline constexpr size_t CLIENT_PROCESSOR_COUNT = PacketType::PacketSC::PACKET_SC_COUNT - PACKET_SC_START;

class ClientPacketProcessor {
public:
    ClientPacketProcessor();
    ~ClientPacketProcessor();

public:
    void RegisterProcessFn(PacketTypeT type, ProcessorFn&& function);
    void ProcessPackets(RecvBuffer& recvBuffer);
    void ProcessPacket(PacketHeader* header);

    void Clear();

private:
    std::array<std::function<void(PacketHeader*)>, CLIENT_PROCESSOR_COUNT> mProcessFuncArr;
};

class ServerPacketProcessor {
public:
    ServerPacketProcessor();
    ~ServerPacketProcessor();

public:
    void RegisterProcessFn(PacketTypeT type, ProcessorFn&& function);
    void ProcessPackets(RecvBuffer& recvBuffer);
    void ProcessPacket(PacketHeader* header);

    void Clear();

private:
    std::array<std::function<void(PacketHeader*)>, SERVER_PROCESSOR_COUNT> mProcessFuncArr;
};