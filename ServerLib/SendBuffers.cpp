#include "pch.h"
#include "SendBuffers.h"

SendBuffers::SendBuffers() { } 
SendBuffers::~SendBuffers() { }

void SendBuffers::Init(size_t bufferSize) {
    mPool.AllocMemBlocks(sizeof(OverlappedEx) + bufferSize, BUFFER_COUNT);
}

OverlappedSend* SendBuffers::GetOverlapped(const PacketHeaderSC* const header, const uint8_t* const payload, const PacketSizeT payloadSize) {
    void* ptr = mPool.Pop();
    if (nullptr == ptr) {
        return nullptr;
    }

    auto buf = reinterpret_cast<char*>(ptr) + sizeof(OverlappedSend);
    auto overlappedSend = reinterpret_cast<OverlappedSend*>(ptr);

    overlappedSend->ResetOverlapped();
    ::memcpy(buf, header, sizeof(PacketHeaderSC));
    ::memcpy(buf + sizeof(PacketHeaderSC), payload, payloadSize);
    ::memset(&(overlappedSend->owner), 0, sizeof(std::shared_ptr<INetworkObject>));
    overlappedSend->type = IOType::SEND;
    overlappedSend->wsaBuf.buf = buf;
    overlappedSend->wsaBuf.len = static_cast<UINT32>(payloadSize + sizeof(PacketHeaderSC));

    return overlappedSend;
}

OverlappedSend* SendBuffers::GetOverlapped(const PacketHeaderCS* const header, const uint8_t* const payload, const PacketSizeT payloadSize) {
    void* ptr = mPool.Pop();
    if (nullptr == ptr) {
        return nullptr;
    }

    auto buf = reinterpret_cast<char*>(ptr) + sizeof(OverlappedSend);
    auto overlappedSend = reinterpret_cast<OverlappedSend*>(ptr);

    overlappedSend->ResetOverlapped();
    ::memcpy(buf, header, sizeof(PacketHeaderCS));
    ::memcpy(buf + sizeof(PacketHeaderCS), payload, payloadSize);
    ::memset(&(overlappedSend->owner), 0, sizeof(std::shared_ptr<INetworkObject>));
    overlappedSend->type = IOType::SEND;
    overlappedSend->wsaBuf.buf = buf;
    overlappedSend->wsaBuf.len = static_cast<UINT32>(payloadSize + sizeof(PacketHeaderCS));

    return overlappedSend;
}

OverlappedSend* SendBuffers::GetOverlapped(OverlappedSend* const srcOverlapped) {
    void* ptr = mPool.Pop();
    if (nullptr == ptr) {
        return nullptr;
    }

    auto buf = reinterpret_cast<char*>(ptr) + sizeof(OverlappedSend);
    auto overlappedSend = reinterpret_cast<OverlappedSend*>(ptr);

    overlappedSend->ResetOverlapped();
    auto copySrcBuf = srcOverlapped->wsaBuf.buf;
    auto copySrcLen = srcOverlapped->wsaBuf.len;

    ::memcpy(buf, copySrcBuf, copySrcLen);
    ::memset(&(overlappedSend->owner), 0, sizeof(std::shared_ptr<INetworkObject>));

    if (nullptr != srcOverlapped->owner) {
        overlappedSend->owner = srcOverlapped->owner;
    }

    overlappedSend->type = IOType::SEND;
    overlappedSend->wsaBuf.buf = buf;
    overlappedSend->wsaBuf.len = copySrcLen;

    return overlappedSend;
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

bool SendBuffers::ReleaseOverlapped(OverlappedSend* const overlapped) {
    return mPool.Push(overlapped);
}

SendBufferFactory::SendBufferFactory() {
    for (auto i : MEM_BLOCK_SIZES) {
        mBuffers[i].Init(i);
    }
}

SendBufferFactory::~SendBufferFactory() { }

OverlappedSend* SendBufferFactory::GetOverlapped(const PacketHeaderSC* const header, const uint8_t* const payload, const PacketSizeT payloadSize) {
    PacketSizeT dataSize = payloadSize + sizeof(PacketHeaderSC);
    if (dataSize > MEM_BLOCK_SIZES[MEM_BLOCK_SIZE_CNT - 1] or 0 == dataSize) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Send buffer DataSize Error Size: {}", dataSize);
        return nullptr;
    }

#if defined(DEBUG) || defined(_DEBUG) || defined(PRINT_DEBUG_LOG)
    mSendBuffDebugger.fetch_add(1);
#endif
    size_t bufferSize = *std::upper_bound(MEM_BLOCK_SIZES, MEM_BLOCK_SIZES + MEM_BLOCK_SIZE_CNT, dataSize);
    return mBuffers[bufferSize].GetOverlapped(header, payload, payloadSize);
}

