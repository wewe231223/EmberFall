#include "pch.h"
#include "NetworkCore.h"
#include "Listener.h"
#include "Session.h"

SessionIdType ClientCore::GetSessionId() const {
    return mSessionId;
}

ClientCore::RecvBuf& ClientCore::GetBuffer() {
    return mRecvBuf;
}

bool ClientCore::Start(const std::string& ip, const UINT16 port) {
    WSADATA data{ };
    if (0 != ::WSAStartup(MAKEWORD(2, 2), &data)) {
        return false;
    }

    mSocket = NetworkUtil::CreateClientSocket();
    if (INVALID_SOCKET == mSocket) {
        return false;
    }

    sockaddr_in serverAddr{ };
    if (false == NetworkUtil::InitSockAddr(serverAddr, port, ip.data())) {
        return false;
    }

    if (SOCKET_ERROR == ::connect(mSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr))) {
        auto error = WSAGetLastError();
        if (WSAEWOULDBLOCK != error) {
            return false;
        }
    }

    return true;
}

void ClientCore::End() {
    ::closesocket(mSocket);
    ::WSACleanup();
}

void ClientCore::InitSessionId(SessionIdType id) {
    mSessionId = id;
}

size_t ClientCore::Recv() {
    DWORD recvdBytes;
    DWORD recvFlag{ 0 };

    mRecvWSABuf.buf = reinterpret_cast<char*>(mRecvBuf.data() + mPrevRemain);
    mRecvWSABuf.len = static_cast<uint32_t>(RECV_BUF_SIZ - mPrevRemain);
    auto ret = ::WSARecv(mSocket, &mRecvWSABuf, 1, &recvdBytes, &recvFlag, nullptr, nullptr);
    if (SOCKET_ERROR == ret) {
        if (::WSAGetLastError() != WSAEWOULDBLOCK) {
            Crash("");
        }
        else {
            return 0;
        }
    }

    if (0 >= recvdBytes) {
        End();
    }

    auto remainData = recvdBytes + mPrevRemain;

    auto it = mRecvBuf.data();
    auto last = mRecvBuf.data() + remainData;
    while (it != last) {
        auto packetSize = *reinterpret_cast<PacketSizeT*>(it);
        if (0 == packetSize) {
            MessageBoxA(nullptr, "", "", MB_OK);
        }

        if (std::distance(it, last) < packetSize) {
            break;
        }

        if (it + packetSize < last) {
            it += packetSize;
        }
        else {
            break;
        }
    }

    mPrevRemain = std::distance(it, last);
    return  remainData - mPrevRemain;
}

void ClientCore::Send(OverlappedSend* const overlappedSend) {
    DWORD sent_bytes{ };
    if (SOCKET_ERROR != ::WSASend(mSocket, &overlappedSend->wsaBuf, 1, &sent_bytes, 0, nullptr, nullptr)) {
        FbsPacketFactory::ReleasePacketBuf(overlappedSend);
    }
}

void ClientCore::ProcessRemainData(size_t validSize) {
    auto target = mRecvBuf.data() + validSize;
    if (mPrevRemain > 0) {
        ::memcpy(mRecvBuf.data(), target, mPrevRemain);
    }
}
