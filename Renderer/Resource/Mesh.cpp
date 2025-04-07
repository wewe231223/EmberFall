#include "pch.h"
#include "Mesh.h"
#include "../Utility/Defines.h"

Mesh::Mesh() {
}

Mesh::Mesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, EmbeddedMeshType type, UINT size) {

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
			0, 2, 1, 0, 3, 2
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

		DirectX::XMFLOAT3 cubePositions[] = {
			// Front Face
			{-halfSize, -halfSize,  halfSize}, { halfSize, -halfSize,  halfSize}, {-halfSize,  halfSize,  halfSize},
			{ halfSize, -halfSize,  halfSize}, { halfSize,  halfSize,  halfSize}, {-halfSize,  halfSize,  halfSize},
			// Back Face
			{ halfSize, -halfSize, -halfSize}, {-halfSize, -halfSize, -halfSize}, { halfSize,  halfSize, -halfSize},
			{-halfSize, -halfSize, -halfSize}, {-halfSize,  halfSize, -halfSize}, { halfSize,  halfSize, -halfSize},
			// Left Face
			{-halfSize, -halfSize, -halfSize}, {-halfSize, -halfSize,  halfSize}, {-halfSize,  halfSize, -halfSize},
			{-halfSize, -halfSize,  halfSize}, {-halfSize,  halfSize,  halfSize}, {-halfSize,  halfSize, -halfSize},
			// Right Face
			{ halfSize, -halfSize,  halfSize}, { halfSize, -halfSize, -halfSize}, { halfSize,  halfSize,  halfSize},
			{ halfSize, -halfSize, -halfSize}, { halfSize,  halfSize, -halfSize}, { halfSize,  halfSize,  halfSize},
			// Top Face
			{-halfSize,  halfSize,  halfSize}, { halfSize,  halfSize,  halfSize}, {-halfSize,  halfSize, -halfSize},
			{ halfSize,  halfSize,  halfSize}, { halfSize,  halfSize, -halfSize}, {-halfSize,  halfSize, -halfSize},
			// Bottom Face
			{-halfSize, -halfSize, -halfSize}, { halfSize, -halfSize, -halfSize}, {-halfSize, -halfSize,  halfSize},
			{ halfSize, -halfSize, -halfSize}, { halfSize, -halfSize,  halfSize}, {-halfSize, -halfSize,  halfSize}
		};

		DirectX::XMFLOAT3 cubeNormals[] = {
		  {0.0f,  0.0f,  1.0f}, {0.0f,  0.0f,  1.0f}, {0.0f,  0.0f,  1.0f},
		  {0.0f,  0.0f,  1.0f}, {0.0f,  0.0f,  1.0f}, {0.0f,  0.0f,  1.0f},
		  {0.0f,  0.0f, -1.0f}, {0.0f,  0.0f, -1.0f}, {0.0f,  0.0f, -1.0f},
		  {0.0f,  0.0f, -1.0f}, {0.0f,  0.0f, -1.0f}, {0.0f,  0.0f, -1.0f},
		  {-1.0f,  0.0f,  0.0f}, {-1.0f,  0.0f,  0.0f}, {-1.0f,  0.0f,  0.0f},
		  {-1.0f,  0.0f,  0.0f}, {-1.0f,  0.0f,  0.0f}, {-1.0f,  0.0f,  0.0f},
		  {1.0f,  0.0f,  0.0f}, {1.0f,  0.0f,  0.0f}, {1.0f,  0.0f,  0.0f},
		  {1.0f,  0.0f,  0.0f}, {1.0f,  0.0f,  0.0f}, {1.0f,  0.0f,  0.0f},
		  {0.0f,  1.0f,  0.0f}, {0.0f,  1.0f,  0.0f}, {0.0f,  1.0f,  0.0f},
		  {0.0f,  1.0f,  0.0f}, {0.0f,  1.0f,  0.0f}, {0.0f,  1.0f,  0.0f},
		  {0.0f, -1.0f,  0.0f}, {0.0f, -1.0f,  0.0f}, {0.0f, -1.0f,  0.0f},
		  {0.0f, -1.0f,  0.0f}, {0.0f, -1.0f,  0.0f}, {0.0f, -1.0f,  0.0f}
		};

		DirectX::XMFLOAT2 cubeTexcoords[] = {
		{0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f},
		{1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f},
		{0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f},
		{1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f},
		{0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f},
		{1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f},
		{0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f},
		{1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f},
		{0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f},
		{1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f},
		{0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f},
		{1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f},
		};


		positions.assign(cubePositions, cubePositions + 36);
		normals.assign(cubeNormals, cubeNormals + 36);
		texcoords.assign(cubeTexcoords, cubeTexcoords + 36);

		mIndexed = false;
		mUnitCount = 36;

		mPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

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
	case EmbeddedMeshType::SkyBox:
	{
		float boxSize = size / 2.f;

		SimpleMath::Vector3 boxPositions[] = {
			// Front face
			{-boxSize, -boxSize, -boxSize},
			{-boxSize,  boxSize, -boxSize},
			{ boxSize,  boxSize, -boxSize},
			{ boxSize, -boxSize, -boxSize},

			// Back face
			{ boxSize, -boxSize,  boxSize},
			{ boxSize,  boxSize,  boxSize},
			{-boxSize,  boxSize,  boxSize},
			{-boxSize, -boxSize,  boxSize},

			// Top face
			{-boxSize,  boxSize, -boxSize},
			{-boxSize,  boxSize,  boxSize},
			{ boxSize,  boxSize,  boxSize},
			{ boxSize,  boxSize, -boxSize},

			// Bottom face
			{-boxSize, -boxSize,  boxSize},
			{-boxSize, -boxSize, -boxSize},
			{ boxSize, -boxSize, -boxSize},
			{ boxSize, -boxSize,  boxSize},

			// Left face
			{-boxSize, -boxSize,  boxSize},
			{-boxSize,  boxSize,  boxSize},
			{-boxSize,  boxSize, -boxSize},
			{-boxSize, -boxSize, -boxSize},

			// Right face
			{ boxSize, -boxSize, -boxSize},
			{ boxSize,  boxSize, -boxSize},
			{ boxSize,  boxSize,  boxSize},
			{ boxSize, -boxSize,  boxSize},
		};

		SimpleMath::Vector2 boxUvs[] = {
			// Front face
			{0.0f, 1.0f},
			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{1.0f, 1.0f},

			// Back face
			{0.0f, 1.0f},
			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{1.0f, 1.0f},

			// Top face
			{0.0f, 1.0f},
			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{1.0f, 1.0f},

			// Bottom face
			{0.0f, 1.0f},
			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{1.0f, 1.0f},

			// Left face
			{0.0f, 1.0f},
			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{1.0f, 1.0f},

			// Right face
			{0.0f, 1.0f},
			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{1.0f, 1.0f},
		};

		UINT boxIndices[] = {
			// Front face
			0, 2, 1, 0, 3, 2,
			// Back face
			4, 6, 5, 4, 7, 6,
			// Top face
			8, 10, 9, 8, 11, 10,
			// Bottom face
			12, 14, 13, 12, 15, 14,
			// Left face
			16, 18, 17, 16, 19, 18,
			// Right face
			20, 22, 21, 20, 23, 22,
		};

		positions.insert(positions.end(), std::begin(boxPositions), std::end(boxPositions));
		texcoords.insert(texcoords.end(), std::begin(boxUvs), std::end(boxUvs));
		indices.insert(indices.end(), std::begin(boxIndices), std::end(boxIndices));

		mIndexed = true;
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

	if(!positions.empty()){
		auto& buffer = mVertexBuffers.emplace_back(device, commandList, sizeof(SimpleMath::Vector3), positions.size(), positions.data());

		D3D12_VERTEX_BUFFER_VIEW view{};
		view.BufferLocation = *buffer.GPUBegin();
		view.StrideInBytes = sizeof(SimpleMath::Vector3);
		view.SizeInBytes = static_cast<UINT>(positions.size()) * sizeof(SimpleMath::Vector3);
		mVertexBufferViews[0] = view;
		mAttribute.set(0);
	}
	
	if(!normals.empty()) {
		auto& buffer = mVertexBuffers.emplace_back(device, commandList, sizeof(SimpleMath::Vector3), normals.size(), normals.data());

		D3D12_VERTEX_BUFFER_VIEW view{};
		view.BufferLocation = *buffer.GPUBegin();
		view.StrideInBytes = sizeof(SimpleMath::Vector3);
		view.SizeInBytes = static_cast<UINT>(normals.size()) * sizeof(SimpleMath::Vector3);
		mVertexBufferViews[1] = view;
		mAttribute.set(1);
	}

	if(!texcoords.empty()){
		auto& buffer = mVertexBuffers.emplace_back(device, commandList, sizeof(SimpleMath::Vector2), texcoords.size(), texcoords.data());

		D3D12_VERTEX_BUFFER_VIEW view{};
		view.BufferLocation = *buffer.GPUBegin();
		view.StrideInBytes = sizeof(SimpleMath::Vector2);
		view.SizeInBytes = static_cast<UINT>(texcoords.size()) * sizeof(SimpleMath::Vector2);
		mVertexBufferViews[2] = view;
		mAttribute.set(2);
	}
}

Mesh::Mesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, float size) {
	std::vector<SimpleMath::Vector3> positions;
	std::vector<UINT> texIndices;
	std::vector<SimpleMath::Vector2> texcoords;

	std::vector<UINT> indices;

	float boxSize = size / 2.f;

	SimpleMath::Vector3 boxPositions[] = {
		// Front face
		{-boxSize, -boxSize, -boxSize},
		{-boxSize,  boxSize, -boxSize},
		{ boxSize,  boxSize, -boxSize},
		{ boxSize, -boxSize, -boxSize},

		// Back face
		{ boxSize, -boxSize,  boxSize},
		{ boxSize,  boxSize,  boxSize},
		{-boxSize,  boxSize,  boxSize},
		{-boxSize, -boxSize,  boxSize},

		// Top face
		{-boxSize,  boxSize, -boxSize},
		{-boxSize,  boxSize,  boxSize},
		{ boxSize,  boxSize,  boxSize},
		{ boxSize,  boxSize, -boxSize},

		// Bottom face
		{-boxSize, -boxSize,  boxSize},
		{-boxSize, -boxSize, -boxSize},
		{ boxSize, -boxSize, -boxSize},
		{ boxSize, -boxSize,  boxSize},

		// Left face
		{-boxSize, -boxSize,  boxSize},
		{-boxSize,  boxSize,  boxSize},
		{-boxSize,  boxSize, -boxSize},
		{-boxSize, -boxSize, -boxSize},

		// Right face
		{ boxSize, -boxSize, -boxSize},
		{ boxSize,  boxSize, -boxSize},
		{ boxSize,  boxSize,  boxSize},
		{ boxSize, -boxSize,  boxSize},
	};

	SimpleMath::Vector2 boxUvs[] = {
		// Front face
		{0.0f, 1.0f},
		{0.0f, 0.0f},
		{1.0f, 0.0f},
		{1.0f, 1.0f},

		// Back face
		{0.0f, 1.0f},
		{0.0f, 0.0f},
		{1.0f, 0.0f},
		{1.0f, 1.0f},

		// Top face
		{0.0f, 1.0f},
		{0.0f, 0.0f},
		{1.0f, 0.0f},
		{1.0f, 1.0f},

		// Bottom face
		{0.0f, 1.0f},
		{0.0f, 0.0f},
		{1.0f, 0.0f},
		{1.0f, 1.0f},

		// Left face
		{0.0f, 1.0f},
		{0.0f, 0.0f},
		{1.0f, 0.0f},
		{1.0f, 1.0f},

		// Right face
		{0.0f, 1.0f},
		{0.0f, 0.0f},
		{1.0f, 0.0f},
		{1.0f, 1.0f},
	};

	UINT texIndex[] = {
		0, 0, 0, 0,
		1, 1, 1, 1,
		2, 2, 2, 2,
		3, 3, 3, 3,
		4, 4, 4, 4,
		5, 5, 5, 5,
	};

	UINT boxIndices[] = {
		// Front face
		0, 2, 1, 0, 3, 2,
		// Back face
		4, 6, 5, 4, 7, 6,
		// Top face
		8, 10, 9, 8, 11, 10,
		// Bottom face
		12, 14, 13, 12, 15, 14,
		// Left face
		16, 18, 17, 16, 19, 18,
		// Right face
		20, 22, 21, 20, 23, 22,
	};

	positions.insert(positions.end(), std::begin(boxPositions), std::end(boxPositions));
	texIndices.insert(texIndices.end(), std::begin(texIndex), std::end(texIndex));
	texcoords.insert(texcoords.end(), std::begin(boxUvs), std::end(boxUvs));

	indices.insert(indices.end(), std::begin(boxIndices), std::end(boxIndices));

	{
		auto& buffer = mVertexBuffers.emplace_back(device, commandList, sizeof(SimpleMath::Vector3), positions.size(), positions.data());

		D3D12_VERTEX_BUFFER_VIEW view{};
		view.BufferLocation = *buffer.GPUBegin();
		view.StrideInBytes = sizeof(SimpleMath::Vector3);
		view.SizeInBytes = static_cast<UINT>(positions.size()) * sizeof(SimpleMath::Vector3);
		mVertexBufferViews[0] = view;
		mAttribute.set(0);
	}

	{
		auto& buffer = mVertexBuffers.emplace_back(device, commandList, sizeof(UINT), texIndices.size(), texIndices.data());

		D3D12_VERTEX_BUFFER_VIEW view{};
		view.BufferLocation = *buffer.GPUBegin();
		view.StrideInBytes = sizeof(UINT);
		view.SizeInBytes = static_cast<UINT>(texIndices.size()) * sizeof(UINT);
		mVertexBufferViews[1] = view;
		mAttribute.set(1);
	}

	{
		auto& buffer = mVertexBuffers.emplace_back(device, commandList, sizeof(SimpleMath::Vector2), texcoords.size(), texcoords.data());

		D3D12_VERTEX_BUFFER_VIEW view{};
		view.BufferLocation = *buffer.GPUBegin();
		view.StrideInBytes = sizeof(SimpleMath::Vector2);
		view.SizeInBytes = static_cast<UINT>(texcoords.size()) * sizeof(SimpleMath::Vector2);
		mVertexBufferViews[2] = view;
		mAttribute.set(2);
	}


	mIndexBuffer = DefaultBuffer{ device, commandList, sizeof(UINT), indices.size(), indices.data() };
	mIndexBufferView.BufferLocation = *mIndexBuffer.GPUBegin();
	mIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	mIndexBufferView.SizeInBytes = static_cast<UINT>(indices.size()) * sizeof(UINT);

	mUnitCount = static_cast<UINT>(indices.size());
	mIndexed = true;
	mPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

Mesh::Mesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, float radius, float height, UINT segments) {

	std::vector<SimpleMath::Vector3> positions{};

	std::vector<UINT> indices{}; 


	for (unsigned int i = 0; i < segments; ++i) {
		float angle = 2.0f * static_cast<float>(DirectX::XM_PI) * i / segments;
		float x = radius * std::cos(angle);
		float z = radius * std::sin(angle);
		float yBottom = -height / 2.0f;
		float yTop = height / 2.0f;

		positions.emplace_back( x, yBottom, z );
		positions.emplace_back( x, yTop, z );
	}

	for (unsigned int i = 0; i < segments; ++i) {
		unsigned int next = (i + 1) % segments;

		unsigned int bottom_i = 2 * i;
		unsigned int top_i = 2 * i + 1;
		unsigned int bottom_next = 2 * next;
		unsigned int top_next = 2 * next + 1;

		// 삼각형 1: (bottom_i, bottom_next, top_i)
		indices.push_back(bottom_i);
		indices.push_back(bottom_next);
		indices.push_back(top_i);

		// 삼각형 2: (top_i, bottom_next, top_next)
		indices.push_back(top_i);
		indices.push_back(bottom_next);
		indices.push_back(top_next);
	}

	{
		auto& buffer = mVertexBuffers.emplace_back(device, commandList, sizeof(SimpleMath::Vector3), positions.size(), positions.data());
		D3D12_VERTEX_BUFFER_VIEW view{};
		view.BufferLocation = *buffer.GPUBegin();
		view.StrideInBytes = sizeof(SimpleMath::Vector3);
		view.SizeInBytes = static_cast<UINT>(positions.size()) * sizeof(SimpleMath::Vector3);

		mVertexBufferViews[0] = view;
		mAttribute.set(0);
	}

	mIndexed = true;
	mIndexBuffer = DefaultBuffer{ device, commandList, sizeof(UINT), indices.size(), indices.data() };
	mIndexBufferView.BufferLocation = *mIndexBuffer.GPUBegin();
	mIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	mIndexBufferView.SizeInBytes = static_cast<UINT>(indices.size()) * sizeof(UINT);

	mUnitCount = static_cast<UINT>(indices.size());
	mPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

Mesh::Mesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, const MeshData& meshData) {
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

Mesh::Mesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, const std::filesystem::path& binPath) {
	
}

