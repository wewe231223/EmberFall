#include "pch.h"
#include "TerrainObject.h"

TerrainSegment::TerrainSegment() {

}

TerrainSegment::TerrainSegment(MeshData& data, Mesh* mesh) {
	mMesh = mesh;
}

TerrainSegment::~TerrainSegment() {

}

DirectX::BoundingBox& TerrainSegment::GetBB() {
	return mBoundingBox; 
}

Mesh* TerrainSegment::GetMesh() const {
	return mMesh;
}
