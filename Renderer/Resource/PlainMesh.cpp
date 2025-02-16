#include "pch.h"
#include "PlainMesh.h"
#include "../External/Include/DirectXTK12/GeometricPrimitive.h"


PlainMesh::PlainMesh() {
}

PlainMesh::PlainMesh(ComPtr<ID3D12Device> device, EmbeddedMeshType type, UINT size) {
	
	std::vector<SimpleMath::Vector3> positions; 
	std::vector<SimpleMath::Vector3> normals;
	std::vector<SimpleMath::Vector2> texcoords;

	std::vector<UINT> indices;

	switch (type) {
	case EmbeddedMeshType::Plane:
	{
		float halfSize = size * 0.5f;
		SimpleMath::Vector3 planePositions[] = {
			{ -halfSize , 0.0f, -halfSize }, { halfSize, 0.0f, -halfSize }, { halfSize, 0.0f, halfSize }, { -halfSize, 0.0f, halfSize }
		};
		SimpleMath::Vector3 planeNormals[] = {
			{ 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }
		};

		SimpleMath::Vector2 planeTexcoords[] = {
			{ 0.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f }
		};

		UINT planeIndices[] = {
			0, 1, 2, 0, 2, 3
		};

		positions.assign(planePositions, planePositions + 4);
		normals.assign(planeNormals, planeNormals + 4);
		texcoords.assign(planeTexcoords, planeTexcoords + 4);

		indices.assign(planeIndices, planeIndices + 6);

		mIndexed = true;
		mUnitCount = 6;

		mPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	}
		break;
	case EmbeddedMeshType::Cube:
	{
		float halfSize = size * 0.5f;

		SimpleMath::Vector3 cubeNormals[] = {
			{0, 0, -1}, {0, 0, 1}, {0, -1, 0}, {0, 1, 0}, {-1, 0, 0}, {1, 0, 0}
		};

		SimpleMath::Vector3 cubePositions[] = {
			{-halfSize, -halfSize, -halfSize}, {halfSize, -halfSize, -halfSize},
			{halfSize, halfSize, -halfSize}, {-halfSize, halfSize, -halfSize},
			{-halfSize, -halfSize, halfSize}, {halfSize, -halfSize, halfSize},
			{halfSize, halfSize, halfSize}, {-halfSize, halfSize, halfSize}
		};

		SimpleMath::Vector2 cubeTexcoords[] = {
			{0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}
		};

		UINT cubeFaces[][4] = {
			{0, 1, 2, 3},  // Front
			{4, 5, 6, 7},  // Back
			{0, 4, 7, 3},  // Left
			{1, 5, 6, 2},  // Right
			{0, 1, 5, 4},  // Bottom
			{3, 2, 6, 7}   // Top
		};

		for (int face = 0; face < 6; ++face) {
			for (int i = 0; i < 2; ++i) {
				positions.push_back(cubePositions[cubeFaces[face][0]]);
				normals.push_back(cubeNormals[face]);
				texcoords.push_back(cubeTexcoords[0]);

				positions.push_back(cubePositions[cubeFaces[face][1]]);
				normals.push_back(cubeNormals[face]);
				texcoords.push_back(cubeTexcoords[1]);

				positions.push_back(cubePositions[cubeFaces[face][2]]);
				normals.push_back(cubeNormals[face]);
				texcoords.push_back(cubeTexcoords[2]);

				positions.push_back(cubePositions[cubeFaces[face][0]]);
				normals.push_back(cubeNormals[face]);
				texcoords.push_back(cubeTexcoords[0]);

				positions.push_back(cubePositions[cubeFaces[face][2]]);
				normals.push_back(cubeNormals[face]);
				texcoords.push_back(cubeTexcoords[2]);

				positions.push_back(cubePositions[cubeFaces[face][3]]);
				normals.push_back(cubeNormals[face]);
				texcoords.push_back(cubeTexcoords[3]);
			}
		}
	}
		break;
	case EmbeddedMeshType::Sphere:
	{
		constexpr size_t tessellation = 16;

		const size_t verticalSegments = tessellation;	
		const size_t horizontalSegments = tessellation * 2;

		const float radius = size * 0.5f;

		for (size_t i = 0; i <= verticalSegments; ++i) {
			float v = 1 - (float)i / verticalSegments;

			float latitude = ((float)i * DirectX::XM_PI / verticalSegments) - DirectX::XM_PIDIV2;
			float dy, dxz;
			
			DirectX::XMScalarSinCos(&dy, &dxz, latitude);

			for (size_t j = 0; j <= horizontalSegments; ++j) {
				float u = (float)j / horizontalSegments;

				float longitude = j * DirectX::XM_2PI / horizontalSegments;
				float dx, dz;
				DirectX::XMScalarSinCos(&dx, &dz, longitude);

				dx *= dxz;
				dz *= dxz;

				SimpleMath::Vector3 normal(dx, dy, dz);
				SimpleMath::Vector3 position = normal * radius;

				positions.push_back(position);
				normals.push_back(normal);
				texcoords.push_back({ u, v });
			}
		}
	}
		break;
	default:
		break;
	}
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
