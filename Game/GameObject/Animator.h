#pragma once 
#include <variant>
#include "../MeshLoader/Base/AnimationData.h"
#include "../MeshLoader/Loader/AnimationLoader.h"
#include "../Config/Config.h"
#include "../Utility/Defines.h"

#ifdef max 
#undef max 
#endif 


namespace Legacy {
	class Animator {
	public:
		Animator() = default;
		Animator(const AnimationClip& clip) : mClip(clip) {}
		~Animator() = default;
	public:
		void UpdateBoneTransform(double time, std::vector<SimpleMath::Matrix>& boneTransforms);
	private:
		void ReadNodeHeirarchy(double AnimationTime, BoneNode* pNode, const SimpleMath::Matrix& ParentTransform, const AnimationClip& animation, std::vector<SimpleMath::Matrix>& boneTransforms);

		UINT FindPosition(double AnimationTime, const BoneAnimation& boneAnim);
		UINT FindRotation(double AnimationTime, const BoneAnimation& boneAnim);
		UINT FindScaling(double AnimationTime, const BoneAnimation& boneAnim);

		SimpleMath::Vector3 InterpolatePosition(double AnimationTime, const BoneAnimation& boneAnim);
		SimpleMath::Quaternion InterpolateRotation(double AnimationTime, const BoneAnimation& boneAnim);
		SimpleMath::Vector3 InterpolateScaling(double AnimationTime, const BoneAnimation& boneAnim);
	private:
		AnimationClip mClip{};
	};
}


class Animator {
public:
	Animator() = default;
	Animator(const AnimationClip* clip) : mClip(clip) {}
	~Animator() = default;
public:
	bool GetActivated() const; 
	void UpdateBoneTransform(double time, BoneTransformBuffer& boneTransforms);
private:
	void ReadNodeHeirarchy(double AnimationTime, BoneNode* pNode, const SimpleMath::Matrix& ParentTransform, std::array<SimpleMath::Matrix, Config::MAX_BONE_COUNT_PER_INSTANCE<>>& boneTransforms);

	UINT FindPosition(double AnimationTime, const BoneAnimation& boneAnim);
	UINT FindRotation(double AnimationTime, const BoneAnimation& boneAnim);
	UINT FindScaling(double AnimationTime, const BoneAnimation& boneAnim);

	SimpleMath::Vector3 InterpolatePosition(double AnimationTime, const BoneAnimation& boneAnim);
	SimpleMath::Quaternion InterpolateRotation(double AnimationTime, const BoneAnimation& boneAnim);
	SimpleMath::Vector3 InterpolateScaling(double AnimationTime, const BoneAnimation& boneAnim);
private:
	double mCounter{ 0.0 };

	const AnimationClip* mClip{ nullptr };
};

namespace AnimatorGraph {
    struct TransformComponents {
        SimpleMath::Vector3 translation;
        SimpleMath::Quaternion rotation;
        SimpleMath::Vector3 scale;
    };

    enum class ParameterType {
        Bool,
        Int,
        Trigger,
        Always,
    };

    struct AnimationTransition {
        size_t targetStateIndex;
        double blendDuration{ 0.09 };
        std::string parameterName;
        std::variant<bool, int> expectedValue;
		bool triggerOnEnd{ false };
    };

    struct AnimationState {
        std::string name;
        const AnimationClip* clip;
        double speed{ 1.0 };
        std::vector<AnimationTransition> transitions;
    };

    class AnimationParameter {
    public:
        AnimationParameter(const std::string& name, ParameterType type);
        AnimationParameter();

        std::string name;
        ParameterType type;

        bool boolValue;
        int intValue;
        float floatValue;
    };

    struct BoneMaskAnimationState {
        size_t maskedClipIndex;
        size_t nonMaskedClipIndex;
        double speed{ 1.0 };
        std::string name;
        std::vector<AnimationTransition> transitions;
    };

    inline bool DecomposeMatrix(SimpleMath::Matrix& mat, TransformComponents& outComponents) {
        return mat.Decompose(outComponents.scale, outComponents.rotation, outComponents.translation);
    }

    class Animator {
    public:
        Animator() = default;
        Animator(const std::vector<const AnimationClip*>& clips);
        ~Animator() = default;

