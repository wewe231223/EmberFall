#pragma once

class MemoryPool {
public:
    MemoryPool();
    ~MemoryPool();

public:
    void AllocMemBlocks(size_t memBlockSize, size_t memBlockCount);

private:
    void* mBasePtr{ nullptr };
};

