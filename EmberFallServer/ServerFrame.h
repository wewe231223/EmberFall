#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ServerFrame.h
// 
// 2025 - 02 - 10 : Player들은 모든 게임 씬에서 그 정보를 알고 있어야 할 필요성이 있음.
//                  player가 서로 다른 게임씬에 있더라도 접속 정보는 어디에선가 통합해서 관리해야함.
//                  차라리 GameFrame에서 접속/퇴장한 플레이어정보를 가지고 게임씬에서 이 정보를 참조하도록하자.
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "SessionManager.h"
#include "../ServerLib/Listener.h"

class ServerFrame {
public:
    ServerFrame();
    ~ServerFrame();

public:
    std::shared_ptr<class InputManager> GetInputManager() const;

    bool StartServer();
    void Run();
    void Done();

    GameSession* GetSession(SessionIdType id);
    void CheckSessionsHeartBeat(std::vector<SessionIdType>& sessions);
    void CloseSession(SessionIdType id);

    void Send(SessionIdType to, OverlappedSend* packet);

    void PQCS(int32_t transfferedBytes, ULONG_PTR completionKey, OverlappedEx* overlapped);
    void AddTimerEvent(NetworkObjectIdType id, SysClock::duration delay, IoType eventType, ExtraInfo info = { });

private:
    bool IsGameRoomEvent(IoType type) const;
    void ProcessIoEvent(OverlappedEx* overlappedEx, ULONG_PTR completionKey);
    void IoThread();
    void TimerThread();
    void DbThread();

private:
    volatile bool mDone{ false };
    
    IOCPCore mIocpCore;
    Listener mListener{ SERVER_PORT };
    SessionManager mSessionManager{};

    std::shared_ptr<class InputManager> mInputManager{ };

    size_t mWorkerThreadNum{ };
    std::vector<std::thread> mWorkerThreads{ };

    std::thread mTimerThread{ };

    std::mutex mTimerMapLock{ };
    std::multimap<TimePoint<SysClock>, TimerEvent> mTimerEvents{ };
};