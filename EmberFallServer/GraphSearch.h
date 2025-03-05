#pragma once

#include "GraphEdge.h"

namespace Graphs {
    namespace Search {
        enum class SearchState : BYTE {
            COMPLETE,
            INCOMPLETE,
            TARGET_NOT_FOUND,
        };

        class GraphSearch abstract {
        public:
            using Node = Graph::Node;
            using Edge = Graph::Edge;
            using IndexType = Graph::IndexType;

            struct NodeCost {
                IndexType nodeIdx;
                float cost;

                NodeCost(IndexType idx, float cost) : nodeIdx{ idx }, cost{ cost } { }

                bool operator<(const NodeCost& right) {
                    return cost < right.cost;
                }
            };

        public:
            GraphSearch(Graph& graph) : mGraph{ graph } { }
            virtual ~GraphSearch() { }

        public:
            virtual void Search(float timeSlice) abstract;
            virtual void Search(GameUnits::GameUnit<GameUnits::Second> timeSlice) abstract;
            virtual void Search(std::chrono::milliseconds timeSlice) abstract;

            virtual std::list<int> GetShortestPath() abstract;

        protected:
            Graphs::Graph& mGraph;
        };

        template <typename Heuristic> 
        class GraphSearchAStar : public GraphSearch {
        public:
            GraphSearchAStar(Graph& graph, IndexType source, IndexType dest);
            ~GraphSearchAStar() { }

        public:
            virtual void Search(float timeSlice) override;
            virtual void Search(GameUnits::GameUnit<GameUnits::Second> timeSlice) override;
            virtual void Search(std::chrono::milliseconds timeSlice) override;

            virtual std::list<int> GetShortestPath() override;

            std::vector<int> GetShortestPathTree();
            float GetTotalCostToTarget();

        private:
            IndexType mSourceNode{ };
            IndexType mDestNode{ };

            std::priority_queue<NodeCost> mPriorityQueue;

            std::vector<float> mCosts;
            std::vector<float> mDepthCosts;
            std::vector<Edge*> mShortestPathTree;
            std::vector<Edge*> mSearchFrontier; // 탐색중인 노드의 부모노드를 기억
        };
    }
}

// 구현부분 (너무 길어서 분리함
namespace Graphs {
    namespace Search {
        template<typename Heuristic>
        inline GraphSearchAStar<Heuristic>::GraphSearchAStar(Graph& graph, IndexType source, IndexType dest)
            : GraphSearch{ graph }, mSourceNode{source}, mDestNode{dest}, mPriorityQueue{ } {
            // queue 초기화 및 코스트 초기화
            mDepthCosts.resize(mGraph.GetNodeSize(), 0.0f);
            mCosts.resize(mGraph.GetNodeSize(), 0.0f);
            mPriorityQueue.emplace(source, 0.0f);
        }

        // Search Cycle
        template<typename Heuristic>
        inline void GraphSearchAStar<Heuristic>::Search(float timeSlice) {
            if (mPriorityQueue.empty()) {
                return SearchState::TARGET_NOT_FOUND;
            }

            auto nextClosestNode = mPriorityQueue.pop();

            if (nextClosestNode == mDestNode) {
                return SearchState::COMPLETE;
            }

            mShortestPathTree[nextClosestNode] = mSearchFrontier[nextClosestNode];

            auto edgeIter = mGraph.EdgeListBegin(nextClosestNode);
            for (; edgeIter != mGraph.EdgeListEnd(); ++edgeIter) {
                auto heuristicCost = Heuristic::Calculate(mGraph, mDestnode, edgeIter->to);
                auto depthCost = mDepthCosts[edgeIter->from] + 1.0f; // 가중치 X

                auto frontierNode = mSearchFrontier[edgeIter->to];
                if (nullptr == frontierNode) { // 처음 방문하는 노드
                    mCosts[edgeIter->to] = depthCost + heuristicCost;
                    mDepthCosts[edgeIter->to] = depthCost;

                    mPriorityQueue.emplace(edgeIter->to, mCosts[edgeIter->to]);

                    mSearchFrontier[edgeIter->to] = &(*edgeIter);
                }
                else if (depthCost < mDepthCosts[edgeIter->to] and nullptr == mShortestPathTree[edgeIter->to]) { 
                    // 방문은 했으나 더 좋은 경로 발견 시
                    mCosts[edgeIter->to] = depthCost + heuristicCost;
                    mDepthCosts[edgeIter->to] = depthCost;

                    mSearchFrontier[edgeIter->to] = &(*edgeIter);
                }
            }
        }
    }
}

