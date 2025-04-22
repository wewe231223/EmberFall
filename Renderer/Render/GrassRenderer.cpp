#include "pch.h"
#include "GrassRenderer.h"
#include <random>
#include <fstream>
#include <filesystem>
#include "../Utility/Exceptions.h"

GrassRenderer::GrassRenderer(ComPtr<ID3D12Device10> device, ComPtr<ID3D12GraphicsCommandList> commandList, DefaultBufferGPUIterator terrainHeader, DefaultBufferGPUIterator terrainData) {
	mTerrainHeader = terrainHeader;
	mTerrainData = terrainData; 

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

// 1. Camera 
// 2. terrain header
// 3. terrain data
// 4. grass position
// 5. material Index 
// 6. material 
// 7. textures 
void GrassRenderer::Render(ComPtr<ID3D12GraphicsCommandList6> commandList, DefaultBufferGPUIterator cameraBuffer, D3D12_GPU_DESCRIPTOR_HANDLE tex, D3D12_GPU_VIRTUAL_ADDRESS material) {
	commandList->SetGraphicsRootSignature(mRootSignature.Get());
	commandList->SetPipelineState(mPipelineState.Get());


	// 1. Camera 
	// 2. terrain header
	// 3. terrain data
	// 4. grass position
	// 5. material Index 
	// 6. material 
	// 7. textures 
	commandList->SetGraphicsRootConstantBufferView(0, *cameraBuffer);
	commandList->SetGraphicsRootConstantBufferView(1, *mTerrainHeader);
	commandList->SetGraphicsRootShaderResourceView(2, *mTerrainData);
	commandList->SetGraphicsRootShaderResourceView(3, *mGrassPosition.GPUBegin());
	commandList->SetGraphicsRoot32BitConstants(4, 1, &mMaterialIndex, 0);
	commandList->SetGraphicsRootShaderResourceView(5, material);
	commandList->SetGraphicsRootDescriptorTable(6, tex);

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

	stream.Rasterizer.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER;
	stream.Rasterizer.Desc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
//	stream.Rasterizer.Desc.CullMode = D3D12_CULL_MODE_NONE;

	stream.DepthStencil.Type = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL;
	stream.DepthStencil.Desc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

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
	descriptorRange.BaseShaderRegister = 3;
	descriptorRange.RegisterSpace = 0;
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// 1. Camera 
	// 2. terrain header
	// 3. terrain data
	// 4. grass position
	// 5. material Index 
	// 6. material 
	// 7. textures 
	D3D12_ROOT_PARAMETER rootParameters[7]{};

	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].Descriptor.ShaderRegister = 0;
	rootParameters[0].Descriptor.RegisterSpace = 0;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].Descriptor.ShaderRegister = 1;
	rootParameters[1].Descriptor.RegisterSpace = 0;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParameters[2].Descriptor.ShaderRegister = 0;
	rootParameters[2].Descriptor.RegisterSpace = 0;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParameters[3].Descriptor.ShaderRegister = 1;
	rootParameters[3].Descriptor.RegisterSpace = 0;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameters[4].Constants.ShaderRegister = 2;
	rootParameters[4].Constants.RegisterSpace = 0;
	rootParameters[4].Constants.Num32BitValues = 1;
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParameters[5].Descriptor.ShaderRegister = 2;
	rootParameters[5].Descriptor.RegisterSpace = 0;
	rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[6].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[6].DescriptorTable.pDescriptorRanges = &descriptorRange;
	rootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


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
