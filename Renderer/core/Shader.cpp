#include "pch.h"
#include "Shader.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <unordered_map>
#include <windows.h>
#include <d3dcompiler.h>
#include <regex>
#include "../Config/Config.h"
#include "../Utility/Exceptions.h"
#include "../Utility/Crash.h"
#include "../EditorInterface/Console/Console.h"
#include "../Utility/Defines.h"

#undef max 

//------------------------------------------------------------------------------------------------[ ShaderManager ]------------------------------------------------------------------------------------------------

ShaderFileManager::ShaderFileManager() {
	std::ifstream metadataFile{ SHADER_METADATA_PATH };
	CrashExp(metadataFile.is_open(), "File not found");

	std::string line{};
	std::regex shaderRegex{ R"((\w+\.hlsl)\s*\{\s*((?:\w+\s+\w+\s+\w+\b\s*)+)\}\s*(?:<\s*([\w\.]+(?:\s*,\s*[\w\.]+)*)\s*>)?)" };

	while (std::getline(metadataFile, line)) {
		std::smatch match;
		if (!std::regex_match(line, match, shaderRegex)) {
			Console.Log("Invalid shader metadata format : {}", LogType::Error, line);
			continue;
		} 

		std::string	source{ match[1] };
		std::istringstream shaderInfoStream{ match[2] };
		std::vector<std::tuple<std::string, std::string, std::string>> shaderInfo{};
		std::string type{};
		std::string	model{};
		std::string entry{};
		
		while (shaderInfoStream >> type >> model >> entry) {
			shaderInfo.emplace_back(type, model, entry);
		}

		std::vector<std::filesystem::path> includes{};
		if (match[3].matched) {
			std::istringstream shaderIncludeStream{ match[3] };
			std::filesystem::path includeFile{};

			while (shaderIncludeStream >> includeFile) {
				includeFile = "Shader/Sources/" / includeFile;
				includes.emplace_back(includeFile);
			}
		}

		ShaderFileManager::ProcessShader( "Shader/Sources/" + source, shaderInfo, includes);
	}
}

ComPtr<ID3D12Blob>& ShaderFileManager::GetShaderBlob(const std::string& name, ShaderType type) {
	return mShaderBlobs[name][type];
}

std::array<ComPtr<ID3D12Blob>, ShaderType::END>& ShaderFileManager::GetShaderBlobs(const std::string& name) {
	return mShaderBlobs[name];
}

void ShaderFileManager::ProcessShader(const std::filesystem::path& source, const std::vector<std::tuple<std::string, std::string, std::string>>& shaderInfo, const std::vector<std::filesystem::path>& includes) {
	static std::filesystem::path binPath{ "Shader/Binarys/" };

	if (not std::filesystem::exists(source)) {
		Console.Log("Shader source file not found : {}", LogType::Error, source.string());
		return; 
	}

	auto lastSourceWriteTime{ std::filesystem::last_write_time(source) };
	for (const auto& include : includes) {
		if (std::filesystem::exists(include) and std::filesystem::last_write_time(include) > lastSourceWriteTime) {
			lastSourceWriteTime = std::filesystem::last_write_time(include);
		}
	}

	std::filesystem::path firstBinaryPath;
	for (const auto& [type, model, entry] : shaderInfo) {
		std::string binaryFileName{ source.stem().string() + "_" + type + ".bin" };
		std::filesystem::path binaryPath{ binPath / binaryFileName };

		if (std::filesystem::exists(binaryPath)) {
			firstBinaryPath = binaryPath;
			break;
		}
	}



	if (firstBinaryPath.empty() or std::filesystem::last_write_time(firstBinaryPath) <= lastSourceWriteTime) {
		// 재 컴파일 이후 바이너리 파일에 쓰고 맵에 등록까지 완료 
		for (const auto& [type, model, entry] : shaderInfo) {
			ShaderFileManager::ReCompile(source, type, model, entry);
		} 
	}
	else {
		for (const auto& [type, model, entry] : shaderInfo) {
			std::string binaryFileName{ source.stem().string() + "_" + type + ".bin" };
			std::filesystem::path binaryPath{ binPath / binaryFileName };

			ComPtr<ID3D12Blob> shaderBlob{};
			ShaderFileManager::Load(shaderBlob, binaryPath);

			ShaderType eType{};

			if (type == "vs") {
				eType = ShaderType::VertexShader;
			}
			else if (type == "ps") {
				eType = ShaderType::PixelShader;
			}
			else if (type == "gs") {
				eType = ShaderType::GeometryShader;
			}
			else if (type == "hs") {
				eType = ShaderType::HullShader;
			}
			else if (type == "ds") {
				eType = ShaderType::DomainShader;
			}
			else {
				Crash(("Invalid Shaer Type : " + type).c_str());
			}

			mShaderBlobs[source.stem().string()][eType] = shaderBlob;
		}
	}

}

void ShaderFileManager::ReCompile(const std::filesystem::path& source, const std::string& type, const std::string& model, const std::string& entry) {
	Console.Log("Compiling shader : {}", LogType::Info, source.string());

	ComPtr<ID3D12Blob> shaderBlob{};
	ComPtr<ID3D12Blob> errorBlob{};
	
	auto hr = ::D3DCompileFromFile(source.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry.c_str(), model.c_str(), 0, 0, &shaderBlob, &errorBlob);

	if (FAILED(hr)) {
		OutputDebugStringA(reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
	}


	std::string binaryFileName{ source.stem().string() + "_" + type + ".bin" };
	std::filesystem::path binaryPath{ "Shader/Binarys/" + binaryFileName };

	if (SUCCEEDED(hr)) {
		std::ofstream binaryFile{ binaryPath, std::ios::binary };
		CrashExp(binaryFile.is_open(), "Failed to create binary file");

		binaryFile.write(static_cast<char*>(shaderBlob->GetBufferPointer()), shaderBlob->GetBufferSize());
	}
	else {
		Console.Log("Failed to compile shader : {}\nLoad previous version!", LogType::Error, reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
		CrashExp(std::filesystem::exists(binaryPath), "Binary file not found");
		
		if (shaderBlob) {
			shaderBlob->Release();
		}

		ShaderFileManager::Load(shaderBlob, binaryPath);
	}
	
	ShaderType eType{};

	if (type == "vs") {
		eType = ShaderType::VertexShader;
	}
	else if (type == "ps") {
		eType = ShaderType::PixelShader;
	}
	else if (type == "gs") {
		eType = ShaderType::GeometryShader;
	}
	else if (type == "hs") {
		eType = ShaderType::HullShader;
	}
	else if (type == "ds") {
		eType = ShaderType::DomainShader;
	}
	else {
		Crash(("Invalid Shaer Type : " + type).c_str());
	}

	mShaderBlobs[source.stem().string()][eType] = shaderBlob;
}

void ShaderFileManager::Load(ComPtr<ID3D12Blob>& blob, const std::filesystem::path& path) {
	Console.Log("Loading shader : {}", LogType::Info, path.string());

	auto size = std::filesystem::file_size(path);

	::D3DCreateBlob(size, blob.GetAddressOf());

	std::ifstream file{ path, std::ios::binary };
	file.read(static_cast<char*>(blob->GetBufferPointer()), size);
}

ShaderFileManager gShaderManager{};

//------------------------------------------------------------------------------------------------[ ShaderManager ]------------------------------------------------------------------------------------------------

GraphicsShaderBase::GraphicsShaderBase() {
	
}

GraphicsShaderBase::~GraphicsShaderBase() {

}

GraphicsShaderBase::InputLayout GraphicsShaderBase::CreateInputLayout() {
	InputLayout result{};
	result.ElementCount = 0;

	return result;
}

D3D12_RASTERIZER_DESC GraphicsShaderBase::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	::ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));

	//	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}