    public:
        void UpdateBoneTransform(double deltaTime, BoneTransformBuffer& boneTransforms);
        void TransitionToClip(size_t clipIndex);
        void SetTransitionDuration(double duration = 0.09);
        double GetNormalizedTime();

    private:
        void ComputeBoneTransforms(const AnimationClip* clip, double animationTime, BoneTransformBuffer& boneTransforms);
        void ReadNodeHeirarchy(double AnimationTime, BoneNode* node, const DirectX::SimpleMath::Matrix& ParentTransform, std::array<DirectX::SimpleMath::Matrix, Config::MAX_BONE_COUNT_PER_INSTANCE<>>& boneTransforms, const AnimationClip* clip);
        void ReadNodeHeirarchyTransition(double currentAnimTime, double targetAnimTime, double blendFactor, BoneNode* node, const DirectX::SimpleMath::Matrix& ParentTransform, std::array<DirectX::SimpleMath::Matrix, Config::MAX_BONE_COUNT_PER_INSTANCE<>>& boneTransforms);

        void CaptureTransitionComponents(BoneNode* node, const DirectX::SimpleMath::Matrix& parentTransform, double animTime, double blendFactor);

        unsigned int FindPosition(double AnimationTime, const BoneAnimation& boneAnim);
        unsigned int FindRotation(double AnimationTime, const BoneAnimation& boneAnim);
        unsigned int FindScaling(double AnimationTime, const BoneAnimation& boneAnim);

        DirectX::SimpleMath::Vector3 InterpolatePosition(double AnimationTime, const BoneAnimation& boneAnim);
        DirectX::SimpleMath::Quaternion InterpolateRotation(double AnimationTime, const BoneAnimation& boneAnim);
        DirectX::SimpleMath::Vector3 InterpolateScaling(double AnimationTime, const BoneAnimation& boneAnim);

    private:
        std::vector<const AnimationClip*> mClips{};

        size_t mCurrentClipIndex{ 0 };
        size_t mTargetClipIndex{ 0 };

        bool mTransitioning{ false };

        double mTransitionTime{ 0.0 };
        double mTransitionDuration{ 0.09 };
        double mCurrentTime{ 0.0 };

        std::unordered_map<unsigned int, TransformComponents> mStoredTransitionComponents{};
        bool mHasStoredTransition{ false };
    };





    class AnimationGraphController {
    public:
        AnimationGraphController() = default;
        AnimationGraphController(const std::vector<AnimationState>& states);
        ~AnimationGraphController() = default;

		AnimationGraphController(const AnimationGraphController&) = default;
		AnimationGraphController& operator=(const AnimationGraphController&) = default;

		AnimationGraphController(AnimationGraphController&&) = default;
		AnimationGraphController& operator=(AnimationGraphController&&) = default;

		operator bool() const;
    public:
        void Update(double deltaTime, BoneTransformBuffer& boneTransforms);

        void AddParameter(const std::string& name, ParameterType type);

        void SetBool(const std::string& name, bool value);
        void SetInt(const std::string& name, int value);
        void SetTrigger(const std::string& name);
        void ResetTrigger(const std::string& name);

		void SetBool(BYTE index, bool value);
		void SetInt(BYTE index, int value);
		void SetTrigger(BYTE index);

        const AnimationParameter* GetParameter(const std::string& name) const;

		void Transition(size_t targretIndex, double transitionDuration = 0.09);
    private:
        void EvaluateTransitions();
    
    private:
        bool mActiveState{ false };

        std::vector<AnimationState> mStates{};

        std::unordered_map<std::string, AnimationParameter> mParameters{};
        std::vector<std::string> mParameterIndexed{}; 

        size_t mCurrentStateIndex{};
        Animator mAnimator{};
    };

    class BoneMaskAnimator {
    public:
        BoneMaskAnimator() = default;
        BoneMaskAnimator(const std::vector<const AnimationClip*>& clips, const std::vector<unsigned int>& boneMask);
        
		~BoneMaskAnimator() = default;
    public:
        void UpdateBoneTransforms(double deltaTime, BoneTransformBuffer& boneTransforms);
        void TransitionMaskedToClip(size_t clipIndex);
        void TransitionNonMaskedToClip(size_t clipIndex);
        void SetTransitionDuration(double duration = 0.09);

        double GetNormalizedTimeNonMasked();
        double GetNormalizedTimeMasked();

