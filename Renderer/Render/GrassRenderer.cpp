#include "pch.h"
#include "GrassRenderer.h"
#include <random>
#include <fstream>
#include <filesystem>
#include "../Utility/Exceptions.h"
#include "../Game/System/Timer.h"

GrassRenderer::GrassRenderer(ComPtr<ID3D12Device10> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	std::vector<GrassPoint> points{};
	points.resize(GRASS_INSTANCE_COUNT<size_t>);

	std::ifstream file{ "Resources/Binarys/Terrain/grass.bin", std::ios::binary };

	if (not file) {
		Crash("Failed to open grass.bin file");
	}

	file.read(reinterpret_cast<char*>(points.data()), sizeof(GrassPoint) * GRASS_INSTANCE_COUNT<size_t>); 

	mGrassPosition = DefaultBuffer(device, commandList, sizeof(GrassPoint), points.size(), points.data());
	
	ComPtr<IDxcUtils> dxcUtils{};
	ComPtr<IDxcCompiler3> dxcCompiler{};
	ComPtr<IDxcIncludeHandler> includeHandler{};

	CheckHR(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils)));
	CheckHR(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler)));
	
	CheckHR(dxcUtils->CreateDefaultIncludeHandler(&includeHandler));

	const std::filesystem::path shaderPath{ L"Shader/Sources/GrassShader.hlsl" };

	ComPtr<IDxcBlobEncoding> sourceBlob{};
	CheckHR(dxcUtils->LoadFile(shaderPath.c_str(), nullptr, &sourceBlob));

#ifdef _DEBUG 
	LPCWSTR meshShaderArgs[] = {
		shaderPath.c_str(),         // 소스 파일 경로
		L"-T", L"ms_6_5",           // Target: Mesh Shader 6.5
		L"-E", L"mainMS",           // Entry point
		L"-Zi",                     // 디버그 정보 포함
		L"-Qembed_debug",			// cso 에 디버그 정보 포함 
		L"-Od",                     // 최적화 비활성화 (디버깅용)
	};

	LPCWSTR amplificationShaderArgs[] = {
		shaderPath.c_str(),         // 소스 파일 경로
		L"-T", L"as_6_5",           // Target: Amplification Shader 6.5
		L"-E", L"mainAS",           // Entry point
		L"-Zi",                     // 디버그 정보 포함
		L"-Qembed_debug",			// cso 에 디버그 정보 포함 
		L"-Od",                     // 최적화 비활성화 (디버깅용)
	};

	LPCWSTR pixelShaderArgs[] = {
		shaderPath.c_str(),         // 소스 파일 경로
		L"-T", L"ps_6_0",           // Target: Pixel Shader 6.5
		L"-E", L"mainPS",           // Entry point
		L"-Zi",                     // 디버그 정보 포함
		L"-Qembed_debug",			// cso 에 디버그 정보 포함 
		L"-Od",                     // 최적화 비활성화 (디버깅용)
	};
#else 
	LPCWSTR meshShaderArgs[] = {
		shaderPath.c_str(),         // 소스 파일 경로 
		L"-T", L"ms_6_5",			// Target: Mesh Shader 6.5
		L"-E", L"mainMS",
		L"-O3",						// <- 최적화 최대화
	};

	LPCWSTR amplificationShaderArgs[] = {
		shaderPath.c_str(),         // 소스 파일 경로
		L"-T", L"as_6_5",           // Target: Amplification Shader 6.5
		L"-E", L"mainAS",             // Entry point
		L"-O3",						// <- 최적화 최대화
	};

	LPCWSTR pixelShaderArgs[] = {
		shaderPath.c_str(),         // 소스 파일 경로
		L"-T", L"ps_6_0",           // Target: Pixel Shader 6.5
		L"-E", L"mainPS",             // Entry point
		L"-O3",						// <- 최적화 최대화
	};
