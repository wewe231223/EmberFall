#include "pch.h"
#include "Listener.h"
#include "Session.h"
#include "NetworkCore.h"

Listener::Listener(const UINT16 port)
    : mLocalPort{ port } {
}

Listener::~Listener() {
    Close();
}

HANDLE Listener::GetHandle() const {
    return reinterpret_cast<HANDLE>(mListenSocket);
}

bool Listener::Init() {
    mListenSocket = NetworkUtil::CreateSocket();
    if (INVALID_SOCKET == mListenSocket) {
        return false;
    }

    sockaddr_in sockAddr{ };
    NetworkUtil::InitSockAddr(sockAddr, mLocalPort);
    NetworkUtil::SetSocketOpt(mListenSocket, SOL_SOCKET, SO_REUSEADDR, 1);

    if (SOCKET_ERROR == ::bind(mListenSocket, reinterpret_cast<sockaddr*>(&sockAddr), sizeof(sockAddr))) {
        auto mess = NetworkUtil::WSAErrorMessage();
        MessageBoxA(nullptr, mess.c_str(), "SOCKET ERROR", MB_OK);
        return false;
    }

    if (SOCKET_ERROR == ::listen(mListenSocket, SOMAXCONN)) {
        auto mess = NetworkUtil::WSAErrorMessage();
        MessageBoxA(nullptr, mess.c_str(), "SOCKET ERROR", MB_OK);
        return false;
    }

    return true;
}

void Listener::Close() {
    ::closesocket(mListenSocket);
    mListenSocket = INVALID_SOCKET;
}

bool Listener::IsClosed() const {
    return mListenSocket == INVALID_SOCKET;
}

SOCKET Listener::GetListenSocket() const {
    return mListenSocket;
}

void Listener::RegisterAccept() {
    mOverlappedAccept.ResetOverlapped();
    mOverlappedAccept.connectedSocket = NetworkUtil::CreateSocket();
    INT addrSize{ sizeof(sockaddr_in) + 16 }; // addrsize는 내부 구현상 사용하는 주소체게 구조체 크기 + 16이 되어야함
    DWORD received{ };
    auto registSuccess = ::AcceptEx(
        mListenSocket,
        mOverlappedAccept.connectedSocket,
        mOverlappedAccept.buffer.data(),
        0,
        addrSize,
        addrSize,
        &received,
        &mOverlappedAccept
    );

    if (not registSuccess) {
        int errorCode = ::WSAGetLastError();
        if (WSA_IO_PENDING != errorCode) {
            auto mess = NetworkUtil::WSAErrorMessage();
            MessageBoxA(nullptr, mess.c_str(), "", MB_OK);
            RegisterAccept();
        }
    }
}