    private:
        void ComputeBoneTransforms(double animTimeMasked, double animTimeNonMasked, BoneTransformBuffer& boneTransforms);
        void ReadNodeHeirarchyTransition(double currentAnimTimeMasked, double currentAnimTimeNonMasked, double blendFactorMasked, double blendFactorNonMasked,
            BoneNode* node, const DirectX::SimpleMath::Matrix& parentTransform, BoneTransformBuffer& boneTransforms);
        void ReadNodeHeirarchy(double animTimeMasked, double animTimeNonMasked,
            BoneNode* node, const DirectX::SimpleMath::Matrix& parentTransform, BoneTransformBuffer& boneTransforms);
        unsigned int FindPosition(double AnimationTime, const BoneAnimation& boneAnim);
        unsigned int FindRotation(double AnimationTime, const BoneAnimation& boneAnim);
        unsigned int FindScaling(double AnimationTime, const BoneAnimation& boneAnim);

        DirectX::SimpleMath::Vector3 InterpolatePosition(double AnimationTime, const BoneAnimation& boneAnim);
        DirectX::SimpleMath::Quaternion InterpolateRotation(double AnimationTime, const BoneAnimation& boneAnim);
        DirectX::SimpleMath::Vector3 InterpolateScaling(double AnimationTime, const BoneAnimation& boneAnim);

        void CaptureTransitionComponents(BoneNode* node, const DirectX::SimpleMath::Matrix& parentTransform, double animTime, double blendFactor, bool masked);

    private:
        std::vector<const AnimationClip*> mClips {};
        const AnimationClip* mDefaultClip {};
        const AnimationClip* mClipMasked{};
        const AnimationClip* mClipNonMasked{};

        bool mTransitioningMasked{ false };
        const AnimationClip* mTargetClipMasked{ nullptr };
        double mTransitionTimeMaskedTransition{ 0.0 };

        bool mTransitioningNonMasked{ false };
        const AnimationClip* mTargetClipNonMasked{};
        double mTransitionTimeNonMaskedTransition{ 0.0 };

        double mTransitionDuration{ 0.09 };

        std::unordered_set<unsigned int> mBoneMask{};
        std::shared_ptr<BoneNode> mRootNode{};
        double mCurrentTimeMasked{ 0.0 };
        double mCurrentTimeNonMasked{ 0.0 };

        std::unordered_map<unsigned int, TransformComponents> mStoredTransitionComponentsMasked{};
        std::unordered_map<unsigned int, TransformComponents> mStoredTransitionComponentsNonMasked{};

        bool mHasStoredTransitionMasked{ false };
        bool mHasStoredTransitionNonMasked{ false };
    };


    class BoneMaskAnimationGraphController {
    public:
        BoneMaskAnimationGraphController() = default;
        BoneMaskAnimationGraphController(const std::vector<const AnimationClip*>& clips, const std::vector<UINT>& boneMask, const std::vector<BoneMaskAnimationState>& states);
        ~BoneMaskAnimationGraphController() = default;

		BoneMaskAnimationGraphController(const BoneMaskAnimationGraphController&) = default;
		BoneMaskAnimationGraphController& operator=(const BoneMaskAnimationGraphController&) = default;

		BoneMaskAnimationGraphController(BoneMaskAnimationGraphController&&) = default;
		BoneMaskAnimationGraphController& operator=(BoneMaskAnimationGraphController&&) = default;
    public:
        operator bool() const; 

        void Update(double deltaTime, BoneTransformBuffer& boneTransforms);

        void AddParameter(const std::string& name, ParameterType type);

        void SetBool(const std::string& name, bool value);
        void SetInt(const std::string& name, int value);
        void SetTrigger(const std::string& name);

        void SetBool(BYTE index, bool value); 
		void SetInt(BYTE index, int value);
		void SetTrigger(BYTE index);
        

        void ResetTrigger(const std::string& name);
        const AnimationParameter* GetParameter(const std::string& name) const;

        void Transition(size_t targretIndex, double transitionDuration = 0.09);
    private:
        void EvaluateTransitions();
    private:
        bool mActiveState{ false };

        std::vector<BoneMaskAnimationState> mStates{};

        std::unordered_map<std::string, AnimationParameter> mParameters{};
        std::vector<std::string> mParameterIndexed{};

        size_t mCurrentStateIndex{};
        BoneMaskAnimator mAnimator{};
    };

};
