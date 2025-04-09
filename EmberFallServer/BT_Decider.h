#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// BT_Decider.h
//  
// 2025 - 04 - 03 : 일정 시간마다 설정된 Behavior Tree마다 평가값을 계산 해서 그 상황에서 가장 알맞는 행동을 하도록
//                  지시하는 기능을 완성.
//                  일단 1초마다 평가, 평가시 이전 노드와 다른 노드가 세팅되면 그 노드를 처음부터 실행함.
//                  아니라면 해당노드를 계속 업데이트
// 
//                  Decider를 만듦으로써 각 노드를 무조건 끝까지 실행하는게 아닌 상황에 따라 알맞는 노드를 
//                  그때그때 선택할 수 있도록 만듦.
//                  
//                  BT_Decider에 세팅되는 BT들은 태스크 단위로 만들어 져야함 (ex: Patrol, Attack, Chase...)
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "BehaviorTreeBase.h"

namespace BT {
    template <typename... Args> requires IsDerivedFrom<BehaviorTree, Args...>
    class BT_Decider {
    public:
        using TreeNode = std::unique_ptr<BehaviorTree>;
        using TreeList = std::vector<TreeNode>;
        using TreeListIter = TreeList::iterator;

    public:
        BT_Decider() {}

        ~BT_Decider() {
            mRoots.clear();
        }

    public:
        void Build(std::shared_ptr<Script> ownerScript) {
            mRoots.reserve(sizeof...(Args));

            mOwner = ownerScript;
            (BuildTree<Args>(), ...);

            Start();
        }

        void Decide() {
            auto maxNode = std::ranges::max_element(mRoots, [this](const auto& node1, const auto& node2) {
                float val1 = node1->CalculateDecideValue(mOwner);
                float val2 = node2->CalculateDecideValue(mOwner);
                return val1 < val2;
                }
            );

            if (maxNode == mCurrNode) {
                return;
            }

            ChangeTargetRoot(maxNode);
        }

        void Update(const float deltaTime) {
            if (Packets::AnimationState_DEAD == mOwner->GetOwner()->mAnimationStateMachine.GetCurrState()) {
                return;
            }

            mDecideTimeCounter += deltaTime;
            if (mDecideTimeCounter > mDecideTimeInterval) {
                Decide();
                mDecideTimeCounter = 0.0f;
            }
            (*mCurrNode)->Update(deltaTime);
        }

        void DispatchGameEvent(GameEvent* event) {
            (*mCurrNode)->DispatchGameEvent(event);
        }

        void Interrupt() {
            Decide();
        }

    public:

        void Start() {
            auto maxNode = std::ranges::max_element(mRoots, [this](const auto& node1, const auto& node2) {
                float val1 = node1->CalculateDecideValue(mOwner);
                float val2 = node2->CalculateDecideValue(mOwner);
                return val1 < val2;
                }
            );

            mCurrNode = maxNode;
            (*mCurrNode)->Start();
        }

        template <typename T>
        void BuildTree() {
            TreeNode tree = std::make_unique<T>();
            tree->Build(mOwner);
            mRoots.emplace_back(std::move(tree));
        }

        void ChangeTargetRoot(TreeListIter node) {
            mCurrNode = node;
            (*mCurrNode)->Start();
        }

    private:
        TreeList mRoots{ };
        TreeListIter mCurrNode{ };
        float mDecideTimeCounter{ 0.0f };
        float mDecideTimeInterval{ 2.0f };
        std::shared_ptr<Script> mOwner{ nullptr };
    };
}