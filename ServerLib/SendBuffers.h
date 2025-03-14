#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////
//
// SendBuffers, SendBufferFactory
// 2025 - 01 - 14 (설명 추가 날짜)
//      김성준: 데이터 송신시 new delete가 빈번하게 일어나는 점을 해결하기 위해 메모리 풀을 이용
//              SendBufferFactory를 전역 객체로 설정하고 패킷이나 송신 데이터의 크기에 따라 다른
//              메모리 풀에서 할당된 메모리를 가져온다.
// 
// 01 - 19 수정
//      SendBuffer는 메모리를 다음과 같이 할당한다 [OverlappedEx | BLOC_SIZE] (* BLOCK_SIZE -> 32, 64, 128 ...)
//      SendBuffer의 Init 함수에서는 (BLOCK_SIZE + sizeof OverlappedEx) * COUNT 만큼 메모리를 할당 받고 
//      GetOverlapped 함수에서 OverlappedSend 구조체를 초기화해서 반환한다.
// 
//      SendBufferFactory는 SendBuffer를 여러 크기로 나누어 관리하고 들어온 데이터의 사이즈에 따라 다른 버퍼를 할당함.
//      ex) dataSize: 38 -> 64 크기의 SendBuffer에서 할당해줌
//      여러개의 SendBuffer는 unordered_map으로 관리한다. (검색시간 O(1))
//          
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "MemoryPool.h"

inline constexpr size_t MEM_BLOCK_SIZE_CNT = sizeof(MEM_BLOCK_SIZES) / sizeof(size_t);

class SendBuffers {
public:
    inline static constexpr size_t BUFFER_COUNT = 10000;

public:
    SendBuffers();
    ~SendBuffers();

    SendBuffers(const SendBuffers&) = delete;
    SendBuffers(SendBuffers&&) noexcept = delete;
    SendBuffers& operator=(const SendBuffers&) = delete;
    SendBuffers& operator=(SendBuffers&&) noexcept = delete;

public:
    void Init(size_t bufferSize);
    OverlappedSend* GetOverlapped(void* data, size_t dataSize);
    bool ReleaseOverlapped(OverlappedSend* overlapped);

private:
    MemoryPool mPool{ };
};

class SendBufferFactory {
public:
    SendBufferFactory();
    ~SendBufferFactory();

public:
    OverlappedSend* GetOverlapped(void* data, size_t dataSize);
    bool ReleaseOverlapped(OverlappedSend* overlapped);

private:
    std::unordered_map<size_t, SendBuffers> mBuffers{ };
};