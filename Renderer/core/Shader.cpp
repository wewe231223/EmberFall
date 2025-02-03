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

//------------------------------------------------------------------------------------------------[ ShaderManager ]------------------------------------------------------------------------------------------------

ShaderManager::ShaderManager() {
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

		ShaderManager::ProcessShader( "Shader/Sources/" + source, shaderInfo, includes);
	}
}

ComPtr<ID3D12Blob>& ShaderManager::GetShaderBlob(const std::string& name, ShaderType type) {
	return mShaderBlobs[name][type];
}

std::array<ComPtr<ID3D12Blob>, ShaderType::END>& ShaderManager::GetShaderBlobs(const std::string& name) {
	return mShaderBlobs[name];
}

void ShaderManager::ProcessShader(const std::filesystem::path& source, const std::vector<std::tuple<std::string, std::string, std::string>>& shaderInfo, const std::vector<std::filesystem::path>& includes) {
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
			ShaderManager::ReCompile(source, type, model, entry);
		} 
	}
	else {
		for (const auto& [type, model, entry] : shaderInfo) {
			std::string binaryFileName{ source.stem().string() + "_" + type + ".bin" };
			std::filesystem::path binaryPath{ binPath / binaryFileName };

			ComPtr<ID3D12Blob> shaderBlob{};
			ShaderManager::Load(shaderBlob, binaryPath);

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

void ShaderManager::ReCompile(const std::filesystem::path& source, const std::string& type, const std::string& model, const std::string& entry) {
	Console.Log("Compiling shader : {}", LogType::Info, source.string());

	ComPtr<ID3D12Blob> shaderBlob{};
	ComPtr<ID3D12Blob> errorBlob{};
	
	auto hr = ::D3DCompileFromFile(source.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry.c_str(), model.c_str(), 0, 0, &shaderBlob, &errorBlob);

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

		ShaderManager::Load(shaderBlob, binaryPath);
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

void ShaderManager::Load(ComPtr<ID3D12Blob>& blob, const std::filesystem::path& path) {
	auto size = std::filesystem::file_size(path);

	::D3DCreateBlob(size, blob.GetAddressOf());

	std::ifstream file{ path, std::ios::binary };
	file.read(static_cast<char*>(blob->GetBufferPointer()), size);
}

ShaderManager gShaderManager{};

//------------------------------------------------------------------------------------------------[ ShaderManager ]------------------------------------------------------------------------------------------------

GraphicsShaderBase::GraphicsShaderBase(ComPtr<ID3D12Device> device) {
	static std::array<CD3DX12_STATIC_SAMPLER_DESC, 6> staticSamplers{};

	staticSamplers[0] = { 0, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP };
	staticSamplers[1] = { 1, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP };
	staticSamplers[2] = { 2, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP };
	staticSamplers[3] = { 3, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP };
	staticSamplers[4] = { 4, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0.0f, 8 };
	staticSamplers[5] = { 5, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0.0f, 8 };

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
	CrashExp(SUCCEEDED(hr), reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));

	CheckHR(device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));
	
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};

	auto inputLayout = CreateInputLayout();

	// 뭔가 뭔가.. 여기서 더..? 
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
	CreateRTVFormat(psoDesc.RTVFormats);
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.DSVFormat = CreateDSVFormat();

	CheckHR(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPipelineState)));
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

void GraphicsShaderBase::CreateRTVFormat(DXGI_FORMAT* formats) {
	::memset(formats, static_cast<int>(Config::RENDER_TARGET_FORMAT), sizeof(DXGI_FORMAT) * 8);
}

DXGI_FORMAT GraphicsShaderBase::CreateDSVFormat() {
	return DXGI_FORMAT_D24_UNORM_S8_UINT;
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
