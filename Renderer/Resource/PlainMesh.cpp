#include "pch.h"
#include "PlainMesh.h"
#include "../Utility/Defines.h"

PlainMesh::PlainMesh() {
}

PlainMesh::PlainMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, EmbeddedMeshType type, UINT size) {

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

		mIndexed = false;
		mUnitCount = static_cast<UINT>(positions.size());
	}
	break;
	case EmbeddedMeshType::Sphere:
	{
		constexpr UINT tessellation = 16;

		const UINT verticalSegments = tessellation;
		const UINT horizontalSegments = tessellation * 2;

		const float radius = size * 0.5f;

		for (UINT i = 0; i <= verticalSegments; ++i) {
			float v = 1 - (float)i / verticalSegments;

			float latitude = ((float)i * DirectX::XM_PI / verticalSegments) - DirectX::XM_PIDIV2;
			float dy, dxz;

			DirectX::XMScalarSinCos(&dy, &dxz, latitude);

			for (UINT j = 0; j <= horizontalSegments; ++j) {
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

		const UINT stride = horizontalSegments + 1;

		for (UINT i = 0; i < verticalSegments; ++i) {
			for (UINT j = 0; j <= horizontalSegments; ++j) {
				UINT nextI = i + 1;
				UINT nextJ = (j + 1) % stride;

				// 와인딩 순서를 반대로 변경 (CW → CCW)
				indices.push_back(i * stride + j);
				indices.push_back(i * stride + nextJ);
				indices.push_back(nextI * stride + j);

				indices.push_back(i * stride + nextJ);
				indices.push_back(nextI * stride + nextJ);
				indices.push_back(nextI * stride + j);
			}
		}


		mIndexed = true;
		mUnitCount = static_cast<UINT>(indices.size());
	}
	break;
	default:
		break;
	}

	if (mIndexed) {
		mIndexBuffer = DefaultBuffer(device, commandList, sizeof(UINT), indices.size(), indices.data());
		mIndexBufferView.BufferLocation = *mIndexBuffer.GPUBegin();
		mIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
		mIndexBufferView.SizeInBytes = static_cast<UINT>(indices.size()) * sizeof(UINT);
	}

	{
		auto& buffer = mVertexBuffers.emplace_back(device, commandList, sizeof(SimpleMath::Vector3), positions.size(), positions.data());

		D3D12_VERTEX_BUFFER_VIEW view{};
		view.BufferLocation = *buffer.GPUBegin();
		view.StrideInBytes = sizeof(SimpleMath::Vector3);
		view.SizeInBytes = static_cast<UINT>(positions.size()) * sizeof(SimpleMath::Vector3);
		mVertexBufferViews[0] = view;
	}
	
	{
		auto& buffer = mVertexBuffers.emplace_back(device, commandList, sizeof(SimpleMath::Vector3), normals.size(), normals.data());

		D3D12_VERTEX_BUFFER_VIEW view{};
		view.BufferLocation = *buffer.GPUBegin();
		view.StrideInBytes = sizeof(SimpleMath::Vector3);
		view.SizeInBytes = static_cast<UINT>(normals.size()) * sizeof(SimpleMath::Vector3);
		mVertexBufferViews[1] = view;
	}

	{
		auto& buffer = mVertexBuffers.emplace_back(device, commandList, sizeof(SimpleMath::Vector2), texcoords.size(), texcoords.data());

		D3D12_VERTEX_BUFFER_VIEW view{};
		view.BufferLocation = *buffer.GPUBegin();
		view.StrideInBytes = sizeof(SimpleMath::Vector2);
		view.SizeInBytes = static_cast<UINT>(texcoords.size()) * sizeof(SimpleMath::Vector2);
		mVertexBufferViews[2] = view;
	}

	// position, normal, texcoord
	mAttribute.set(0);
	mAttribute.set(1);
	mAttribute.set(2);
}