#endif 


	DxcBuffer sourceBuffer{};
	sourceBuffer.Ptr = sourceBlob->GetBufferPointer();
	sourceBuffer.Size = sourceBlob->GetBufferSize();
	sourceBuffer.Encoding = DXC_CP_UTF8;

	ComPtr<IDxcBlobUtf8> errors{};
	ComPtr<IDxcBlobUtf16> name{};

	ComPtr<IDxcResult> meshShaderResult{};
	CheckHR(dxcCompiler->Compile(&sourceBuffer, meshShaderArgs, _countof(meshShaderArgs), includeHandler.Get(), IID_PPV_ARGS(&meshShaderResult)));

	CheckHR(meshShaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), name.GetAddressOf()));
	if (errors and errors->GetStringLength() > 0) {
		OutputDebugStringA((char*)errors->GetStringPointer());
		Crash(false); 
	} 

	ComPtr<IDxcResult> amplificationShaderResult{};
	CheckHR(dxcCompiler->Compile(&sourceBuffer, amplificationShaderArgs, _countof(amplificationShaderArgs), includeHandler.Get(), IID_PPV_ARGS(&amplificationShaderResult)));

	CheckHR(amplificationShaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), name.GetAddressOf()));
	if (errors and errors->GetStringLength() > 0) {
		OutputDebugStringA((char*)errors->GetStringPointer());
		Crash(false);
	}

	ComPtr<IDxcResult> pixelShaderResult{};
	CheckHR(dxcCompiler->Compile(&sourceBuffer, pixelShaderArgs, _countof(pixelShaderArgs), includeHandler.Get(), IID_PPV_ARGS(&pixelShaderResult)));

	CheckHR(pixelShaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), name.GetAddressOf()));
	if (errors and errors->GetStringLength() > 0) {
		OutputDebugStringA((char*)errors->GetStringPointer());
		Crash(false);
	}

	ComPtr<IDxcBlob> meshShaderBinary{};
	CheckHR(meshShaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&meshShaderBinary), name.GetAddressOf()));

	ComPtr<IDxcBlob> amplificationShaderBinary{};
	CheckHR(amplificationShaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&amplificationShaderBinary), name.GetAddressOf()));

	ComPtr<IDxcBlob> pixelShaderBinary{};
	CheckHR(pixelShaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pixelShaderBinary), name.GetAddressOf()));

	mAmplificationShader	= { amplificationShaderBinary->GetBufferPointer()		, amplificationShaderBinary->GetBufferSize() };
	mMeshShader				= { meshShaderBinary->GetBufferPointer()				, meshShaderBinary->GetBufferSize() };
	mPixelShader			= { pixelShaderBinary->GetBufferPointer()				, pixelShaderBinary->GetBufferSize() };



	GrassRenderer::CreateRootSignature(device);
	GrassRenderer::CreatePipelineState(device);
}

void GrassRenderer::SetMaterial(UINT materialIndex) {
	mMaterialIndex = materialIndex;
}

void GrassRenderer::Render(ComPtr<ID3D12GraphicsCommandList6> commandList, DefaultBufferGPUIterator cameraBuffer, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS material) {
	commandList->SetGraphicsRootSignature(mRootSignature.Get());
	commandList->SetPipelineState(mPipelineState.Get());


	// 0. Camera 
	// 1. Time
	// 2. grass position
	// 3. material Index 
	// 4. material 
	// 5. textures 
	commandList->SetGraphicsRootConstantBufferView(0, *cameraBuffer);
	commandList->SetGraphicsRoot32BitConstant(1, Time.GetTimeSinceSceneStarted<UINT, std::chrono::milliseconds>(), 0);
	commandList->SetGraphicsRootShaderResourceView(2, *mGrassPosition.GPUBegin());
	commandList->SetGraphicsRoot32BitConstants(3, 1, &mMaterialIndex, 0);
	commandList->SetGraphicsRootShaderResourceView(4, material);
	commandList->SetGraphicsRootDescriptorTable(5, tex);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED);

	// Dispatch mesh 
	commandList->DispatchMesh(2500, 1, 1);
}



