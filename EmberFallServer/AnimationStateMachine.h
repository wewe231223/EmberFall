#pragma once

#include "Resources.h"

class AnimationStateMachine {
public:
    AnimationStateMachine();
    ~AnimationStateMachine();

public:
    void Init(const std::string& entityKey);

    bool IsChangable() const;
    Packets::AnimationState GetCurrState() const;
    float GetDuration(Packets::AnimationState state) const;
    float GetRemainDuration() const;

    void SetOwner(std::shared_ptr<class GameObject> owner);
    void SetDefaultState(Packets::AnimationState state);
    void ChangeState(Packets::AnimationState nextState, bool force=false);

    void Update(const float deltaTime);

public:
    bool mAnimationChanged{ false };

private:
    bool mIsLoopAnimation{ true };
    float mAnimationCounter{ };

    Packets::AnimationState mDefaultState{ };
    AnimationState mDefaultAnimInfo{ };

    Packets::AnimationState mCurrState{ };
    AnimationState mCurrAnimInfo{ };

    std::shared_ptr<class GameObject> mOwner{ nullptr };
    std::shared_ptr<AnimationInfo> mAnimInfo{ };
};