Mesh::~Mesh() {
}

Mesh::Mesh(const Mesh& other) {
	mVertexBuffers = other.mVertexBuffers;
	mIndexBuffer = other.mIndexBuffer;

	mVertexBufferViews = other.mVertexBufferViews;
	mIndexBufferView = other.mIndexBufferView;

	mPrimitiveTopology = other.mPrimitiveTopology;
	mIndexed = other.mIndexed;
	mUnitCount = other.mUnitCount;
}

Mesh& Mesh::operator=(const Mesh& other) {
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

Mesh::Mesh(Mesh&& other) noexcept {
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

Mesh& Mesh::operator=(Mesh&& other) noexcept {
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

void Mesh::Bind(ComPtr<ID3D12GraphicsCommandList> commandList, const std::bitset<8>& shaderAttribute) const {
	
	CrashExp(IsSubSet(mAttribute, shaderAttribute), "Attribute is not subset of PlainMesh attribute");

	const static D3D12_VERTEX_BUFFER_VIEW clearViews[8]{};
	commandList->IASetVertexBuffers(0, 8, clearViews);

	const static D3D12_INDEX_BUFFER_VIEW clearIndexView{};
	commandList->IASetIndexBuffer(&clearIndexView);
	

	commandList->IASetPrimitiveTopology(mPrimitiveTopology);
	UINT slot{ 0 };
	for (auto i = 0; i < 8; ++i) {
		if (shaderAttribute[i] == 1) {
			commandList->IASetVertexBuffers(slot++, 1, &mVertexBufferViews[i]);
		}
	}

	if (mIndexed) {
		commandList->IASetIndexBuffer(&mIndexBufferView);
	}
}

bool Mesh::GetIndexed() const {
	return mIndexed;
}

UINT Mesh::GetUnitCount() const {
	return mUnitCount;
}