void GrassRenderer::CreatePipelineState(ComPtr<ID3D12Device10> device) {
	struct alignas(8) Subobject_RootSignature {
		D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE;
		ID3D12RootSignature* pRootSignature = nullptr;
	};

	struct alignas(8) Subobject_Shader {
		D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type;
		D3D12_SHADER_BYTECODE Shader;
	};

	struct alignas(8) Subobject_Blend {
		D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND;
		D3D12_BLEND_DESC Desc;
	};

	struct alignas(8) Subobject_Rasterizer {
		D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER;
		D3D12_RASTERIZER_DESC Desc;
	};

	struct alignas(8) Subobject_DepthStencil {
		D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL;
		D3D12_DEPTH_STENCIL_DESC Desc;
	};

	struct alignas(8) Subobject_Topology {
		D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE Topology;
	};

	struct alignas(8) Subobject_RTVFormats {
		D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS;
		D3D12_RT_FORMAT_ARRAY Formats;
	};

	struct alignas(8) Subobject_DSVFormat {
		D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT;
		DXGI_FORMAT Format;
	};

	// 전체 스트림 구조체
	struct alignas(8) MeshPipelineStream {
		Subobject_RootSignature Root;
		Subobject_Shader AS;
		Subobject_Shader MS;
		Subobject_Shader PS;
		Subobject_Blend Blend;
		Subobject_Rasterizer Rasterizer;
		Subobject_DepthStencil DepthStencil;
		Subobject_Topology Topology;
		Subobject_RTVFormats RTVs;
		Subobject_DSVFormat DSV;
	};


	MeshPipelineStream stream = {};

	stream.Root.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE;
	stream.Root.pRootSignature = mRootSignature.Get();

	stream.AS.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS;
	stream.AS.Shader = mAmplificationShader;

	stream.MS.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS;
	stream.MS.Shader = mMeshShader;

	stream.PS.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS;
	stream.PS.Shader = mPixelShader;

	stream.Blend.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND;
	stream.Blend.Desc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

	stream.Blend.Desc.AlphaToCoverageEnable = FALSE;
	stream.Blend.Desc.IndependentBlendEnable = TRUE;

	stream.Blend.Desc.RenderTarget[0].BlendEnable = FALSE;
	stream.Blend.Desc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	stream.Blend.Desc.RenderTarget[1].BlendEnable = FALSE;
	stream.Blend.Desc.RenderTarget[1].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	stream.Blend.Desc.RenderTarget[2].BlendEnable = FALSE;
	stream.Blend.Desc.RenderTarget[2].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	stream.Rasterizer.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER;
	stream.Rasterizer.Desc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	stream.Rasterizer.Desc.CullMode = D3D12_CULL_MODE_NONE;

	stream.DepthStencil.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL;
	stream.DepthStencil.Desc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	stream.DepthStencil.Desc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL; 

	stream.Topology.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY;
	stream.Topology.Topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	stream.RTVs.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS;
	stream.RTVs.Formats.NumRenderTargets = Config::GBUFFER_COUNT<UINT>;
	for (UINT i = 0; i < stream.RTVs.Formats.NumRenderTargets; ++i)
		stream.RTVs.Formats.RTFormats[i] = DXGI_FORMAT_R32G32B32A32_FLOAT;

	stream.DSV.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT;
	stream.DSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	D3D12_PIPELINE_STATE_STREAM_DESC streamDesc{};
	streamDesc.pPipelineStateSubobjectStream = &stream;
	streamDesc.SizeInBytes = sizeof(MeshPipelineStream);

	ComPtr<ID3D12PipelineState> pso;
	CheckHR(device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&pso)));
	
	mPipelineState = pso;
}

