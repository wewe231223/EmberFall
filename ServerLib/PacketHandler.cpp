#include "pch.h"
#include "PacketHandler.h"

RecvBuffer::RecvBuffer() { }

RecvBuffer::~RecvBuffer() { }

RecvBuffer::RecvBuffer(const RecvBuffer& other) {
    std::copy(other.mBuffer.begin(), other.mBuffer.end(), mBuffer.begin());
    mWritePos = other.mWritePos.load();
    mReadPos = other.mReadPos.load();
}

RecvBuffer::RecvBuffer(RecvBuffer&& other) noexcept {
    std::copy(other.mBuffer.begin(), other.mBuffer.end(), mBuffer.begin());
    mWritePos = other.mWritePos.load();
    mReadPos = other.mReadPos.load();
}

RecvBuffer& RecvBuffer::operator=(const RecvBuffer& other) {
    std::copy(other.mBuffer.begin(), other.mBuffer.end(), mBuffer.begin());
    mWritePos = other.mWritePos.load();
    mReadPos = other.mReadPos.load();

    return *this;
}

RecvBuffer& RecvBuffer::operator=(RecvBuffer&& other) noexcept {
    std::copy(other.mBuffer.begin(), other.mBuffer.end(), mBuffer.begin());
    mWritePos = other.mWritePos.load();
    mReadPos = other.mReadPos.load();

    return *this;
}

char* RecvBuffer::Data() {
    return mBuffer.data();
}

void RecvBuffer::Write(void* data, size_t size) {
    auto dest = mWritePos.fetch_add(size);
    ::memcpy(mBuffer.data() + dest, data, size);
}

void RecvBuffer::Reset() {
    mWritePos.store(0);
    mReadPos.store(0);
}

size_t RecvBuffer::Size() const {
    return mWritePos.load();
}

PacketHandler::PacketHandler() { }

PacketHandler::~PacketHandler() { }

void PacketHandler::Write(void* data, size_t size) {
    mSwap.wait(true);

    mWriting.store(true);

    mWriteCount.fetch_add(1);
    mBuffers[mWriteOnlyIdx].Write(data, size);
    mWriteCount.fetch_sub(1);

    mWriting.store(0 != mWriteCount);
    if (true == mSwap.load() and false == mWriting.load()) {
        std::cout << "notifying Writing End" << std::endl;
        mWriting.notify_all();
    }
}

void PacketHandler::Swap() {
    mSwap.store(true);
    mWriting.wait(true);

    std::swap(mWriteOnlyIdx, mReadOnlyIdx);
    mBuffers[mWriteOnlyIdx].Reset();

    mSwap.store(false);
    mSwap.notify_all();
}

void PacketHandler::Reset() {
    for (auto& buffer : mBuffers) {
        buffer.Reset();
    }
}

RecvBuffer& PacketHandler::GetBuffer(bool swap) {
    if (swap) {
        Swap();
    }

    return mBuffers[mReadOnlyIdx];
}
