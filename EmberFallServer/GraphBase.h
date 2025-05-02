#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GraphEdge.h
// 
// 2025 - 03 - 03 : 그래프 구현에 필요한 간선, 노드를 구현, 기본적인 그래프 구현
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Graphs {
    template <typename IndexT> requires std::is_arithmetic_v<IndexT>
    struct GraphNode {
        IndexT index;

        GraphNode() { }
        GraphNode(IndexT index) : index{ index } { }
    };

    template <typename IndexT> requires std::is_arithmetic_v<IndexT>
    struct GraphEdge {
        IndexT from{ };
        IndexT to{ };
        
        GraphEdge() { }
        GraphEdge(IndexT from, IndexT to) : from{ from }, to{ to } { }
    };

    template <typename IndexT, typename WeightT = double> requires std::is_arithmetic_v<IndexT>
    struct WeightedGraphEdge {
        IndexT from;
        IndexT to;
        WeightT weight;
    };

    class Graph {
    public:
        using IndexType = INT32;

        using Node = GraphNode<IndexType>;
        using Edge = GraphEdge<IndexType>;

        using NodeVector = std::vector<Node>;
        using EdgeList = std::list<Edge>;
        using AdjacencyList = std::vector<EdgeList>;

    public:
        static constexpr IndexType INVALID_NODE = -1;

    public:
        Graph() = default;
        ~Graph() = default;

    public:
        void Reserve(size_t nodeSize);

        EdgeList::iterator EdgeListBegin(IndexType idx);
        EdgeList::iterator EdgeListEnd(IndexType idx);

        size_t GetNodeSize() const;
        size_t GetEdgeSize() const;

        Node GetNode(IndexType node) const;
        Node& GetNode(IndexType node);

        Edge GetEdge(IndexType from, IndexType to) const;
        Edge& GetEdge(IndexType from, IndexType to);

        void AddNode(Node node);
        void RemoveNode(IndexType node);

        void AddEdge(Edge edge);
        void AddEdge(IndexType from, IndexType to);
        void RemoveEdge(IndexType from, IndexType to);
            
    private:
        NodeVector mNodes;
        AdjacencyList mAdjacencyList; // 인접 리스트
    };
}