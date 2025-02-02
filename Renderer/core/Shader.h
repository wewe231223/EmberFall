#pragma once 
#include <string>
#include <filesystem>
#include "../Utility/DirectXInclude.h"


enum ShaderType : BYTE {
	None = 0xFF,
	VertexShader = 0,
	PixelShader,
	GeometryShader,
	HullShader,
	DomainShader,
	END,
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
	void Test() {}
private:
	void ProcessShader(const std::filesystem::path& source, const std::vector<std::tuple<std::string, std::string, std::string>>& shaderInfo, const std::vector<std::filesystem::path>& includes);
	void ReCompile(const std::filesystem::path& source, const std::string& type, const std::string& model, const std::string& entry);
	/* 호출하기 전에 파일이 있는지 확인해야 함 */ 
	void Load(ComPtr<ID3D12Blob>& blob, const std::filesystem::path& path);
private:
	std::unordered_map<std::string,std::array<ComPtr<ID3D12Blob>, ShaderType::END>> mShaderBlobs{};
};

extern ShaderManager gShaderManager; 

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

