#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// PlayerScript.h
// 
// 2025 - 02 - 10 : PlayerScript 
//                  Input을 GameObject와 분리하고 PlayerScript에서 참조하도록 수정함.
//                  Input의 생성/삭제는 GameFrame에서 player의 생성과 동시에 생성, 삭제함
//                  
//                  Player는 현재 자신이 속한 GameScene의 정보를 알 수 있도록 참조하도록 함.
// 
//        02 - 11 : ViewList 추가
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "Script.h"
#include "ViewList.h"

class GameObject;
class Input;
class IServerGameScene;

class PlayerScript : public Script {
public:
    PlayerScript(std::shared_ptr<GameObject> owner, std::shared_ptr<Input> input, std::shared_ptr<SessionManager> sessionManager);
    virtual ~PlayerScript();

public:
    void ResetGameScene(std::shared_ptr<IServerGameScene> gameScene);
    std::shared_ptr<IServerGameScene> GetCurrentScene() const;

    virtual void Update(const float deltaTime);

    virtual void OnHandleCollisionEnter(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) override;
    virtual void OnHandleCollisionStay(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) override;
    virtual void OnHandleCollisionExit(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) override;

private:
    std::shared_ptr<Input> mInput{ };
    std::shared_ptr<IServerGameScene> mGameScene{ };
    ViewList mViewList;
};

