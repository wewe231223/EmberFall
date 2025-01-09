#include "pch.h"
#include "Listener.h"
#include "Session.h"

Listener::Listener(const std::string& localIp, const unsigned short port)
    : mLocalIp{ localIp }, mLocalPort{ port } {
    mListenSocket = NetworkUtil::CreateSocket();
    CrashExp(INVALID_SOCKET != mListenSocket);

    sockaddr_in sockAddr{ };
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = ::htons(port);
    ::inet_pton(AF_INET, localIp.c_str(), &sockAddr.sin_addr.s_addr);

    if (SOCKET_ERROR == ::bind(mListenSocket, reinterpret_cast<sockaddr*>(&sockAddr), sizeof(sockAddr))) {
        Crash("");
    }

    if (SOCKET_ERROR == ::listen(mListenSocket, SOMAXCONN)) {
        Crash("");
    }
}

Listener::~Listener() {
    Close();
}

HANDLE Listener::GetHandle() const {
    return reinterpret_cast<HANDLE>(mListenSocket);
}

void Listener::Close() {
    ::closesocket(mListenSocket);
}

void Listener::ProcessOverlapped(OverlappedEx* overlapped, INT32 numOfBytes) {
    if (IOType::ACCEPT != overlapped->type) {
        /* Print Error Log */
        return;
    }

    // TODO
    RegisterAccept();
}

void Listener::RegisterAccept() {
    std::shared_ptr<Session> session = std::make_shared<Session>();

    ::memset(&mAcceptOverlappedEx, 0, sizeof(OVERLAPPED));
    SOCKET clientSocket = reinterpret_cast<SOCKET>(session->GetHandle());

    INT addrSize{ sizeof(sockaddr_in) + 16 }; // addrsize는 내부 구현상 사용하는 주소체게 구조체 크기 + 16이 되어야함
    DWORD received{ };
    auto registSuccess = ::AcceptEx(
        mListenSocket,
        clientSocket,
        &mAcceptOverlappedEx.wsaBuf,
        0,
        addrSize,
        addrSize,
        &received,
        &mAcceptOverlappedEx
    );

    if (not registSuccess) {
        int errorCode = ::WSAGetLastError();
        if (WSA_IO_PENDING != errorCode) {
            RegisterAccept();
        }
    }
}

void Listener::ProcessAccept() {

}