void GrassRenderer::CreateRootSignature(ComPtr<ID3D12Device10> device) {
	static std::array<CD3DX12_STATIC_SAMPLER_DESC, 7> staticSamplers{};

	staticSamplers[0] = { 0, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP };
	staticSamplers[1] = { 1, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP };
	staticSamplers[2] = { 2, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP };
	staticSamplers[3] = { 3, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP };
	staticSamplers[4] = { 4, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0.0f, 16 };
	staticSamplers[5] = { 5, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0.0f, 16 };
	staticSamplers[6] = { 6, D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER, 0.0f, 16 };


	D3D12_DESCRIPTOR_RANGE descriptorRange{};
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange.NumDescriptors = Config::MAX_TEXTURE_COUNT<UINT>;
	descriptorRange.BaseShaderRegister = 2;
	descriptorRange.RegisterSpace = 0;
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// 1. Camera 
	// 2. Time
	// 3. grass position
	// 4. material Index 
	// 5. material 
	// 6. textures 
	D3D12_ROOT_PARAMETER rootParameters[6]{};

	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].Descriptor.ShaderRegister = 0;
	rootParameters[0].Descriptor.RegisterSpace = 0;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameters[1].Constants.ShaderRegister = 1;
	rootParameters[1].Constants.RegisterSpace = 0;
	rootParameters[1].Constants.Num32BitValues = 1;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParameters[2].Descriptor.ShaderRegister = 0;
	rootParameters[2].Descriptor.RegisterSpace = 0;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameters[3].Constants.ShaderRegister = 2;
	rootParameters[3].Constants.RegisterSpace = 0;
	rootParameters[3].Constants.Num32BitValues = 1;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParameters[4].Descriptor.ShaderRegister = 1;
	rootParameters[4].Descriptor.RegisterSpace = 0;
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[5].DescriptorTable.pDescriptorRanges = &descriptorRange;
	rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};

	rootSignatureDesc.NumParameters = _countof(rootParameters);
	rootSignatureDesc.pParameters = rootParameters;
	rootSignatureDesc.NumStaticSamplers = static_cast<UINT>(staticSamplers.size());
	rootSignatureDesc.pStaticSamplers = staticSamplers.data();
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> rootSignatureBlob{};
	ComPtr<ID3DBlob> errorBlob{};

	if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignatureBlob, errorBlob.GetAddressOf()))) {
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		Crash(false);
	}

	CheckHR(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));
}




namespace V2 {
	void GrassTree::BuildTreeFromFile(const std::string& filename) {
		mTerrainCollider.LoadFromFile("Resources/Binarys/Terrain/NTerrain.bin");

		std::ifstream ifs(filename, std::ios::binary);
		if (!ifs) {
			CrashExp(false, "Failed to open file for reading");
		}

		size_t pointCount;
		ifs.read(reinterpret_cast<char*>(&pointCount), sizeof(size_t));

		std::vector<SimpleMath::Vector2> flatPoints(pointCount);
		ifs.read(reinterpret_cast<char*>(flatPoints.data()), pointCount * sizeof(SimpleMath::Vector2));

		mPoints.resize(pointCount);
		for (size_t i = 0; i < pointCount; ++i) {
			float y = mTerrainCollider.GetHeight(flatPoints[i].x, flatPoints[i].y);
			mPoints[i] = SimpleMath::Vector3(flatPoints[i].x, y, flatPoints[i].y);
		}

		mNodes.clear();
		mNodes.reserve(pointCount);
		Build(0, pointCount);
	}

	size_t GrassTree::GetSize() const {
		return mPoints.size();
	}

	void GrassTree::SaveToFile(const std::string& filename) const {
		std::ofstream ofs(filename, std::ios::binary);
		if (!ofs) {
			CrashExp(false, "Failed to open file for writing");
		}

		size_t pointCount = mPoints.size();
		ofs.write(reinterpret_cast<const char*>(&pointCount), sizeof(size_t));
		ofs.write(reinterpret_cast<const char*>(mPoints.data()), pointCount * sizeof(SimpleMath::Vector3));

		size_t nodeCount = mNodes.size();
		ofs.write(reinterpret_cast<const char*>(&nodeCount), sizeof(size_t));
		ofs.write(reinterpret_cast<const char*>(mNodes.data()), nodeCount * sizeof(Node));
	}

