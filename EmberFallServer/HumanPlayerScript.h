#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// HumanPlayerScript.h
// 
// 2025 - 02 - 10 : HumanPlayerScript 
//                  Input을 GameObject와 분리하고 PlayerScript에서 참조하도록 수정함.
//                  Input의 생성/삭제는 GameFrame에서 player의 생성과 동시에 생성, 삭제함
//                  
//                  Player는 현재 자신이 속한 GameScene의 정보를 알 수 있도록 참조하도록 함.
// 
//        02 - 11 : ViewList 추가
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "PlayerScript.h"
#include "Inventory.h"

class HumanPlayerScript : public PlayerScript {
public:
    HumanPlayerScript(std::shared_ptr<GameObject> owner, std::shared_ptr<Input> input);
    virtual ~HumanPlayerScript();

public:
    virtual void Init() override;

    virtual void Update(const float deltaTime) override;
    virtual void LateUpdate(const float deltaTime) override;

    virtual void OnCollision(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) override;
    virtual void OnCollisionTerrain(const float height) override;

    virtual void DoInteraction(const std::shared_ptr<GameObject>& target) override;

    virtual void DispatchGameEvent(struct GameEvent* event) override;

private:
    std::shared_ptr<GameObject> GetNearestObject();

    void CheckAndMove(const float deltaTime);
    void CheckAndJump(const float deltaTime);

    void CancelInteraction();
    void DestroyingGem(const float deltaTime, const std::shared_ptr<GameObject>& gem);
    void SuccessInteraction();
    void AcquireItem(const float deltaTime, const std::shared_ptr<GameObject>& item);
    void UseItem();

private:
    bool mInteraction{ false };
    Inventory mInventory{ };

    NetworkObjectIdType mInteractionObj{ };
    std::shared_ptr<GameObject> mInteractionTrigger{ };
};

