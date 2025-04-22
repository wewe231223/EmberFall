#pragma once

struct AnimationInfo {
    float duration;
    bool loop;
    Packets::AnimationState state;
};

inline std::array<AnimationInfo, Packets::AnimationState_MAX + 1> DEFAULT_ANIM_INFO{
    AnimationInfo{ 3.0f, true, Packets::AnimationState_IDLE },
    AnimationInfo{ 3.0f, true, Packets::AnimationState_MOVE_FORWARD },
    AnimationInfo{ 3.0f, true, Packets::AnimationState_MOVE_BACKWARD },
    AnimationInfo{ 3.0f, true, Packets::AnimationState_MOVE_LEFT },
    AnimationInfo{ 3.0f, true, Packets::AnimationState_MOVE_RIGHT },
    AnimationInfo{ 3.0f, false, Packets::AnimationState_JUMP },
    AnimationInfo{ 2.3f, false, Packets::AnimationState_ATTACKED },
    AnimationInfo{ 1.7f, false, Packets::AnimationState_ATTACK },
    AnimationInfo{ 3.0f, true, Packets::AnimationState_INTERACTION },
    AnimationInfo{ 3.0f, false, Packets::AnimationState_DEAD }
};

class AnimationStateMachine {
public:
    AnimationStateMachine();
    ~AnimationStateMachine();

public:
    bool IsChangable() const;
    Packets::AnimationState GetCurrState() const;
    float GetDuration(Packets::AnimationState state) const;
    float GetRemainDuration() const;

    void SetOwner(std::shared_ptr<class GameObject> owner);
    void SetDefaultState(Packets::AnimationState state);
    void ChangeState(Packets::AnimationState nextState, bool force=false);

    void Update(const float deltaTime);

private:
    bool mAnimationChangable{ true };
    float mAnimationCounter{ };
    AnimationInfo mDefaultState{ Packets::AnimationState_IDLE };
    AnimationInfo mCurrState{ Packets::AnimationState_IDLE };

    std::shared_ptr<class GameObject> mOwner{ nullptr };
    std::array<AnimationInfo, Packets::AnimationState_MAX + 1> mAnimationInfo{ DEFAULT_ANIM_INFO };
};