D3D12_BLEND_DESC GraphicsShaderBase::CreateBlendState() {
	D3D12_BLEND_DESC blendStateDesc;
	::ZeroMemory(&blendStateDesc, sizeof(D3D12_BLEND_DESC));

	blendStateDesc.AlphaToCoverageEnable = FALSE;
	blendStateDesc.IndependentBlendEnable = FALSE;
	blendStateDesc.RenderTarget[0].BlendEnable = FALSE;
	blendStateDesc.RenderTarget[0].LogicOpEnable = FALSE;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	blendStateDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return(blendStateDesc);
}

D3D12_DEPTH_STENCIL_DESC GraphicsShaderBase::CreateDepthStencilState() {
	D3D12_DEPTH_STENCIL_DESC depthStencilState;
	::ZeroMemory(&depthStencilState, sizeof(D3D12_DEPTH_STENCIL_DESC));

	depthStencilState.DepthEnable = TRUE;
	depthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilState.DepthFunc = Config::DEFAULT_REVERSE_Z ? D3D12_COMPARISON_FUNC_GREATER_EQUAL : D3D12_COMPARISON_FUNC_LESS;
	depthStencilState.StencilEnable = FALSE;
	depthStencilState.StencilReadMask = 0x00;
	depthStencilState.StencilWriteMask = 0x00;
	depthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	depthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return depthStencilState;
}

GraphicsShaderBase::RootParameters GraphicsShaderBase::CreateRootParameters() {
	return RootParameters();
}

D3D12_ROOT_SIGNATURE_FLAGS GraphicsShaderBase::CreateRootSignatureFlag() {
	return D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE GraphicsShaderBase::CreatePrimitiveTopologyType() {
	return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
}

UINT GraphicsShaderBase::CreateNumOfRenderTarget() {
	return 1;
}

void GraphicsShaderBase::CreateRTVFormat(const std::span<DXGI_FORMAT>& formats) {
	formats[0] = Config::RENDER_TARGET_FORMAT;
}

DXGI_FORMAT GraphicsShaderBase::CreateDSVFormat() {
	return DXGI_FORMAT_D24_UNORM_S8_UINT;
}

GraphicsShaderBase::StreamOutputState GraphicsShaderBase::CreateStreamOutputState() {
	return StreamOutputState();
}

D3D12_SHADER_BYTECODE GraphicsShaderBase::CreateVertexShader() {
	return D3D12_SHADER_BYTECODE();
}

D3D12_SHADER_BYTECODE GraphicsShaderBase::CreatePixelShader() {
	return D3D12_SHADER_BYTECODE();
}

D3D12_SHADER_BYTECODE GraphicsShaderBase::CreateGeometryShader() {
	return D3D12_SHADER_BYTECODE();
}

D3D12_SHADER_BYTECODE GraphicsShaderBase::CreateHullShader() {
	return D3D12_SHADER_BYTECODE();
}

D3D12_SHADER_BYTECODE GraphicsShaderBase::CreateDomainShader() {
	return D3D12_SHADER_BYTECODE();
}

void GraphicsShaderBase::CreateShader(ComPtr<ID3D12Device> device) {
	static std::array<CD3DX12_STATIC_SAMPLER_DESC, 7> staticSamplers{};

	staticSamplers[0] = { 0, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP };
	staticSamplers[1] = { 1, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP };
	staticSamplers[2] = { 2, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP };
	staticSamplers[3] = { 3, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP };
	staticSamplers[4] = { 4, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0.0f, 16 };
	staticSamplers[5] = { 5, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0.0f, 16 };
	staticSamplers[6] = { 6, D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER, 0.0f, 16 };
	
	D3D12_ROOT_SIGNATURE_DESC rootsignatureDesc{};
	auto rootParameters = CreateRootParameters();

	rootsignatureDesc.NumParameters = rootParameters.ParameterCount;
	rootsignatureDesc.pParameters = rootParameters.Parameters.data();
	rootsignatureDesc.NumStaticSamplers = static_cast<UINT>(staticSamplers.size());
	rootsignatureDesc.pStaticSamplers = staticSamplers.data();
	rootsignatureDesc.Flags = CreateRootSignatureFlag();

	ComPtr<ID3D12Blob> signatureBlob{};
	ComPtr<ID3D12Blob> errorBlob{};

	auto hr = ::D3D12SerializeRootSignature(&rootsignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signatureBlob.GetAddressOf(), errorBlob.GetAddressOf());
	// CrashExp(SUCCEEDED(hr), reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));

	if (FAILED(hr)) {
		OutputDebugStringA(reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
		Crash(false);
	}

	CheckHR(device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	auto inputLayout = CreateInputLayout();

	psoDesc.InputLayout = { inputLayout.InputElements.data(), inputLayout.ElementCount };
	psoDesc.pRootSignature = mRootSignature.Get();
	psoDesc.VS = CreateVertexShader();
	psoDesc.PS = CreatePixelShader();
	psoDesc.GS = CreateGeometryShader();
	psoDesc.HS = CreateHullShader();
	psoDesc.DS = CreateDomainShader();
	psoDesc.RasterizerState = CreateRasterizerState();
	psoDesc.BlendState = CreateBlendState();
	psoDesc.DepthStencilState = CreateDepthStencilState();
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = CreatePrimitiveTopologyType();
	psoDesc.NumRenderTargets = CreateNumOfRenderTarget();
	CreateRTVFormat(std::span<DXGI_FORMAT>(psoDesc.RTVFormats, 8));
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.DSVFormat = CreateDSVFormat();
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	
	auto soState = CreateStreamOutputState();
	psoDesc.StreamOutput = { soState.SODeclaration.data(), soState.SODeclarationCount, soState.BufferStrides.data(), soState.BufferCount, soState.RasterizedStream };

	CheckHR(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPipelineState)));

	psoDesc.NumRenderTargets = 3;
	std::memset(psoDesc.RTVFormats, DXGI_FORMAT_UNKNOWN, sizeof(DXGI_FORMAT) * 8);
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.RTVFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	CheckHR(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mShadowPipelineState)));
}

