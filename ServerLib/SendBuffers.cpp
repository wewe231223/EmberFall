#include "pch.h"
#include "SendBuffers.h"

SendBuffers::SendBuffers(size_t bufferSize) {
    mPool.AllocMemBlocks(bufferSize, BUFFER_COUNT);
}

SendBuffers::~SendBuffers() { }

OverlappedSend* SendBuffers::GetOverlapped(void* data, size_t dataSize) {
    void* ptr = mPool.Pop();
    if (nullptr == ptr) {
        return nullptr;
    }

    auto overlappedSend = reinterpret_cast<OverlappedSend*>(ptr);
    overlappedSend->ResetOverlapped();
    ::memcpy(overlappedSend->buffer.data(), data, dataSize);
    ::memset(&(overlappedSend->owner), 0, sizeof(std::shared_ptr<INetworkObject>));
    overlappedSend->type = IOType::SEND;
    overlappedSend->wsaBuf.buf = overlappedSend->buffer.data();
    overlappedSend->wsaBuf.len = dataSize;

    return overlappedSend;
}

bool SendBuffers::ReleaseOverlapped(OverlappedSend* overlapped) {
    return mPool.Push(overlapped);
}

OverlappedSend* SendBufferFactory::GetOverlapped(void* data, size_t dataSize) {
    std::cout << std::format("Pop SendBuffer {}\n", dataSize);
    return mBuffer.GetOverlapped(data, dataSize); // TEST
}

bool SendBufferFactory::ReleaseOverlapped(OverlappedSend* overlapped) {
    std::cout << std::format("push SendBuffer {}\n", overlapped->wsaBuf.len);
    return mBuffer.ReleaseOverlapped(overlapped); // TEST
}