PlainMesh::PlainMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, const MeshData& meshData) {
	mAttribute = meshData.vertexAttribute;

	if (meshData.vertexAttribute[0] == 1) {
		auto& buffer = mVertexBuffers.emplace_back(device, commandList, sizeof(SimpleMath::Vector3), meshData.position.size(), meshData.position.data());

		D3D12_VERTEX_BUFFER_VIEW view{};
		view.BufferLocation = *buffer.GPUBegin();
		view.StrideInBytes = sizeof(SimpleMath::Vector3);
		view.SizeInBytes = static_cast<UINT>(meshData.position.size()) * sizeof(SimpleMath::Vector3);
		mVertexBufferViews[0] = view;
	}

	if (meshData.vertexAttribute[1] == 1) {
		auto& buffer = mVertexBuffers.emplace_back(device, commandList, sizeof(SimpleMath::Vector3), meshData.normal.size(), meshData.normal.data());

		D3D12_VERTEX_BUFFER_VIEW view{};
		view.BufferLocation = *buffer.GPUBegin();
		view.StrideInBytes = sizeof(SimpleMath::Vector3);
		view.SizeInBytes = static_cast<UINT>(meshData.normal.size()) * sizeof(SimpleMath::Vector3);
		mVertexBufferViews[1] = view;
	}

	if (meshData.vertexAttribute[2] == 1) {
		auto& buffer = mVertexBuffers.emplace_back(device, commandList, sizeof(SimpleMath::Vector2), meshData.texCoord1.size(), meshData.texCoord1.data());

		D3D12_VERTEX_BUFFER_VIEW view{};
		view.BufferLocation = *buffer.GPUBegin();
		view.StrideInBytes = sizeof(SimpleMath::Vector2);
		view.SizeInBytes = static_cast<UINT>(meshData.texCoord1.size()) * sizeof(SimpleMath::Vector2);
		mVertexBufferViews[2] = view;
	}

	if (meshData.vertexAttribute[3] == 1) {
		auto& buffer = mVertexBuffers.emplace_back(device, commandList, sizeof(SimpleMath::Vector2), meshData.texCoord2.size(), meshData.texCoord2.data());

		D3D12_VERTEX_BUFFER_VIEW view{};
		view.BufferLocation = *buffer.GPUBegin();
		view.StrideInBytes = sizeof(SimpleMath::Vector2);
		view.SizeInBytes = static_cast<UINT>(meshData.texCoord2.size()) * sizeof(SimpleMath::Vector2);
		mVertexBufferViews[3] = view;
	}

	if (meshData.vertexAttribute[4] == 1) {
		auto& buffer = mVertexBuffers.emplace_back(device, commandList, sizeof(SimpleMath::Vector3), meshData.tangent.size(), meshData.tangent.data());

		D3D12_VERTEX_BUFFER_VIEW view{};
		view.BufferLocation = *buffer.GPUBegin();
		view.StrideInBytes = sizeof(SimpleMath::Vector3);
		view.SizeInBytes = static_cast<UINT>(meshData.tangent.size()) * sizeof(SimpleMath::Vector3);
		mVertexBufferViews[4] = view;
	}

	if (meshData.vertexAttribute[5] == 1) {
		auto& buffer = mVertexBuffers.emplace_back(device, commandList, sizeof(SimpleMath::Vector3), meshData.bitangent.size(), meshData.bitangent.data());

		D3D12_VERTEX_BUFFER_VIEW view{};
		view.BufferLocation = *buffer.GPUBegin();
		view.StrideInBytes = sizeof(SimpleMath::Vector3);
		view.SizeInBytes = static_cast<UINT>(meshData.bitangent.size()) * sizeof(SimpleMath::Vector3);
		mVertexBufferViews[5] = view;
	}

	if (meshData.vertexAttribute[6] == 1) {
		auto& buffer = mVertexBuffers.emplace_back(device, commandList, sizeof(SimpleMath::Vector4), meshData.boneID.size(), meshData.boneID.data());

		D3D12_VERTEX_BUFFER_VIEW view{};
		view.BufferLocation = *buffer.GPUBegin();
		view.StrideInBytes = sizeof(unsigned int) * 4;
		view.SizeInBytes = static_cast<UINT>(meshData.boneID.size()) * sizeof(unsigned int) * 4;
		mVertexBufferViews[6] = view;
	}

	if (meshData.vertexAttribute[7] == 1) {
		auto& buffer = mVertexBuffers.emplace_back(device, commandList, sizeof(SimpleMath::Vector4), meshData.boneWeight.size(), meshData.boneWeight.data());

		D3D12_VERTEX_BUFFER_VIEW view{};
		view.BufferLocation = *buffer.GPUBegin();
		view.StrideInBytes = sizeof(float) * 4;
		view.SizeInBytes = static_cast<UINT>(meshData.boneWeight.size()) * sizeof(float) * 4;
		mVertexBufferViews[7] = view;
	}

	if (meshData.indexed) {
		mIndexBuffer = DefaultBuffer{ device, commandList, sizeof(unsigned int), meshData.index.size(), meshData.index.data() };
		mIndexBufferView.BufferLocation = *mIndexBuffer.GPUBegin();
		mIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
		mIndexBufferView.SizeInBytes = static_cast<UINT>(meshData.index.size()) * sizeof(UINT);
		
		mIndexed = true;
	}
	mUnitCount = meshData.unitCount;
	mPrimitiveTopology = meshData.primitiveTopology;
}

PlainMesh::PlainMesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, const std::filesystem::path& binPath) {

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
		mUnitCount = other.mUnitCount;
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
		mUnitCount = other.mUnitCount;
	}
	return *this;
}

void PlainMesh::Bind(ComPtr<ID3D12GraphicsCommandList> commandList, const std::bitset<8>& shaderAttribute) const {
	
	CrashExp(IsSubSet(mAttribute, shaderAttribute), "Attribute is not subset of PlainMesh attribute");

	commandList->IASetPrimitiveTopology(mPrimitiveTopology);
	for (auto i = 0; i < 8; ++i) {
		if (shaderAttribute[i] == 1) {
			commandList->IASetVertexBuffers(i, 1, &mVertexBufferViews[i]);
		}
	}

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
