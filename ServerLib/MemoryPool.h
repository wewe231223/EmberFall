#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MemoryPool.cpp
// 2025 - 01 - 18 김성준 : new delete 연산을 최대한 줄이기 위한 메모리풀
//      AllocMemBlocks 함수를 통해 미리 할당. Push, Pop을 통해 메모리를 얻어온다.
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class AvailableBlockSize : size_t {
    BLOCK_SIZE_32 = 32,
    BLOCK_SIZE_64 = 64,
    BLOCK_SIZE_128 = 128,
    BLOCK_SIZE_256 = 256,
    BLOCK_SIZE_512 = 512,
    BLOCK_SIZE_1024 = 1024,
};

class MemoryPool {
public:
    using MemBlockIdxType = size_t;

    inline static size_t MIN_BLOCK_SIZE = 32;
    inline static AvailableBlockSize AVAILABLE_BLOCK_SIZES[]{
         AvailableBlockSize::BLOCK_SIZE_32,
         AvailableBlockSize::BLOCK_SIZE_64,
         AvailableBlockSize::BLOCK_SIZE_128,
         AvailableBlockSize::BLOCK_SIZE_256,
         AvailableBlockSize::BLOCK_SIZE_512,
         AvailableBlockSize::BLOCK_SIZE_1024
    };

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

