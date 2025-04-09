#pragma once

struct AnimationInfo {
    float duration;
    bool loop;
    AnimationState state;
};

inline std::array<AnimationInfo, AnimationState::DEAD + 1> DEFAULT_ANIM_INFO{
    AnimationInfo{ 3.0f, true, AnimationState::IDLE },
    AnimationInfo{ 3.0f, true, AnimationState::MOVE_FORWARD },
    AnimationInfo{ 3.0f, true, AnimationState::MOVE_BACKWARD },
    AnimationInfo{ 3.0f, true, AnimationState::MOVE_LEFT },
    AnimationInfo{ 3.0f, true, AnimationState::MOVE_RIGHT },
    AnimationInfo{ 3.0f, false, AnimationState::JUMP },
    AnimationInfo{ 2.3f, false, AnimationState::ATTACKED },
    AnimationInfo{ 2.3f, false, AnimationState::ATTACK },
    AnimationInfo{ 3.0f, true, AnimationState::INTERACTION },
    AnimationInfo{ 3.0f, false, AnimationState::DEAD }
};

class AnimationStateMachine {
public:
    AnimationStateMachine();
    ~AnimationStateMachine();

public:
    bool IsChangable() const;
    AnimationState GetCurrState() const;
    float GetDuration(AnimationState state) const;
    float GetRemainDuration() const;

    void SetOwner(std::shared_ptr<class GameObject> owner);
    void SetDefaultState(AnimationState state);
    void ChangeState(AnimationState nextState, bool force=false);

    void Update(const float deltaTime);

private:
    bool mAnimationChangable{ true };
    float mAnimationCounter{ };
    AnimationInfo mDefaultState{ AnimationState::IDLE };
    AnimationInfo mCurrState{ AnimationState::IDLE };

    std::shared_ptr<class GameObject> mOwner{ nullptr };
    std::array<AnimationInfo, AnimationState::DEAD + 1> mAnimationInfo{ DEFAULT_ANIM_INFO };
};