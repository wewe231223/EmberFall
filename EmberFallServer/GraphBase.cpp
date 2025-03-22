#include "pch.h"
#include "GraphBase.h"

using namespace Graphs;

void Graphs::Graph::Reserve(size_t nodeSize) {
    mNodes.reserve(nodeSize);
    mAdjacencyList.reserve(nodeSize);
}

Graph::EdgeList::iterator Graphs::Graph::EdgeListBegin(IndexType idx) {
    return mAdjacencyList[idx].begin();
}

Graph::EdgeList::iterator Graphs::Graph::EdgeListEnd(IndexType idx) {
    return mAdjacencyList[idx].end();
}

size_t Graph::GetNodeSize() const {
    return mNodes.size();
}

size_t Graph::GetEdgeSize() const {
    return std::accumulate(mAdjacencyList.begin(), mAdjacencyList.end(), 0ULL, 
        [=](size_t val, const EdgeList& list)
        { 
            return val + list.size();
        }
    );
}

Graph::Node Graph::GetNode(IndexType node) const {
    return mNodes[node];
}

Graph::Node& Graph::GetNode(IndexType node) {
    return mNodes[node];
}

Graph::Edge Graph::GetEdge(IndexType from, IndexType to) const {
    for (auto& edge : mAdjacencyList[from]) {
        if (to == edge.to) {
            return edge;
        }
    }

    return Edge{ };
}

Graph::Edge& Graph::GetEdge(IndexType from, IndexType to) {
    for (auto& edge : mAdjacencyList[from]) {
        if (to == edge.to) {
            return edge;
        }
    }

}

void Graph::AddNode(Node node) {
    if (node.index != mNodes.size()) {
        Crash("Invalid Garph Node");
    }

    mNodes.push_back(node);
    mAdjacencyList.emplace_back();
}

void Graph::RemoveNode(IndexType node) {
}

void Graph::AddEdge(Edge edge) {
    mAdjacencyList[edge.from].push_back(edge);
    mAdjacencyList[edge.to].emplace_back(edge.to, edge.from);
}

void Graphs::Graph::AddEdge(IndexType from, IndexType to) {
    mAdjacencyList[from].emplace_back(from, to);
    mAdjacencyList[to].emplace_back(to, from);
}

void Graph::RemoveEdge(IndexType from, IndexType to) {

}