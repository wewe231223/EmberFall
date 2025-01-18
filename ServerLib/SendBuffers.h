#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////
//
// SendBuffers, SendBufferFactory
// 2025 - 01 - 14 (설명 추가 날짜)
//      김성준: 데이터 송신시 new delete가 빈번하게 일어나는 점을 해결하기 위해 메모리 풀을 이용
//              SendBufferFactory를 전역 객체로 설정하고 패킷이나 송신 데이터의 크기에 따라 다른
//              메모리 풀에서 할당된 메모리를 가져온다.
//          
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "MemoryPool.h"

class SendBuffers {
public:
    inline static constexpr size_t BUFFER_COUNT = 100;

public:
    SendBuffers(size_t bufferSize);
    ~SendBuffers();

    SendBuffers(const SendBuffers&) = delete;
    SendBuffers(SendBuffers&&) noexcept = delete;
    SendBuffers& operator=(const SendBuffers&) = delete;
    SendBuffers& operator=(SendBuffers&&) noexcept = delete;

public:
    OverlappedSend* GetOverlapped(void* data, size_t dataSize);
    bool ReleaseOverlapped(OverlappedSend* overlapped);

private:
    MemoryPool mPool{ };
};

class SendBufferFactory {
public:
    SendBufferFactory() { }
    ~SendBufferFactory() { }

public:
    OverlappedSend* GetOverlapped(void* data, size_t dataSize);
    bool ReleaseOverlapped(OverlappedSend* overlapped);

private:
    // 테스트용으로 1024만 운용
    SendBuffers mBuffer{ 1024 };
};