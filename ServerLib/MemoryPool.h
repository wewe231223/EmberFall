#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MemoryPool.cpp
// 2025 - 01 - 18 김성준 : new delete 연산을 최대한 줄이기 위한 메모리풀
//      AllocMemBlocks 함수를 통해 미리 할당. Push, Pop을 통해 메모리를 얻어온다.
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MemoryPool {
public:
    using MemBlockIdxType = size_t;

public:
    MemoryPool();
    ~MemoryPool();

    MemoryPool(const MemoryPool&) = delete;
    MemoryPool(MemoryPool&&) noexcept = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;
    MemoryPool& operator=(MemoryPool&&) noexcept = delete;

public:
    bool AllocMemBlocks(size_t memBlockSize, size_t memBlockCount);

    void* Pop();
    bool Push(void* ptr);

private:
    void* mBasePtr{ nullptr };
    size_t mMemBlockSize{ };
    size_t mMemBlockCount{ };
    Concurrency::concurrent_queue<MemBlockIdxType> mAvailableIndices{ };
};

