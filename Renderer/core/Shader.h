#pragma once 
#include <string>
#include <filesystem>
#include "../Utility/DirectXInclude.h"

enum ShaderType : BYTE {
	None = 1,
	VertexShader,
	PixelShader,
	GeometryShader,
	HullShader,
	DomainShader,
};


class ShaderManager {
	constexpr static const char* SHADER_METADATA_PATH = "Shader/ShaderMetadata.txt";
public:
	ShaderManager(); 
	~ShaderManager() = default; 

	ShaderManager(const ShaderManager&) = delete;
	ShaderManager& operator=(const ShaderManager&) = delete;

	ShaderManager(ShaderManager&&) = delete;
	ShaderManager& operator=(ShaderManager&&) = delete;
public:

private:
	void ReCompile(const std::filesystem::path& source, const std::string model, const std::string entry);
private:
	std::unordered_map<std::string, std::pair<size_t,ComPtr<ID3D10Blob>>> mShaderBlobs{};
};


class GraphicsShaderBase {
	struct InputLayout {
		std::array<D3D12_INPUT_ELEMENT_DESC, 16> InputElements{};
		UINT ElementCount{ 0 };
	};
public:
	GraphicsShaderBase(); 
	virtual ~GraphicsShaderBase(); 
public:
	virtual InputLayout CreateInputLayout(); 
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState(); 
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(); 

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(); 
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader();
	virtual D3D12_SHADER_BYTECODE CreateHullShader();
	virtual D3D12_SHADER_BYTECODE CreateDomainShader();

	
};