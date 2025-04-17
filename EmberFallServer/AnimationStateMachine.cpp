#include "pch.h"
#include "AnimationStateMachine.h"
#include "GameObject.h"

AnimationStateMachine::AnimationStateMachine() { }

AnimationStateMachine::~AnimationStateMachine() { }

bool AnimationStateMachine::IsChangable() const {
    return mAnimationChangable;
}

Packets::AnimationState AnimationStateMachine::GetCurrState() const {
    return mCurrState.state;
}

float AnimationStateMachine::GetDuration(Packets::AnimationState state) const {
    return mAnimationInfo[static_cast<size_t>(state)].duration;
}

float AnimationStateMachine::GetRemainDuration() const {
    return mCurrState.duration - mAnimationCounter;
}

void AnimationStateMachine::SetOwner(std::shared_ptr<class GameObject> owner) {
    mOwner = owner;
}

void AnimationStateMachine::SetDefaultState(Packets::AnimationState state) {
    mDefaultState = mAnimationInfo[static_cast<size_t>(state)];
}

void AnimationStateMachine::ChangeState(Packets::AnimationState nextState, bool force) {
    if (false == force and mCurrState.state == nextState) {
        return;
    }

    auto ownerId = mOwner->GetId();

    mCurrState = mAnimationInfo[static_cast<size_t>(nextState)];

    mAnimationChangable = mCurrState.loop;
    mAnimationCounter = 0.0f;
    mAnimationChanged = true;
}

void AnimationStateMachine::Update(const float deltaTime) {
    if (true == mCurrState.loop) {
        return;
    }

    mAnimationCounter += deltaTime;
    if (mAnimationCounter > mCurrState.duration) {
        if (mAnimationChangable) {
            ChangeState(mDefaultState.state);
        }
        mAnimationChangable = true;
    }
}
