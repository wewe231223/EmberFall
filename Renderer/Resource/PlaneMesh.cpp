#include "pch.h"
#include "PlaneMesh.h"

PlaneMesh::PlaneMesh() {
}

PlaneMesh::~PlaneMesh() {
}

PlaneMesh::PlaneMesh(PlaneMesh&& other) noexcept {
	if (this != &other) {
		mVertexBuffers = std::move(other.mVertexBuffers);
		mIndexBuffer = std::move(other.mIndexBuffer);
		mVertexBufferViews = std::move(other.mVertexBufferViews);
		mIndexBufferView = other.mIndexBufferView;
		mPrimitiveTopology = other.mPrimitiveTopology;
		mIndexed = other.mIndexed;
	}
}

PlaneMesh& PlaneMesh::operator=(PlaneMesh&& other) noexcept {
	if (this != &other) {
		mVertexBuffers = std::move(other.mVertexBuffers);
		mIndexBuffer = std::move(other.mIndexBuffer);
		mVertexBufferViews = std::move(other.mVertexBufferViews);
		mIndexBufferView = other.mIndexBufferView;
		mPrimitiveTopology = other.mPrimitiveTopology;
		mIndexed = other.mIndexed;
	}
	return *this;
}

void PlaneMesh::Bind(ComPtr<ID3D12GraphicsCommandList> commandList) const {
	commandList->IASetPrimitiveTopology(mPrimitiveTopology);
	commandList->IASetVertexBuffers(0,static_cast<UINT>(mVertexBufferViews.size()), mVertexBufferViews.data());
	if (mIndexed) {
		commandList->IASetIndexBuffer(&mIndexBufferView);
	}
}

UINT PlaneMesh::GetUnitCount() const {
	return mUnitCount;
}
