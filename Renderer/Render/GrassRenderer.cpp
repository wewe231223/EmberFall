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

	const std::filesystem::path shaderPath{ L"Resources/Shaders/GrassShader.hlsl" };

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

	ComPtr<IDxcResult> meshShaderResult{};
	CheckHR(dxcCompiler->Compile(&sourceBuffer, meshShaderArgs, _countof(meshShaderArgs), includeHandler.Get(), IID_PPV_ARGS(&meshShaderResult)));

	CheckHR(meshShaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));
	if (errors and errors->GetStringLength() > 0) {
		OutputDebugStringA((char*)errors->GetStringPointer());
		Crash(false); 
	} 

	ComPtr<IDxcResult> amplificationShaderResult{};
	CheckHR(dxcCompiler->Compile(&sourceBuffer, amplificationShaderArgs, _countof(amplificationShaderArgs), includeHandler.Get(), IID_PPV_ARGS(&amplificationShaderResult)));

	CheckHR(amplificationShaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));
	if (errors and errors->GetStringLength() > 0) {
		OutputDebugStringA((char*)errors->GetStringPointer());
		Crash(false);
	}

	ComPtr<IDxcResult> pixelShaderResult{};
	CheckHR(dxcCompiler->Compile(&sourceBuffer, pixelShaderArgs, _countof(pixelShaderArgs), includeHandler.Get(), IID_PPV_ARGS(&pixelShaderResult)));

	CheckHR(pixelShaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));
	if (errors and errors->GetStringLength() > 0) {
		OutputDebugStringA((char*)errors->GetStringPointer());
		Crash(false);
	}

	ComPtr<IDxcBlob> meshShaderBinary{};
	CheckHR(meshShaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&meshShaderBinary), nullptr));

	ComPtr<IDxcBlob> amplificationShaderBinary{};
	CheckHR(amplificationShaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&amplificationShaderBinary), nullptr));

	ComPtr<IDxcBlob> pixelShaderBinary{};
	CheckHR(pixelShaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pixelShaderBinary), nullptr));


}
