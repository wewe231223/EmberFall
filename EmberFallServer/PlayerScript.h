#pragma once

#include "Script.h"
#include "ViewList.h"

class GameObject;
class Input;

class PlayerScript : public Script {
public:
    PlayerScript(std::shared_ptr<GameObject> owner, std::shared_ptr<Input> input, ObjectTag tag, ScriptType scType);
    virtual ~PlayerScript();

public:
    std::shared_ptr<Input> GetInput() const;
    ViewList& GetViewList();

    void SetOwnerSession(std::shared_ptr<class GameSession> session);

    void UpdateViewList(const std::vector<NetworkObjectIdType>& inViewRangeNPC, const std::vector<NetworkObjectIdType>& inViewRangePlayer);

    virtual void Init() abstract;

    virtual void Update(const float deltaTime) abstract;
    virtual void LateUpdate(const float deltaTime) abstract;

    virtual void OnCollision(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) abstract;
    virtual void OnCollisionTerrain(const float height) abstract;

    virtual void DoInteraction(const std::shared_ptr<GameObject>& target) { }

    virtual void DispatchGameEvent(struct GameEvent* event) abstract;

protected:
    bool mSuperMode{ false };
    GameUnits::GameUnit<GameUnits::MeterPerSec> mSuperSpeed{ 10.0mps };
    std::shared_ptr<Input> mInput{ };

private:
    Lock::SRWLock mViewListLock{ };
    ViewList mViewList;
    
    NetworkObjectIdType mInteractionObj{ };

    std::weak_ptr<GameSession> mSession{ };
};