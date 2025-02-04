#pragma once

inline constexpr UINT16 SERVER_PORT = 7777;

class ServerFrame {
public:
    ServerFrame();
    ~ServerFrame();

public:
    void InitGameScenes();
    void GameLoop();

private:
    bool mDone{ };
    std::shared_ptr<class ServerCore> mServerCore{ nullptr };
    std::shared_ptr<class GameTimer> mTimer{ };

    std::vector<std::shared_ptr<class IServerGameScene>> mGameScenes{ };
    std::shared_ptr<class IServerGameScene> mCurrentScene{ };
};