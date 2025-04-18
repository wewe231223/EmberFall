#include "pch.h"
#include "GrassRenderer.h"
#include <random>
#include <fstream>
#include <filesystem>
#include "../Utility/Exceptions.h"

GrassRenderer::GrassRenderer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, DefaultBufferGPUIterator terrainHeader, DefaultBufferGPUIterator terrainData) {
	mTerrainHeader = terrainHeader;
	mTerrainData = mTerrainData; 

	std::vector<GrassPoint> points{};
	points.reserve(GRASS_INSTANCE_COUNT<size_t>);

	std::ifstream file{ "Resources/Binarys/Terrain/grass.bin", std::ios::binary };
	file.read(reinterpret_cast<char*>(points.data()), sizeof(GrassPoint) * points.size()); 

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
		L"-E", L"main",             // Entry point
		L"-Zi",                     // 디버그 정보 포함
		L"-Qembed_debug",			// cso 에 디버그 정보 포함 
		L"-Od",                     // 최적화 비활성화 (디버깅용)
	};

	LPCWSTR amplificationShaderArgs[] = {
		shaderPath.c_str(),         // 소스 파일 경로
		L"-T", L"as_6_5",           // Target: Amplification Shader 6.5
		L"-E", L"main",             // Entry point
		L"-Zi",                     // 디버그 정보 포함
		L"-Qembed_debug",			// cso 에 디버그 정보 포함 
		L"-Od",                     // 최적화 비활성화 (디버깅용)
	};

	LPCWSTR pixelShaderArgs[] = {
		shaderPath.c_str(),         // 소스 파일 경로
		L"-T", L"ps_6_0",           // Target: Pixel Shader 6.5
		L"-E", L"main",             // Entry point
		L"-Zi",                     // 디버그 정보 포함
		L"-Qembed_debug",			// cso 에 디버그 정보 포함 
		L"-Od",                     // 최적화 비활성화 (디버깅용)
	};
#else 
	LPCWSTR meshShaderArgs[] = {
		shaderPath.c_str(),         // 소스 파일 경로 
		L"-T", L"ms_6_5",			// Target: Mesh Shader 6.5
		L"-E", L"main",
		L"-O3",						// <- 최적화 최대화
	};

	LPCWSTR amplificationShaderArgs[] = {
		shaderPath.c_str(),         // 소스 파일 경로
		L"-T", L"as_6_5",           // Target: Amplification Shader 6.5
		L"-E", L"main",             // Entry point
		L"-O3",						// <- 최적화 최대화
	};

	LPCWSTR pixelShaderArgs[] = {
		shaderPath.c_str(),         // 소스 파일 경로
		L"-T", L"ps_6_0",           // Target: Pixel Shader 6.5
		L"-E", L"main",             // Entry point
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
	descriptorRange.BaseShaderRegister = 1;
	descriptorRange.RegisterSpace = 0;
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// 1. Camera 
	// 2. terrain header
	// 3. terrain data 
	// 4. material 
	// 5. textures 
	D3D12_ROOT_PARAMETER rootParameters[5]{};

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

	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameters[3].Constants.ShaderRegister = 2;
	rootParameters[3].Constants.RegisterSpace = 0;
	rootParameters[3].Constants.Num32BitValues = 1;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[4].DescriptorTable.pDescriptorRanges = &descriptorRange;
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};

	rootSignatureDesc.NumParameters = _countof(rootParameters);
	rootSignatureDesc.pParameters = rootParameters;
	rootSignatureDesc.NumStaticSamplers = staticSamplers.size();
	rootSignatureDesc.pStaticSamplers = staticSamplers.data();
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> rootSignatureBlob{};
	ComPtr<ID3DBlob> errorBlob{};

	CheckHR(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignatureBlob, errorBlob.GetAddressOf()));



}