UINT GraphicsShaderBase::GetShaderID() const {
	return std::numeric_limits<UINT>::max();
}

const std::bitset<8>& GraphicsShaderBase::GetAttribute() const {
	return mAttribute;
}

void GraphicsShaderBase::SetShadowPassShader(ComPtr<ID3D12GraphicsCommandList> commandList) {
	commandList->SetPipelineState(mShadowPipelineState.Get());
	commandList->SetGraphicsRootSignature(mRootSignature.Get());
}

void GraphicsShaderBase::SetGPassShader(ComPtr<ID3D12GraphicsCommandList> commandList) {
	commandList->SetPipelineState(mPipelineState.Get());
	commandList->SetGraphicsRootSignature(mRootSignature.Get());
}





StandardShader::StandardShader(){
}

void StandardShader::CreateShader(ComPtr<ID3D12Device> device) {
	GraphicsShaderBase::CreateShader(device);
	mAttribute.set(0);
	mAttribute.set(1);
	mAttribute.set(2);
}

GraphicsShaderBase::InputLayout StandardShader::CreateInputLayout() {
	GraphicsShaderBase::InputLayout inputLayout{};

	inputLayout.ElementCount = 3;

	inputLayout.InputElements[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputLayout.InputElements[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputLayout.InputElements[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	return inputLayout;
}

GraphicsShaderBase::RootParameters StandardShader::CreateRootParameters() {
	GraphicsShaderBase::RootParameters params{};

	params.Parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	params.Parameters[0].Descriptor.ShaderRegister = 0;
	params.Parameters[0].Descriptor.RegisterSpace = 0;
	params.Parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.Parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	params.Parameters[1].Descriptor.ShaderRegister = 0;
	params.Parameters[1].Descriptor.RegisterSpace = 0;
	params.Parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.Parameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	params.Parameters[2].Descriptor.ShaderRegister = 1;
	params.Parameters[2].Descriptor.RegisterSpace = 0;
	params.Parameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.Ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	params.Ranges[0].NumDescriptors = Config::MAX_TEXTURE_COUNT<UINT>;
	params.Ranges[0].BaseShaderRegister = 2;
	params.Ranges[0].RegisterSpace = 0;
	params.Ranges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	params.Parameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	params.Parameters[3].DescriptorTable.NumDescriptorRanges = 1;
	params.Parameters[3].DescriptorTable.pDescriptorRanges = params.Ranges.data();
	params.Parameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.ParameterCount = 4; 

	return params;
}

UINT StandardShader::CreateNumOfRenderTarget() {
	return Config::GBUFFER_COUNT<UINT>;
}

void StandardShader::CreateRTVFormat(const std::span<DXGI_FORMAT>& formats) {
	formats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	formats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	formats[2] = DXGI_FORMAT_R32G32B32A32_FLOAT;
}

D3D12_SHADER_BYTECODE StandardShader::CreateVertexShader() {
	auto& blob = gShaderManager.GetShaderBlob("Standard", ShaderType::VertexShader);
	return { blob->GetBufferPointer(), blob->GetBufferSize() };
}

D3D12_SHADER_BYTECODE StandardShader::CreatePixelShader() {
	auto& blob = gShaderManager.GetShaderBlob("Standard", ShaderType::PixelShader);
	return { blob->GetBufferPointer(), blob->GetBufferSize() };
}

TerrainShader::TerrainShader() {

}

void TerrainShader::CreateShader(ComPtr<ID3D12Device> device) {
	GraphicsShaderBase::CreateShader(device);
	mAttribute.set(0);
	mAttribute.set(2);
	mAttribute.set(3);
}

GraphicsShaderBase::InputLayout TerrainShader::CreateInputLayout() {
	GraphicsShaderBase::InputLayout inputLayout{};

	inputLayout.ElementCount = 3;

	inputLayout.InputElements[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputLayout.InputElements[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputLayout.InputElements[2] = { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	return inputLayout;
}

GraphicsShaderBase::RootParameters TerrainShader::CreateRootParameters() {
	GraphicsShaderBase::RootParameters params{};

	params.Parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	params.Parameters[0].Descriptor.ShaderRegister = 0;
	params.Parameters[0].Descriptor.RegisterSpace = 0;
	params.Parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.Parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	params.Parameters[1].Descriptor.ShaderRegister = 0;
	params.Parameters[1].Descriptor.RegisterSpace = 0;
	params.Parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.Parameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	params.Parameters[2].Descriptor.ShaderRegister = 1;
	params.Parameters[2].Descriptor.RegisterSpace = 0;
	params.Parameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.Ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	params.Ranges[0].NumDescriptors = Config::MAX_TEXTURE_COUNT<UINT>;
	params.Ranges[0].BaseShaderRegister = 2;
	params.Ranges[0].RegisterSpace = 0;
	params.Ranges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	params.Parameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	params.Parameters[3].DescriptorTable.NumDescriptorRanges = 1;
	params.Parameters[3].DescriptorTable.pDescriptorRanges = params.Ranges.data();
	params.Parameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.ParameterCount = 4;

	return params;
}

D3D12_RASTERIZER_DESC TerrainShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	::ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));

//	rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}

UINT TerrainShader::CreateNumOfRenderTarget() {
	return Config::GBUFFER_COUNT<UINT>;
}

void TerrainShader::CreateRTVFormat(const std::span<DXGI_FORMAT>& formats) {
	formats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	formats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	formats[2] = DXGI_FORMAT_R32G32B32A32_FLOAT;
}

D3D12_SHADER_BYTECODE TerrainShader::CreateVertexShader() {
	auto& blob = gShaderManager.GetShaderBlob("TerrainTessellation", ShaderType::VertexShader);
	return { blob->GetBufferPointer(), blob->GetBufferSize() };
}

D3D12_SHADER_BYTECODE TerrainShader::CreateHullShader() {
	auto& blob = gShaderManager.GetShaderBlob("TerrainTessellation", ShaderType::HullShader);
	return { blob->GetBufferPointer(), blob->GetBufferSize() };
}

D3D12_SHADER_BYTECODE TerrainShader::CreateDomainShader() {
	auto& blob = gShaderManager.GetShaderBlob("TerrainTessellation", ShaderType::DomainShader);
	return { blob->GetBufferPointer(), blob->GetBufferSize() };
}

D3D12_SHADER_BYTECODE TerrainShader::CreatePixelShader() {
	auto& blob = gShaderManager.GetShaderBlob("TerrainTessellation", ShaderType::PixelShader);
	return { blob->GetBufferPointer(), blob->GetBufferSize() };
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE TerrainShader::CreatePrimitiveTopologyType() {
	return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
}

SkinnedShader::SkinnedShader() {
}



void SkinnedShader::CreateShader(ComPtr<ID3D12Device> device) {
	GraphicsShaderBase::CreateShader(device);
	mAttribute.set(0);
	mAttribute.set(1);
	mAttribute.set(2);
	mAttribute.set(6);
	mAttribute.set(7);
}

GraphicsShaderBase::InputLayout SkinnedShader::CreateInputLayout() {
	GraphicsShaderBase::InputLayout inputLayout{};

	inputLayout.ElementCount = 5;

	inputLayout.InputElements[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputLayout.InputElements[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputLayout.InputElements[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputLayout.InputElements[3] = { "BONEID", 0, DXGI_FORMAT_R32G32B32A32_SINT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputLayout.InputElements[4] = { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	return inputLayout;
}

GraphicsShaderBase::RootParameters SkinnedShader::CreateRootParameters() {
	GraphicsShaderBase::RootParameters params{};

	params.Parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	params.Parameters[0].Descriptor.ShaderRegister = 0;
	params.Parameters[0].Descriptor.RegisterSpace = 0;
	params.Parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.Parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	params.Parameters[1].Descriptor.ShaderRegister = 0;
	params.Parameters[1].Descriptor.RegisterSpace = 0;
	params.Parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.Parameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	params.Parameters[2].Descriptor.ShaderRegister = 1;
	params.Parameters[2].Descriptor.RegisterSpace = 0;
	params.Parameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.Ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	params.Ranges[0].NumDescriptors = Config::MAX_TEXTURE_COUNT<UINT>;
	params.Ranges[0].BaseShaderRegister = 2;
	params.Ranges[0].RegisterSpace = 0;
	params.Ranges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	params.Parameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	params.Parameters[3].DescriptorTable.NumDescriptorRanges = 1;
	params.Parameters[3].DescriptorTable.pDescriptorRanges = params.Ranges.data();
	params.Parameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.Parameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	params.Parameters[4].Descriptor.ShaderRegister = 2;
	params.Parameters[4].Descriptor.RegisterSpace = 1;
	params.Parameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


	params.ParameterCount = 5;

	return params;
}

UINT SkinnedShader::CreateNumOfRenderTarget() {
	return Config::GBUFFER_COUNT<UINT>;
}

void SkinnedShader::CreateRTVFormat(const std::span<DXGI_FORMAT>& formats) {
	formats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	formats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	formats[2] = DXGI_FORMAT_R32G32B32A32_FLOAT;
}

D3D12_SHADER_BYTECODE SkinnedShader::CreateVertexShader() {
	auto& blob = gShaderManager.GetShaderBlob("StandardAnimation", ShaderType::VertexShader);
	return { blob->GetBufferPointer(), blob->GetBufferSize() };
}

D3D12_SHADER_BYTECODE SkinnedShader::CreatePixelShader() {
	auto& blob = gShaderManager.GetShaderBlob("StandardAnimation", ShaderType::PixelShader);
	return { blob->GetBufferPointer(), blob->GetBufferSize() };
}






SkyBoxShader::SkyBoxShader() {
}

void SkyBoxShader::CreateShader(ComPtr<ID3D12Device> device) {
	GraphicsShaderBase::CreateShader(device);
	mAttribute.set(0);
	mAttribute.set(1);
	mAttribute.set(2);
}

GraphicsShaderBase::InputLayout SkyBoxShader::CreateInputLayout() {
	GraphicsShaderBase::InputLayout inputLayout{};

	inputLayout.ElementCount = 3;

	inputLayout.InputElements[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputLayout.InputElements[1] = { "TEXINDEX", 0, DXGI_FORMAT_R32_UINT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputLayout.InputElements[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	return inputLayout;
}

GraphicsShaderBase::RootParameters SkyBoxShader::CreateRootParameters() {
	GraphicsShaderBase::RootParameters params{};

	params.Parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	params.Parameters[0].Descriptor.ShaderRegister = 0;
	params.Parameters[0].Descriptor.RegisterSpace = 0;
	params.Parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.Parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	params.Parameters[1].Descriptor.ShaderRegister = 0;
	params.Parameters[1].Descriptor.RegisterSpace = 0;
	params.Parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.Parameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	params.Parameters[2].Descriptor.ShaderRegister = 1;
	params.Parameters[2].Descriptor.RegisterSpace = 0;
	params.Parameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.Ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	params.Ranges[0].NumDescriptors = Config::MAX_TEXTURE_COUNT<UINT>;
	params.Ranges[0].BaseShaderRegister = 2;
	params.Ranges[0].RegisterSpace = 0;
	params.Ranges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	params.Parameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	params.Parameters[3].DescriptorTable.NumDescriptorRanges = 1;
	params.Parameters[3].DescriptorTable.pDescriptorRanges = params.Ranges.data();
	params.Parameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.ParameterCount = 4;

	return params;
}

D3D12_DEPTH_STENCIL_DESC SkyBoxShader::CreateDepthStencilState() {
	D3D12_DEPTH_STENCIL_DESC depthStencilState;
	::ZeroMemory(&depthStencilState, sizeof(D3D12_DEPTH_STENCIL_DESC));

	depthStencilState.DepthEnable = FALSE;
	depthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_NEVER;
	depthStencilState.StencilEnable = FALSE;
	depthStencilState.StencilReadMask = 0x00;
	depthStencilState.StencilWriteMask = 0x00;
	depthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	depthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return depthStencilState;
}

UINT SkyBoxShader::CreateNumOfRenderTarget() {
	return Config::GBUFFER_COUNT<UINT>;
}

void SkyBoxShader::CreateRTVFormat(const std::span<DXGI_FORMAT>& formats) {
	formats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	formats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	formats[2] = DXGI_FORMAT_R32G32B32A32_FLOAT;
}

D3D12_SHADER_BYTECODE SkyBoxShader::CreateVertexShader() {
	auto& blob = gShaderManager.GetShaderBlob("SkyBox", ShaderType::VertexShader);
	return { blob->GetBufferPointer(), blob->GetBufferSize() };
}

D3D12_SHADER_BYTECODE SkyBoxShader::CreatePixelShader() {
	auto& blob = gShaderManager.GetShaderBlob("SkyBox", ShaderType::PixelShader);
	return { blob->GetBufferPointer(), blob->GetBufferSize() };
}

DefferedShader::DefferedShader() {
}

void DefferedShader::CreateShader(ComPtr<ID3D12Device> device) {
	GraphicsShaderBase::CreateShader(device);
}

GraphicsShaderBase::InputLayout DefferedShader::CreateInputLayout() {
	GraphicsShaderBase::InputLayout inputLayout{};

	inputLayout.ElementCount = 2;

	inputLayout.InputElements[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputLayout.InputElements[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	return inputLayout;
}

GraphicsShaderBase::RootParameters DefferedShader::CreateRootParameters() {
	GraphicsShaderBase::RootParameters params{};

	params.Parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	params.Parameters[0].Descriptor.ShaderRegister = 0;
	params.Parameters[0].Descriptor.RegisterSpace = 0;
	params.Parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.Ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	params.Ranges[0].NumDescriptors = Config::GBUFFER_COUNT<UINT> + 1 ; // Shadow Map
	params.Ranges[0].BaseShaderRegister = 0;
	params.Ranges[0].RegisterSpace = 0;
	params.Ranges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	params.Parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	params.Parameters[1].DescriptorTable.NumDescriptorRanges = 1;
	params.Parameters[1].DescriptorTable.pDescriptorRanges = params.Ranges.data();
	params.Parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	

	params.ParameterCount = 2;

	return params;
}

D3D12_DEPTH_STENCIL_DESC DefferedShader::CreateDepthStencilState() {
	D3D12_DEPTH_STENCIL_DESC depthStencilState;
	::ZeroMemory(&depthStencilState, sizeof(D3D12_DEPTH_STENCIL_DESC));

	depthStencilState.DepthEnable = FALSE;
	depthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_NEVER;
	depthStencilState.StencilEnable = FALSE;
	depthStencilState.StencilReadMask = 0x00;
	depthStencilState.StencilWriteMask = 0x00;
	depthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	depthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return depthStencilState;
}

DXGI_FORMAT DefferedShader::CreateDSVFormat() {
	return DXGI_FORMAT_UNKNOWN;
}

D3D12_RASTERIZER_DESC DefferedShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	::ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));

	//rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}

D3D12_SHADER_BYTECODE DefferedShader::CreateVertexShader() {
	auto& blob = gShaderManager.GetShaderBlob("Deffered", ShaderType::VertexShader);
	return { blob->GetBufferPointer(), blob->GetBufferSize() };
}

D3D12_SHADER_BYTECODE DefferedShader::CreatePixelShader() {
	auto& blob = gShaderManager.GetShaderBlob("Deffered", ShaderType::PixelShader);
	return { blob->GetBufferPointer(), blob->GetBufferSize() };
}

SkeletonBBShader::SkeletonBBShader() {
}

void SkeletonBBShader::CreateShader(ComPtr<ID3D12Device> device) {
	GraphicsShaderBase::CreateShader(device);
}

GraphicsShaderBase::InputLayout SkeletonBBShader::CreateInputLayout() {
	return InputLayout();
}

GraphicsShaderBase::RootParameters SkeletonBBShader::CreateRootParameters() {
	GraphicsShaderBase::RootParameters params{};

	params.Parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	params.Parameters[0].Descriptor.ShaderRegister = 0;
	params.Parameters[0].Descriptor.RegisterSpace = 0;
	params.Parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.Parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	params.Parameters[1].Descriptor.ShaderRegister = 0;
	params.Parameters[1].Descriptor.RegisterSpace = 0;
	params.Parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.ParameterCount = 2;

	return params;
}

UINT SkeletonBBShader::CreateNumOfRenderTarget() {
	return Config::GBUFFER_COUNT<UINT>;
}

void SkeletonBBShader::CreateRTVFormat(const std::span<DXGI_FORMAT>& formats) {
	formats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	formats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	formats[2] = DXGI_FORMAT_R32G32B32A32_FLOAT;
}

D3D12_SHADER_BYTECODE SkeletonBBShader::CreateVertexShader() {
	auto& blob = gShaderManager.GetShaderBlob("SkeletonBoundingBoxRender", ShaderType::VertexShader);
	return { blob->GetBufferPointer(), blob->GetBufferSize() };
}

D3D12_SHADER_BYTECODE SkeletonBBShader::CreateGeometryShader() {
	auto& blob = gShaderManager.GetShaderBlob("SkeletonBoundingBoxRender", ShaderType::GeometryShader);
	return { blob->GetBufferPointer(), blob->GetBufferSize() };
}

D3D12_SHADER_BYTECODE SkeletonBBShader::CreatePixelShader() {
	auto& blob = gShaderManager.GetShaderBlob("SkeletonBoundingBoxRender", ShaderType::PixelShader);
	return { blob->GetBufferPointer(), blob->GetBufferSize() };
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE SkeletonBBShader::CreatePrimitiveTopologyType() {
	return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
}



StandardBBShader::StandardBBShader() {
}

void StandardBBShader::CreateShader(ComPtr<ID3D12Device> device) {
	GraphicsShaderBase::CreateShader(device);
}

GraphicsShaderBase::InputLayout StandardBBShader::CreateInputLayout() {
	return InputLayout();
}

GraphicsShaderBase::RootParameters StandardBBShader::CreateRootParameters() {
	GraphicsShaderBase::RootParameters params{};

	params.Parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	params.Parameters[0].Descriptor.ShaderRegister = 0;
	params.Parameters[0].Descriptor.RegisterSpace = 0;
	params.Parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.Parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	params.Parameters[1].Descriptor.ShaderRegister = 0;
	params.Parameters[1].Descriptor.RegisterSpace = 0;
	params.Parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.ParameterCount = 2;

	return params;
}

UINT StandardBBShader::CreateNumOfRenderTarget() {
	return Config::GBUFFER_COUNT<UINT>;
}

void StandardBBShader::CreateRTVFormat(const std::span<DXGI_FORMAT>& formats) {
	formats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	formats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	formats[2] = DXGI_FORMAT_R32G32B32A32_FLOAT;
}

D3D12_SHADER_BYTECODE StandardBBShader::CreateVertexShader() {
	auto& blob = gShaderManager.GetShaderBlob("StandardBoundingBoxRender", ShaderType::VertexShader);
	return { blob->GetBufferPointer(), blob->GetBufferSize() };
}

D3D12_SHADER_BYTECODE StandardBBShader::CreateGeometryShader() {
	auto& blob = gShaderManager.GetShaderBlob("StandardBoundingBoxRender", ShaderType::GeometryShader);
	return { blob->GetBufferPointer(), blob->GetBufferSize() };
}

D3D12_SHADER_BYTECODE StandardBBShader::CreatePixelShader() {
	auto& blob = gShaderManager.GetShaderBlob("StandardBoundingBoxRender", ShaderType::PixelShader);
	return { blob->GetBufferPointer(), blob->GetBufferSize() };
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE StandardBBShader::CreatePrimitiveTopologyType() {
	return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
}

ParticleSOShader::ParticleSOShader() {
}

void ParticleSOShader::CreateShader(ComPtr<ID3D12Device> device) {
	GraphicsShaderBase::CreateShader(device);
}

GraphicsShaderBase::InputLayout ParticleSOShader::CreateInputLayout() {
	InputLayout result{}; 

	result.InputElements[0] = { "POSITION",			0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	result.InputElements[1] = { "WIDTH",			0, DXGI_FORMAT_R32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	result.InputElements[2] = { "HEIGHT",			0, DXGI_FORMAT_R32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	result.InputElements[3] = { "MATERIAL",			0, DXGI_FORMAT_R32_UINT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	result.InputElements[4] = { "SPRITABLE",		0, DXGI_FORMAT_R32_UINT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	result.InputElements[5] = { "SPRITEFRAMEINROW", 0, DXGI_FORMAT_R32_UINT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	result.InputElements[6] = { "SPRITEFRAMEINCOL", 0, DXGI_FORMAT_R32_UINT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	result.InputElements[7] = { "SPRITEDURATION",	0, DXGI_FORMAT_R32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	result.InputElements[8] = { "DIRECTION",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	result.InputElements[9] = { "VELOCITY",			0, DXGI_FORMAT_R32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	result.InputElements[10] = { "TOTALLIFETIME",	0, DXGI_FORMAT_R32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	result.InputElements[11] = { "LIFETIME",		0, DXGI_FORMAT_R32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	result.InputElements[12] = { "PARTICLETYPE",	0, DXGI_FORMAT_R32_UINT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	result.InputElements[13] = { "EMITTYPE",		0, DXGI_FORMAT_R32_UINT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	result.InputElements[14] = { "REMAINEMIT",		0, DXGI_FORMAT_R32_UINT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	result.InputElements[15] = { "EMITINDEX",		0, DXGI_FORMAT_R32_UINT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	result.ElementCount = 16;

	return result;
}

GraphicsShaderBase::RootParameters ParticleSOShader::CreateRootParameters() {
	RootParameters result{};

	result.Parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	result.Parameters[0].Constants.Num32BitValues = 2;
	result.Parameters[0].Constants.ShaderRegister = 0;
	result.Parameters[0].Constants.RegisterSpace = 0;
	result.Parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// TerrainHeader 
	result.Parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	result.Parameters[1].Descriptor.ShaderRegister = 1;
	result.Parameters[1].Descriptor.RegisterSpace = 0;
	result.Parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// RandomBuffer 
	result.Parameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	result.Parameters[2].Descriptor.ShaderRegister = 0;
	result.Parameters[2].Descriptor.RegisterSpace = 0;
	result.Parameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	
	// Emit Buffer 
	result.Parameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	result.Parameters[3].Descriptor.ShaderRegister = 1;
	result.Parameters[3].Descriptor.RegisterSpace = 0;
	result.Parameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// Terrain Buffer 
	result.Parameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	result.Parameters[4].Descriptor.ShaderRegister = 2;
	result.Parameters[4].Descriptor.RegisterSpace = 0;
	result.Parameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	result.ParameterCount = 5; 

	return result;
}

D3D12_RASTERIZER_DESC ParticleSOShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC result{};

	result.FillMode = D3D12_FILL_MODE_SOLID;
	result.CullMode = D3D12_CULL_MODE_NONE;
	result.FrontCounterClockwise = FALSE;
	result.DepthBias = 0;
	result.DepthBiasClamp = 0.0f;
	result.SlopeScaledDepthBias = 0.0f;
	result.DepthClipEnable = TRUE;
	result.MultisampleEnable = FALSE;
	result.AntialiasedLineEnable = FALSE;
	result.ForcedSampleCount = 0;
	result.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return result;
}

D3D12_BLEND_DESC ParticleSOShader::CreateBlendState() {
	D3D12_BLEND_DESC result{}; 

	// Deffered! 

	return result;
}

D3D12_DEPTH_STENCIL_DESC ParticleSOShader::CreateDepthStencilState() {
	D3D12_DEPTH_STENCIL_DESC result{};

	result.DepthEnable = FALSE;
	result.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	result.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	result.StencilEnable = FALSE;
	result.StencilReadMask = 0x00;
	result.StencilWriteMask = 0x00;
	result.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	result.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	result.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	result.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	result.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	result.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	result.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	result.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return result;
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE ParticleSOShader::CreatePrimitiveTopologyType() {
	return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
}

GraphicsShaderBase::StreamOutputState ParticleSOShader::CreateStreamOutputState() {
	StreamOutputState result{};

	result.SODeclaration[0]		= { 0, "POSITION",			0, 0, 3, 0 };
	result.SODeclaration[1]		= { 0, "WIDTH",				0, 0, 1, 0 };
	result.SODeclaration[2]		= { 0, "HEIGHT",			0, 0, 1, 0 };
	result.SODeclaration[3]		= { 0, "MATERIAL",			0, 0, 1, 0 };

	result.SODeclaration[4]		= { 0, "SPRITABLE",			0, 0, 1, 0 };
	result.SODeclaration[5]		= { 0, "SPRITEFRAMEINROW",	0, 0, 1, 0 };
	result.SODeclaration[6]		= { 0, "SPRITEFRAMEINCOL",	0, 0, 1, 0 };
	result.SODeclaration[7]		= { 0, "SPRITEDURATION",	0, 0, 1, 0 };

	result.SODeclaration[8]		= { 0, "DIRECTION",			0, 0, 3, 0 };
	result.SODeclaration[9]		= { 0, "VELOCITY",			0, 0, 1, 0 };
	result.SODeclaration[10]	= { 0, "TOTALLIFETIME",		0, 0, 1, 0 };
	result.SODeclaration[11]	= { 0, "LIFETIME",			0, 0, 1, 0 };

	result.SODeclaration[12]	= { 0, "PARTICLETYPE",		0, 0, 1, 0 };
	result.SODeclaration[13]	= { 0, "EMITTYPE",			0, 0, 1, 0 };
	result.SODeclaration[14]	= { 0, "REMAINEMIT",		0, 0, 1, 0 };
	result.SODeclaration[15]	= { 0, "EMITINDEX",			0, 0, 1, 0 };

	result.SODeclarationCount = 16;

	result.BufferStrides[0] = sizeof(ParticleVertex);

	result.BufferCount = 1; 

	result.RasterizedStream = D3D12_SO_NO_RASTERIZED_STREAM; 

	return result; 
}

D3D12_SHADER_BYTECODE ParticleSOShader::CreateVertexShader() {
	auto& blob = gShaderManager.GetShaderBlob("ParticleSO", ShaderType::VertexShader);
	return { blob->GetBufferPointer(), blob->GetBufferSize() };
}

D3D12_SHADER_BYTECODE ParticleSOShader::CreateGeometryShader() {
	auto& blob = gShaderManager.GetShaderBlob("ParticleSO", ShaderType::GeometryShader);
	return { blob->GetBufferPointer(), blob->GetBufferSize() };
}

UINT ParticleSOShader::CreateNumOfRenderTarget() {
	return 0;
}

void ParticleSOShader::CreateRTVFormat(const std::span<DXGI_FORMAT>& targets) {
	targets[0] = DXGI_FORMAT_UNKNOWN;
}

D3D12_ROOT_SIGNATURE_FLAGS ParticleSOShader::CreateRootSignatureFlag() {
	return D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;
}



ParticleGSShader::ParticleGSShader() {
}

void ParticleGSShader::CreateShader(ComPtr<ID3D12Device> device) {
	GraphicsShaderBase::CreateShader(device);
}

GraphicsShaderBase::InputLayout ParticleGSShader::CreateInputLayout() {
	InputLayout result{};

	result.InputElements[0] = { "POSITION",			0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	result.InputElements[1] = { "WIDTH",			0, DXGI_FORMAT_R32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	result.InputElements[2] = { "HEIGHT",			0, DXGI_FORMAT_R32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	result.InputElements[3] = { "MATERIAL",			0, DXGI_FORMAT_R32_UINT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	result.InputElements[4] = { "SPRITABLE",		0, DXGI_FORMAT_R32_UINT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	result.InputElements[5] = { "SPRITEFRAMEINROW", 0, DXGI_FORMAT_R32_UINT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	result.InputElements[6] = { "SPRITEFRAMEINCOL", 0, DXGI_FORMAT_R32_UINT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	result.InputElements[7] = { "SPRITEDURATION",	0, DXGI_FORMAT_R32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	result.InputElements[8] = { "DIRECTION",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	result.InputElements[9] = { "VELOCITY",			0, DXGI_FORMAT_R32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	result.InputElements[10] = { "TOTALLIFETIME",	0, DXGI_FORMAT_R32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	result.InputElements[11] = { "LIFETIME",		0, DXGI_FORMAT_R32_FLOAT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	result.InputElements[12] = { "PARTICLETYPE",	0, DXGI_FORMAT_R32_UINT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	result.InputElements[13] = { "EMITTYPE",		0, DXGI_FORMAT_R32_UINT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	result.InputElements[14] = { "REMAINEMIT",		0, DXGI_FORMAT_R32_UINT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	result.InputElements[15] = { "EMITINDEX",		0, DXGI_FORMAT_R32_UINT,		0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	result.ElementCount = 16;

	return result;
}

GraphicsShaderBase::RootParameters ParticleGSShader::CreateRootParameters() {
	RootParameters result{};

	result.Parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	result.Parameters[0].Descriptor.ShaderRegister = 0;
	result.Parameters[0].Descriptor.RegisterSpace = 0;
	result.Parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	result.Parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	result.Parameters[1].Constants.Num32BitValues = 1;
	result.Parameters[1].Constants.ShaderRegister = 1;
	result.Parameters[1].Constants.RegisterSpace = 0;
	result.Parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	result.Parameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	result.Parameters[2].Descriptor.ShaderRegister = 0;
	result.Parameters[2].Descriptor.RegisterSpace = 0;
	result.Parameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	result.Ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	result.Ranges[0].NumDescriptors = Config::MAX_TEXTURE_COUNT<UINT>;
	result.Ranges[0].BaseShaderRegister = 1;
	result.Ranges[0].RegisterSpace = 0;
	result.Ranges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	
	result.Parameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	result.Parameters[3].DescriptorTable.NumDescriptorRanges = 1;
	result.Parameters[3].DescriptorTable.pDescriptorRanges = result.Ranges.data();
	result.Parameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	result.ParameterCount = 4; 

	return result;
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE ParticleGSShader::CreatePrimitiveTopologyType() {
	return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
}

D3D12_SHADER_BYTECODE ParticleGSShader::CreateVertexShader() {
	auto& blob = gShaderManager.GetShaderBlob("ParticleGS", ShaderType::VertexShader);
	return { blob->GetBufferPointer(), blob->GetBufferSize() };
}

D3D12_SHADER_BYTECODE ParticleGSShader::CreateGeometryShader() {
	auto& blob = gShaderManager.GetShaderBlob("ParticleGS", ShaderType::GeometryShader);
	return { blob->GetBufferPointer(), blob->GetBufferSize() };
}

D3D12_SHADER_BYTECODE ParticleGSShader::CreatePixelShader() {
	auto& blob = gShaderManager.GetShaderBlob("ParticleGS", ShaderType::PixelShader);
	return { blob->GetBufferPointer(), blob->GetBufferSize() };
}

UINT ParticleGSShader::CreateNumOfRenderTarget() {
	return 3;
}

void ParticleGSShader::CreateRTVFormat(const std::span<DXGI_FORMAT>& targets) {
	targets[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	targets[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	targets[2] = DXGI_FORMAT_R32G32B32A32_FLOAT;
}




TreeShader::TreeShader() {
}

void TreeShader::CreateShader(ComPtr<ID3D12Device> device) {
	GraphicsShaderBase::CreateShader(device);
	mAttribute.set(0);
	mAttribute.set(1);
	mAttribute.set(2);
}

GraphicsShaderBase::InputLayout TreeShader::CreateInputLayout() {
	GraphicsShaderBase::InputLayout inputLayout{};

	inputLayout.ElementCount = 3;

	inputLayout.InputElements[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputLayout.InputElements[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputLayout.InputElements[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	return inputLayout;
}

GraphicsShaderBase::RootParameters TreeShader::CreateRootParameters() {
	GraphicsShaderBase::RootParameters params{};

	params.Parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	params.Parameters[0].Descriptor.ShaderRegister = 0;
	params.Parameters[0].Descriptor.RegisterSpace = 0;
	params.Parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.Parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	params.Parameters[1].Descriptor.ShaderRegister = 0;
	params.Parameters[1].Descriptor.RegisterSpace = 0;
	params.Parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.Parameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	params.Parameters[2].Descriptor.ShaderRegister = 1;
	params.Parameters[2].Descriptor.RegisterSpace = 0;
	params.Parameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.Ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	params.Ranges[0].NumDescriptors = Config::MAX_TEXTURE_COUNT<UINT>;
	params.Ranges[0].BaseShaderRegister = 2;
	params.Ranges[0].RegisterSpace = 0;
	params.Ranges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	params.Parameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	params.Parameters[3].DescriptorTable.NumDescriptorRanges = 1;
	params.Parameters[3].DescriptorTable.pDescriptorRanges = params.Ranges.data();
	params.Parameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.ParameterCount = 4;

	return params;
}

UINT TreeShader::CreateNumOfRenderTarget() {
	return Config::GBUFFER_COUNT<UINT>;
}

void TreeShader::CreateRTVFormat(const std::span<DXGI_FORMAT>& formats) {
	formats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	formats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	formats[2] = DXGI_FORMAT_R32G32B32A32_FLOAT;
}

D3D12_SHADER_BYTECODE TreeShader::CreateVertexShader() {
	auto& blob = gShaderManager.GetShaderBlob("Standard", ShaderType::VertexShader);
	return { blob->GetBufferPointer(), blob->GetBufferSize() };
}

D3D12_SHADER_BYTECODE TreeShader::CreatePixelShader() {
	auto& blob = gShaderManager.GetShaderBlob("Standard", ShaderType::PixelShader);
	return { blob->GetBufferPointer(), blob->GetBufferSize() };
}

D3D12_RASTERIZER_DESC TreeShader::CreateRasterizerState() {
	D3D12_RASTERIZER_DESC rasterizerDesc;
	::ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));

	//	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}



SkyFogShader::SkyFogShader() {
}

void SkyFogShader::CreateShader(ComPtr<ID3D12Device> device) {
	GraphicsShaderBase::CreateShader(device);
	mAttribute.set(0);
}

GraphicsShaderBase::InputLayout SkyFogShader::CreateInputLayout() {
	GraphicsShaderBase::InputLayout inputLayout{};

	inputLayout.ElementCount = 1;

	inputLayout.InputElements[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	return inputLayout;
}

GraphicsShaderBase::RootParameters SkyFogShader::CreateRootParameters() {
	GraphicsShaderBase::RootParameters params{};

	params.Parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	params.Parameters[0].Descriptor.ShaderRegister = 0;
	params.Parameters[0].Descriptor.RegisterSpace = 0;
	params.Parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.Parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	params.Parameters[1].Descriptor.ShaderRegister = 0;
	params.Parameters[1].Descriptor.RegisterSpace = 0;
	params.Parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.Parameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	params.Parameters[2].Descriptor.ShaderRegister = 1;
	params.Parameters[2].Descriptor.RegisterSpace = 0;
	params.Parameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.Ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	params.Ranges[0].NumDescriptors = Config::MAX_TEXTURE_COUNT<UINT>;
	params.Ranges[0].BaseShaderRegister = 2;
	params.Ranges[0].RegisterSpace = 0;
	params.Ranges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	params.Parameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	params.Parameters[3].DescriptorTable.NumDescriptorRanges = 1;
	params.Parameters[3].DescriptorTable.pDescriptorRanges = params.Ranges.data();
	params.Parameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	params.ParameterCount = 4;

	return params;
}

D3D12_DEPTH_STENCIL_DESC SkyFogShader::CreateDepthStencilState() {
	D3D12_DEPTH_STENCIL_DESC depthStencilState;
	::ZeroMemory(&depthStencilState, sizeof(D3D12_DEPTH_STENCIL_DESC));

	depthStencilState.DepthEnable = FALSE;
	depthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_NEVER;
	depthStencilState.StencilEnable = FALSE;
	depthStencilState.StencilReadMask = 0x00;
	depthStencilState.StencilWriteMask = 0x00;
	depthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	depthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return depthStencilState;
}

UINT SkyFogShader::CreateNumOfRenderTarget() {
	return Config::GBUFFER_COUNT<UINT>;
}

void SkyFogShader::CreateRTVFormat(const std::span<DXGI_FORMAT>& formats) {
	formats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	formats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	formats[2] = DXGI_FORMAT_R32G32B32A32_FLOAT;
}

D3D12_SHADER_BYTECODE SkyFogShader::CreateVertexShader() {
	auto& blob = gShaderManager.GetShaderBlob("SkyFog", ShaderType::VertexShader);
	return { blob->GetBufferPointer(), blob->GetBufferSize() };
}

D3D12_SHADER_BYTECODE SkyFogShader::CreatePixelShader() {
	auto& blob = gShaderManager.GetShaderBlob("SkyFog", ShaderType::PixelShader);
	return { blob->GetBufferPointer(), blob->GetBufferSize() };
}