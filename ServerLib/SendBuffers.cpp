#include "pch.h"
#include "SendBuffers.h"

SendBuffers::SendBuffers() { } 
SendBuffers::~SendBuffers() { }

void SendBuffers::Init(size_t bufferSize) {
    mPool.AllocMemBlocks(sizeof(OverlappedEx) + bufferSize, BUFFER_COUNT);
}

OverlappedSend* SendBuffers::GetOverlapped(void* data, size_t dataSize) {
    void* ptr = mPool.Pop();
    if (nullptr == ptr) {
        return nullptr;
    }

    auto buf = reinterpret_cast<char*>(ptr) + sizeof(OverlappedSend);
    auto overlappedSend = reinterpret_cast<OverlappedSend*>(ptr);

    overlappedSend->ResetOverlapped();
    ::memcpy(buf, data, dataSize);
    ::memset(&(overlappedSend->owner), 0, sizeof(std::shared_ptr<INetworkObject>));
    overlappedSend->type = IOType::SEND;
    overlappedSend->wsaBuf.buf = buf;
    overlappedSend->wsaBuf.len = static_cast<UINT32>(dataSize);

    return overlappedSend;
}

bool SendBuffers::ReleaseOverlapped(OverlappedSend* overlapped) {
    return mPool.Push(overlapped);
}

SendBufferFactory::SendBufferFactory() {
    for (auto i : MEM_BLOCK_SIZES) {
        mBuffers[i].Init(i);
    }
}

SendBufferFactory::~SendBufferFactory() { }

OverlappedSend* SendBufferFactory::GetOverlapped(void* data, size_t dataSize) {
    if (dataSize > MEM_BLOCK_SIZES[MEM_BLOCK_SIZE_CNT - 1] or 0 == dataSize) {
        return nullptr;
    }

    size_t bufferSize = *std::upper_bound(MEM_BLOCK_SIZES, MEM_BLOCK_SIZES + MEM_BLOCK_SIZE_CNT, dataSize);
    return mBuffers[bufferSize].GetOverlapped(data, dataSize);
}

bool SendBufferFactory::ReleaseOverlapped(OverlappedSend* overlapped) {
    size_t dataSize = overlapped->wsaBuf.len;
    if (dataSize > MEM_BLOCK_SIZES[MEM_BLOCK_SIZE_CNT - 1] or 0 == dataSize) {
        return false;
    }

    size_t bufferSize = *std::upper_bound(MEM_BLOCK_SIZES, MEM_BLOCK_SIZES + MEM_BLOCK_SIZE_CNT, dataSize);

    return mBuffers[bufferSize].ReleaseOverlapped(overlapped);
}
