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

class ServerFrame {
public:
    ServerFrame();
    ~ServerFrame();

public:
    std::shared_ptr<class InputManager> GetInputManager() const;

    void Run();
    void Done();

    void PQCS(int32_t transfferedBytes, ULONG_PTR completionKey, OverlappedEx* overlapped);
    void AddTimerEvent(uint16_t roomIdx, NetworkObjectIdType id, SysClock::time_point executeTime, TimerEventType eventType, ExtraInfo info = { });

private:
    bool IsGameRoomEvent(TimerEventType type) const;
    void TimerThread();

private:
    volatile bool mDone{ false };
    
    Lock::SRWLock mPlayersLock{ };
    std::unordered_map<SessionIdType, std::shared_ptr<class GameObject>> mPlayers{ };

    std::shared_ptr<class InputManager> mInputManager{ };

    std::thread mTimerThread{ };
    Concurrency::concurrent_priority_queue<TimerEvent> mTimerEvents{ };
};