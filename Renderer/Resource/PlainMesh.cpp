#include "pch.h"
#include "PlainMesh.h"

PlainMesh::PlainMesh() {
}

PlainMesh::~PlainMesh() {
}

PlainMesh::PlainMesh(const PlainMesh& other) {
	mVertexBuffers = other.mVertexBuffers;
	mIndexBuffer = other.mIndexBuffer;

	mVertexBufferViews = other.mVertexBufferViews;
	mIndexBufferView = other.mIndexBufferView;

	mPrimitiveTopology = other.mPrimitiveTopology;
	mIndexed = other.mIndexed;
	mUnitCount = other.mUnitCount;
}

PlainMesh& PlainMesh::operator=(const PlainMesh& other) {
	if (this != &other) {
		mVertexBuffers = other.mVertexBuffers;
		mIndexBuffer = other.mIndexBuffer;

		mVertexBufferViews = other.mVertexBufferViews;
		mIndexBufferView = other.mIndexBufferView;

		mPrimitiveTopology = other.mPrimitiveTopology;
		mIndexed = other.mIndexed;
		mUnitCount = other.mUnitCount;
	}
	return *this;
}

PlainMesh::PlainMesh(PlainMesh&& other) noexcept {
	if (this != &other) {
		mVertexBuffers = std::move(other.mVertexBuffers);
		mIndexBuffer = std::move(other.mIndexBuffer);
		mVertexBufferViews = std::move(other.mVertexBufferViews);
		mIndexBufferView = other.mIndexBufferView;
		mPrimitiveTopology = other.mPrimitiveTopology;
		mIndexed = other.mIndexed;
	}
}

PlainMesh& PlainMesh::operator=(PlainMesh&& other) noexcept {
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

void PlainMesh::Bind(ComPtr<ID3D12GraphicsCommandList> commandList) const {
	commandList->IASetPrimitiveTopology(mPrimitiveTopology);
	commandList->IASetVertexBuffers(0,static_cast<UINT>(mVertexBufferViews.size()), mVertexBufferViews.data());
	if (mIndexed) {
		commandList->IASetIndexBuffer(&mIndexBufferView);
	}
}

bool PlainMesh::GetIndexed() const {
	return mIndexed;
}

UINT PlainMesh::GetUnitCount() const {
	return mUnitCount;
}
