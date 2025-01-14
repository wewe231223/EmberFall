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
    if (true == mSwap.load()) {
        std::cout << "Wait Swap End..." << std::endl;
        mSwap.wait(false);
    }

    mWriteCount.fetch_add(1);
    mBuffers[mWriteOnlyIdx].Write(data, size);
    mWriteCount.fetch_sub(1);
    if (0 == mWriteCount) {
        mWriteCount.notify_all();
    }
}

void PacketHandler::Swap() {
    mSwap.exchange(true);
    if (0 < mWriteCount.load()) {
        std::cout << "Wait Writing End..." << std::endl;
        mWriteCount.wait(0);
    }

    std::cout << "Start Swap!..." << std::endl;
    std::swap(mWriteOnlyIdx, mReadOnlyIdx);
    mBuffers[mWriteOnlyIdx].Reset();

    mSwap.exchange(false);
    std::cout << "Swap End!..." << std::endl;
    mSwap.notify_all();
}

void PacketHandler::Reset() {
    for (auto& buffer : mBuffers) {
        buffer.Reset();
    }
}