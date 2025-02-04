#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////
//
// PacketHandler
// 2025 - 01 - 14 
//      김성준: 여러 쓰레드에서 송신한 데이터를 담아두기 위한 새로운 버퍼 정의
//              더블 버퍼링처럼 동작하도록함
// 
//      Main Thread -> ReadOnlyBuffer
//      Worker Threads -> Write on WriteOnlyBuffer
//      
//      Main Thread가 데이터를 읽고자 할 때 WriteOnlyBuffer와 ReadOnlyBuffer 의 교체가 이루어짐
// 
//      주의 사항.
//          1. Swap 함수 호출 이후의 모든 Write 작업은 멈추고 해당 작업을 수행하려는 쓰레드는 Block
//          2. Swap 함수 호출 이전의 모든 Write 작업은 그대로 진행됨. 이 때 Swap 함수를 호출한 쓰레드는 Block
//          3. Swap 함수 호출 이전의 모든 Write 작업이 종료된 것이 보장되었을 때 Swap 함수가 실행됨
//          4. RecvBuffer 클래스가 아닌 PacketHandler의 Reset은 안전을 보장하지 않음.
//                  Swap 함수 호출시 새로운 WriteOnlyBuffer의 Reset은 보장됨.
//          5. RecvBuffer의 move, copy는 가급적 사용 X
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline constexpr size_t RECV_BUF_SIZE = std::numeric_limits<unsigned short>::max();

class RecvBuffer {
public:
    RecvBuffer();
    ~RecvBuffer();

    RecvBuffer(const RecvBuffer& other);
    RecvBuffer(RecvBuffer&& other) noexcept;
    RecvBuffer& operator=(const RecvBuffer& other);
    RecvBuffer& operator=(RecvBuffer&& other) noexcept;

public:
    size_t Size() const;
    size_t GetReadPos() const;
    bool IsReadEnd() const;
    char* Data();

    void Read(void* buffer, size_t size);
    void Write(void* data, size_t size);
    void Reset();

    template <typename T>
    void Read(T& data) {
        auto srcIter = mBuffer.begin() + mReadPos;
        if constexpr (std::is_same_v<T, PacketHeader>) {
            ::memcpy(&data, NetworkUtil::AddressOf(srcIter), sizeof(PacketHeader));
            return;
        }

        auto packetSize = NetworkUtil::GetPacketSizeFromIter(srcIter);
        if (sizeof(T) < packetSize) {
            return;
        }

        ::memcpy(&data, NetworkUtil::AddressOf(srcIter), packetSize);
        mReadPos += sizeof(T);
    }

private:
    std::atomic_ullong mWritePos{ };
    size_t mReadPos{ };
    std::array<char, RECV_BUF_SIZE> mBuffer{ };
};

class PacketHandler {
public:
    PacketHandler();
    ~PacketHandler();

    PacketHandler(const PacketHandler&) = delete;
    PacketHandler(PacketHandler&&) noexcept = delete;
    PacketHandler& operator=(const PacketHandler&) = delete;
    PacketHandler& operator=(PacketHandler&&) noexcept = delete;

public:
    void Write(void* data, size_t size);
    void Swap();
    void Reset(); // unsafe

    RecvBuffer& GetBuffer(bool swap = true);

private:
    UINT8 mReadOnlyIdx{ 0 };
    UINT8 mWriteOnlyIdx{ 1 };
    std::atomic_bool mSwap{ };
    std::atomic_bool mWriting{ false };
    std::atomic_uchar mWriteCount{ };
    std::array<RecvBuffer, 2> mBuffers{ };
};

