#include "pch.h"
#include "PacketHandler.h"

RecvBuffer::RecvBuffer() { }

RecvBuffer::~RecvBuffer() { }

RecvBuffer::RecvBuffer(const RecvBuffer& other) {
    std::copy(other.mBuffer.begin(), other.mBuffer.end(), mBuffer.begin());
    mWritePos = other.mWritePos.load();
    mReadPos = other.mReadPos;
}

RecvBuffer::RecvBuffer(RecvBuffer&& other) noexcept {
    std::copy(other.mBuffer.begin(), other.mBuffer.end(), mBuffer.begin());
    mWritePos = other.mWritePos.load();
    mReadPos = other.mReadPos;
}

RecvBuffer& RecvBuffer::operator=(const RecvBuffer& other) {
    std::copy(other.mBuffer.begin(), other.mBuffer.end(), mBuffer.begin());
    mWritePos = other.mWritePos.load();
    mReadPos = other.mReadPos;

    return *this;
}

RecvBuffer& RecvBuffer::operator=(RecvBuffer&& other) noexcept {
    std::copy(other.mBuffer.begin(), other.mBuffer.end(), mBuffer.begin());
    mWritePos = other.mWritePos.load();
    mReadPos = other.mReadPos;

    return *this;
}

char* RecvBuffer::Data() {
    return mBuffer.data();
}

void RecvBuffer::AdvanceReadPos(size_t size) {
    mReadPos += size;
}

void RecvBuffer::Read(void* buffer, size_t size) {
    auto srcIter = mBuffer.begin() + mReadPos;
    auto packetSize = NetworkUtil::GetPacketSizeFromIter(srcIter);
    if (size < packetSize) {
        return;
    }

    ::memcpy(buffer, NetworkUtil::AddressOf(srcIter), packetSize);
    mReadPos += size;
}

void RecvBuffer::Write(void* data, size_t size) {
    auto dest = mWritePos.fetch_add(size);

	if (dest + size > mBuffer.size()) {
		MessageBox(nullptr, L"RecvBuffer Overflow", L"Error", MB_OK | MB_ICONERROR);
        return;
	}

    ::memcpy(mBuffer.data() + dest, data, size);
}

void RecvBuffer::Reset() {
    mWritePos.store(0);
    mReadPos = 0;
}

size_t RecvBuffer::Size() const {
    return mWritePos.load();
}

size_t RecvBuffer::GetReadPos() const {
    return mReadPos;    
}

bool RecvBuffer::IsReadEnd() const {
    return Size() <= mReadPos;
}

PacketHandler::PacketHandler() { }

PacketHandler::~PacketHandler() { }

void PacketHandler::Write(void* data, size_t size) {
    std::shared_lock bufferGuard{ mBufferLock };

    mBuffers[mWriteOnlyIdx].Write(data, size);
}

void PacketHandler::Swap() {
    std::unique_lock bufferGuard{ mBufferLock };

    std::swap(mWriteOnlyIdx, mReadOnlyIdx);
    mBuffers[mWriteOnlyIdx].Reset();
}

void PacketHandler::Reset() {
    for (auto& buffer : mBuffers) {
        buffer.Reset();
    }
}

RecvBuffer& PacketHandler::GetBuffer(bool swap) {
    if (swap) {
        Swap();
#if DEF_DEBUG_OR_DEV
        gPacketHandlerDebugSize += mBuffers[mReadOnlyIdx].Size();
#endif
    }

    return mBuffers[mReadOnlyIdx];
}
