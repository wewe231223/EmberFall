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
//      02 - 05 : 데드락 문제
//              너무 생각없이 문제를 해결한거 같다.
//              두개의 atomic_bool 변수로 문제를 해결하는 방식이 잘못되었다는 걸 뒤늦게 깨달았다.
//              wait함수와 store 함수 중간에 ContextSwitching이 일어나서 충분히 데드락 상태에 빠질 수 있다.
//              
//              변경 -> 어짜피 Write는 여러 쓰레드가, Swap (Read) 는 하나의 쓰레드만 접근하도록 할 것이니
//                  SRWLock을 사용하면 문제를 해결할 수 있을 것 같다.
//                  Write에서 Shared 모드로, Swap에서 Exclusive 모드로 Locking하면 문제가 해결된다.
//                  어짜피 Non-Blocking으로 문제를 해결할 생각은 없었으니 이 방법을 쓰기로 했다.
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline constexpr size_t RECV_BUF_SIZE = std::numeric_limits<unsigned short>::max() * 10; // 655350

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

    void AdvanceReadPos(size_t size);
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

    PacketHeader* GetCurrPosToHeader();

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
    Lock::SRWLock mBufferLock{ };
    std::array<RecvBuffer, 2> mBuffers{ };
};