	void GrassTree::LoadFromFile(const std::string& filename) {
		std::ifstream ifs(filename, std::ios::binary);
		if (!ifs) throw std::runtime_error("Failed to open file");

		size_t pointCount;
		ifs.read(reinterpret_cast<char*>(&pointCount), sizeof(size_t));

		mPoints.resize(pointCount);
		ifs.read(reinterpret_cast<char*>(mPoints.data()), pointCount * sizeof(SimpleMath::Vector3));

		size_t nodeCount;
		ifs.read(reinterpret_cast<char*>(&nodeCount), sizeof(size_t));
		mNodes.resize(nodeCount);
		ifs.read(reinterpret_cast<char*>(mNodes.data()), nodeCount * sizeof(Node));
	}

	int GrassTree::ChooseAxis(size_t begin, size_t end) {
		size_t count = end - begin;
		if (count == 0) return 0;

		float meanX = 0, meanZ = 0;
		for (size_t i = begin; i < end; ++i) {
			meanX += mPoints[i].x;
			meanZ += mPoints[i].z;
		}
		meanX /= count;
		meanZ /= count;

		float varX = 0, varZ = 0;
		for (size_t i = begin; i < end; ++i) {
			float dx = mPoints[i].x - meanX;
			float dz = mPoints[i].z - meanZ;
			varX += dx * dx;
			varZ += dz * dz;
		}

		return (varX >= varZ) ? 0 : 1;
	}

	int GrassTree::Build(size_t begin, size_t end) {
		if (begin >= end) return -1;

		size_t count = end - begin;
		if (count <= LEAF_SIZE) {
			Node node;
			node.axis = -1;
			node.leaf.begin = begin;
			node.leaf.end = end;
			node.point = SimpleMath::Vector2(mPoints[begin].x, mPoints[begin].z);

			int idx = static_cast<int>(mNodes.size());
			mNodes.emplace_back(node);
			return idx;
		}

		int axis = ChooseAxis(begin, end);
		size_t medianIdx = (begin + end) / 2;
		auto predicate = [axis](const SimpleMath::Vector3& a, const SimpleMath::Vector3& b) {
			return (axis == 0) ? (a.x < b.x) : (a.z < b.z);
			};
		std::nth_element(mPoints.begin() + begin, mPoints.begin() + medianIdx, mPoints.begin() + end, predicate);

		Node node;
		node.point = SimpleMath::Vector2(mPoints[medianIdx].x, mPoints[medianIdx].z);
		node.axis = axis;

		int currentIdx = static_cast<int>(mNodes.size());
		mNodes.emplace_back(node);

		int left = Build(begin, medianIdx);
		int right = Build(medianIdx + 1, end);

		mNodes[currentIdx].internal.left = left;
		mNodes[currentIdx].internal.right = right;

		return currentIdx;
	}

	void GrassTree::QueryRange(const SimpleMath::Vector2& center, float radius, std::vector<SimpleMath::Vector3>& result) const {
		result.clear();
		QueryRecursive(0, center, radius, result);
	}

	void GrassTree::QueryRecursive(int nodeIdx, const SimpleMath::Vector2& center, float radius, std::vector<SimpleMath::Vector3>& result) const {
		if (nodeIdx == -1) return;

		const Node& node = mNodes[nodeIdx];

		if (node.axis == -1) {
			for (size_t i = node.leaf.begin; i < node.leaf.end; ++i) {
				float dx = mPoints[i].x - center.x;
				float dz = mPoints[i].z - center.y;
				if (dx * dx + dz * dz <= radius * radius) {
					result.emplace_back(mPoints[i]);
				}
			}
			return;
		}

		float dx = node.point.x - center.x;
		float dz = node.point.y - center.y;
		if (dx * dx + dz * dz <= radius * radius) {
			result.emplace_back(SimpleMath::Vector3(node.point.x, mTerrainCollider.GetHeight(node.point.x, node.point.y), node.point.y));
		}

		float splitCoord = (node.axis == 0) ? node.point.x : node.point.y;
		float centerCoord = (node.axis == 0) ? center.x : center.y;

		if (centerCoord - radius <= splitCoord) {
			QueryRecursive(node.internal.left, center, radius, result);
		}
		if (centerCoord + radius >= splitCoord) {
			QueryRecursive(node.internal.right, center, radius, result);
		}
	}