OverlappedSend* SendBufferFactory::GetOverlapped(const PacketHeaderCS* const header, const uint8_t* const payload, const PacketSizeT payloadSize) {
    PacketSizeT dataSize = payloadSize + sizeof(PacketHeaderSC);
    if (dataSize > MEM_BLOCK_SIZES[MEM_BLOCK_SIZE_CNT - 1] or 0 == dataSize) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Send buffer DataSize Error Size: {}", dataSize);
        return nullptr;
    }

#if defined(DEBUG) || defined(_DEBUG) || defined(PRINT_DEBUG_LOG)
    mSendBuffDebugger.fetch_add(1);
#endif
    size_t bufferSize = *std::upper_bound(MEM_BLOCK_SIZES, MEM_BLOCK_SIZES + MEM_BLOCK_SIZE_CNT, dataSize);
    return mBuffers[bufferSize].GetOverlapped(header, payload, payloadSize);
}

OverlappedSend* SendBufferFactory::GetOverlapped(OverlappedSend* const srcOverlapped) {
    auto dataSize = srcOverlapped->wsaBuf.len;
    if (dataSize > MEM_BLOCK_SIZES[MEM_BLOCK_SIZE_CNT - 1] or 0 == dataSize) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Send buffer DataSize Error Size: {}", dataSize);
        return nullptr;
    }

#if defined(DEBUG) || defined(_DEBUG) || defined(PRINT_DEBUG_LOG)
    mSendBuffDebugger.fetch_add(1);
#endif
    size_t bufferSize = *std::upper_bound(MEM_BLOCK_SIZES, MEM_BLOCK_SIZES + MEM_BLOCK_SIZE_CNT, dataSize);
    return mBuffers[bufferSize].GetOverlapped(srcOverlapped);
}

OverlappedSend* SendBufferFactory::GetOverlapped(void* data, size_t dataSize) {
    if (dataSize > MEM_BLOCK_SIZES[MEM_BLOCK_SIZE_CNT - 1] or 0 == dataSize) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Send buffer DataSize Error Size: {}", dataSize);
        return nullptr;
    }

#if defined(DEBUG) || defined(_DEBUG) || defined(PRINT_DEBUG_LOG)
    mSendBuffDebugger.fetch_add(1);
#endif
    size_t bufferSize = *std::upper_bound(MEM_BLOCK_SIZES, MEM_BLOCK_SIZES + MEM_BLOCK_SIZE_CNT, dataSize);
    return mBuffers[bufferSize].GetOverlapped(data, dataSize);
}

bool SendBufferFactory::ReleaseOverlapped(OverlappedSend* const overlapped) {
    size_t dataSize = overlapped->wsaBuf.len;
    if (dataSize > MEM_BLOCK_SIZES[MEM_BLOCK_SIZE_CNT - 1] or 0 == dataSize) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "ReleaseFail Send buffer DataSize Error Size: {}", dataSize);
        return false;
    }

    overlapped->owner.reset();
    size_t bufferSize = *std::upper_bound(MEM_BLOCK_SIZES, MEM_BLOCK_SIZES + MEM_BLOCK_SIZE_CNT, dataSize);

#if defined(DEBUG) || defined(_DEBUG) || defined(PRINT_DEBUG_LOG)
    mSendBuffDebugger.fetch_sub(1);
#endif
    return mBuffers[bufferSize].ReleaseOverlapped(overlapped);
}
