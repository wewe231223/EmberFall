#include "pch.h"
#include "MemoryPool.h"

MemoryPool::MemoryPool() { }

MemoryPool::~MemoryPool() { 
    mAvailableIndices.clear();
    if (nullptr != mBasePtr) {
        ::_aligned_free(mBasePtr);
    }
}

bool MemoryPool::AllocMemBlocks(size_t memBlockSize, size_t memBlockCount) {
    if (0 == memBlockCount or 32 > memBlockSize) {
        return false;
    }

    mMemBlockCount = memBlockCount;
    mMemBlockSize = memBlockSize;

    mBasePtr = ::_aligned_malloc(mMemBlockSize * mMemBlockCount, MEMORY_ALLOCATION_ALIGNMENT);
    if (nullptr == mBasePtr) {
        return false;
    }

    for (MemBlockIdxType blockCnt{ }; blockCnt < memBlockCount; ++blockCnt) {
        mAvailableIndices.push(blockCnt);
    }

    return true;
}

void* MemoryPool::Pop() {
    size_t availableIndex{ };
    if (false == mAvailableIndices.try_pop(availableIndex)) {
        return nullptr; // 더이상 사용할 수 있는 메모리블록이 없는경우
    }

    return static_cast<char*>(mBasePtr) + (availableIndex * mMemBlockSize);
}

bool MemoryPool::Push(void* ptr) {
    ptrdiff_t dist{ static_cast<char*>(ptr) - static_cast<char*>(mBasePtr) };
    if (dist < 0 or dist >= static_cast<ptrdiff_t>(mMemBlockCount * mMemBlockSize)) {
        return false;
    }

    size_t index{ static_cast<size_t>(dist) / mMemBlockSize };
    mAvailableIndices.push(index);
    return true;
}