	GrassRenderer::GrassRenderer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, DefaultBufferCPUIterator cameraBufferLocation) {
		const std::filesystem::path grassPath{ "Resources/Binarys/Terrain/GrassXZ.bin" };
		const std::filesystem::path grassTreePath{ "Resources/Binarys/Terrain/grass_tree.bin" };


		if (std::filesystem::exists(grassPath)) {
			mGrassTree.BuildTreeFromFile(grassPath.string());
			mGrassTree.SaveToFile(grassTreePath.string()); 
		
			//std::filesystem::rename(grassPath, "Resources/Binarys/Terrain/grass_old.bin");
		}
		else {
			if (not std::filesystem::exists(grassTreePath)) {
				CrashExp(false, "Grass tree file does not exist. Please build the grass tree first.");
			}
			mGrassTree.LoadFromFile(grassTreePath.string());
		}

		mGrassInstance = DefaultBuffer(device, sizeof(SimpleMath::Vector3), mGrassTree.GetSize());
		mShader = std::make_shared<GrassShader>();
		mShader->CreateShader(device); 

		mCameraBufferLocation = cameraBufferLocation;
	}

	void GrassRenderer::SetMaterial(UINT materialIndex) {
		mMaterialIndex = materialIndex;
	}

	void GrassRenderer::Render(ComPtr<ID3D12GraphicsCommandList> commandList, DefaultBufferGPUIterator cameraBuffer, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS material) {

		CameraConstants cameraConstants{};
		std::memcpy(&cameraConstants, *mCameraBufferLocation, sizeof(CameraConstants));

		mGrass.clear(); 
		mGrassTree.QueryRange(SimpleMath::Vector2(cameraConstants.cameraPosition.x, cameraConstants.cameraPosition.z), 100.0f, mGrass);

		std::memcpy(*mGrassInstance.CPUBegin(), mGrass.data(), mGrass.size() * sizeof(SimpleMath::Vector3));
		mGrassInstance.Upload(commandList,mGrassInstance.CPUBegin(), mGrassInstance.CPUBegin() + mGrass.size()); 

		mShader->SetGPassShader(commandList);

		// 0. Camera 
		// 1. Time
		// 2. grass position
		// 3. material Index 
		// 4. material 
		// 5. textures 
		commandList->SetGraphicsRootConstantBufferView(0, *cameraBuffer);
		commandList->SetGraphicsRoot32BitConstant(1, Time.GetTimeSinceSceneStarted<UINT, std::chrono::milliseconds>(), 0);
		commandList->SetGraphicsRoot32BitConstants(2, 1, &mMaterialIndex, 0);
		commandList->SetGraphicsRootShaderResourceView(3, material);
		commandList->SetGraphicsRootDescriptorTable(4, tex);

		D3D12_VERTEX_BUFFER_VIEW grassInstanceView{};
		grassInstanceView.BufferLocation = *mGrassInstance.GPUBegin();
		grassInstanceView.SizeInBytes = static_cast<UINT>(mGrassTree.GetSize() * sizeof(SimpleMath::Vector3));
		grassInstanceView.StrideInBytes = sizeof(SimpleMath::Vector3);

		commandList->IASetVertexBuffers(0, 1, &grassInstanceView);
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

		commandList->DrawInstanced(static_cast<UINT>(mGrass.size()), 1, 0, 0); 
	}

}