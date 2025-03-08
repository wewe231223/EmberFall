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


inline constexpr UINT16 SERVER_PORT = 7777;

class ServerFrame {
public:
    ServerFrame();
    ~ServerFrame();

public:
    void InitGameScenes();
    void GameLoop();

private:
    void OnPlayerConnect(SessionIdType id);
    void OnPlayerDisconnect(SessionIdType id);

private:
    bool mDone{ };
    std::shared_ptr<class ServerCore> mServerCore{ nullptr };
    std::shared_ptr<class GameTimer> mTimer{ };

    std::vector<std::shared_ptr<class IServerGameScene>> mGameScenes{ };
    std::shared_ptr<class IServerGameScene> mCurrentScene{ };

    Lock::SRWLock mPlayersLock{ };
    std::shared_ptr<class InputManager> mInputManager{ };
    std::unordered_map<SessionIdType, std::shared_ptr<class GameObject>> mPlayers{ };

    Concurrency::concurrent_queue<PlayerEvent> mPlayerEventQueue{ };
};