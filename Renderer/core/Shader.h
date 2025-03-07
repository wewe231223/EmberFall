#pragma once 
#include <array>
#include <string>
#include <filesystem>
#include <span>
#include <bitset>
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

class ShaderFileManager {
	constexpr static const char* SHADER_METADATA_PATH = "Shader/ShaderMetadata.txt";
public:
	ShaderFileManager(); 
	~ShaderFileManager() = default; 

	ShaderFileManager(const ShaderFileManager&) = delete;
	ShaderFileManager& operator=(const ShaderFileManager&) = delete;

	ShaderFileManager(ShaderFileManager&&) = delete;
	ShaderFileManager& operator=(ShaderFileManager&&) = delete;
public:
	void Test() {}
	
	ComPtr<ID3D12Blob>& GetShaderBlob(const std::string& name, ShaderType type);
	std::array<ComPtr<ID3D12Blob>, ShaderType::END>& GetShaderBlobs(const std::string& name);
private:
	void ProcessShader(const std::filesystem::path& source, const std::vector<std::tuple<std::string, std::string, std::string>>& shaderInfo, const std::vector<std::filesystem::path>& includes);
	void ReCompile(const std::filesystem::path& source, const std::string& type, const std::string& model, const std::string& entry);
	/* 호출하기 전에 파일이 있는지 확인해야 함 */ 
	void Load(ComPtr<ID3D12Blob>& blob, const std::filesystem::path& path);
private:
	std::unordered_map<std::string,std::array<ComPtr<ID3D12Blob>, ShaderType::END>> mShaderBlobs{};
};

extern ShaderFileManager gShaderManager; 


class GraphicsShaderBase {
protected:
	struct InputLayout {
		std::array<D3D12_INPUT_ELEMENT_DESC, 16> InputElements{};
		UINT ElementCount{ 0 };
	};

	struct RootParameters {
		std::array<D3D12_ROOT_PARAMETER, 16> Parameters{};
		UINT ParameterCount{ 0 };

		std::array<D3D12_DESCRIPTOR_RANGE, 16> Ranges{};
	};
public:
	GraphicsShaderBase(); 
	virtual ~GraphicsShaderBase(); 

	GraphicsShaderBase(const GraphicsShaderBase&) = default;
	GraphicsShaderBase& operator=(const GraphicsShaderBase&) = default;

	GraphicsShaderBase(GraphicsShaderBase&&) = default;
	GraphicsShaderBase& operator=(GraphicsShaderBase&&) = default;

public:
	virtual void CreateShader(ComPtr<ID3D12Device> device);

	virtual UINT GetShaderID() const;
	const std::bitset<8>& GetAttribute() const;
	void SetShader(ComPtr<ID3D12GraphicsCommandList> commandList);
protected:

	virtual InputLayout CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	virtual RootParameters CreateRootParameters(); 
	virtual D3D12_ROOT_SIGNATURE_FLAGS CreateRootSignatureFlag(); 
	virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE CreatePrimitiveTopologyType();
	virtual UINT CreateNumOfRenderTarget(); 
	virtual void CreateRTVFormat(const std::span<DXGI_FORMAT>&);
	virtual DXGI_FORMAT CreateDSVFormat();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader();
	virtual D3D12_SHADER_BYTECODE CreateHullShader();
	virtual D3D12_SHADER_BYTECODE CreateDomainShader();
protected:
	std::bitset<8> mAttribute{ 0b0000000 };
private:
	ComPtr<ID3D12RootSignature> mRootSignature{};
	ComPtr<ID3D12PipelineState> mPipelineState{};
};


class StandardShader : public GraphicsShaderBase {
public:
	StandardShader();
	virtual ~StandardShader() = default;
public:
	virtual void CreateShader(ComPtr<ID3D12Device> device) override;
protected:
	virtual InputLayout CreateInputLayout() override;
	virtual RootParameters CreateRootParameters() override;

	virtual UINT CreateNumOfRenderTarget() override;
	virtual void CreateRTVFormat(const std::span<DXGI_FORMAT>&) override;

	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;
};


class TerrainShader : public GraphicsShaderBase {
public:
	TerrainShader();
	virtual ~TerrainShader() = default;
public:
	virtual void CreateShader(ComPtr<ID3D12Device> device) override;
protected:
	virtual InputLayout CreateInputLayout() override;
	virtual RootParameters CreateRootParameters() override;

	virtual D3D12_RASTERIZER_DESC CreateRasterizerState() override;

	virtual UINT CreateNumOfRenderTarget() override;
	virtual void CreateRTVFormat(const std::span<DXGI_FORMAT>&) override;

	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
	virtual D3D12_SHADER_BYTECODE CreateHullShader() override;
	virtual D3D12_SHADER_BYTECODE CreateDomainShader() override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;

	virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE CreatePrimitiveTopologyType() override;

};


class SkinnedShader : public GraphicsShaderBase {
public:
	SkinnedShader();
	virtual ~SkinnedShader() = default;
public:
	virtual void CreateShader(ComPtr<ID3D12Device> device) override;
protected:
	virtual InputLayout CreateInputLayout() override;
	virtual RootParameters CreateRootParameters() override;

	virtual UINT CreateNumOfRenderTarget() override;
	virtual void CreateRTVFormat(const std::span<DXGI_FORMAT>&) override;

	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;
};



class SkyBoxShader : public GraphicsShaderBase {
public:
	SkyBoxShader();
	virtual ~SkyBoxShader() = default;
public:
	virtual void CreateShader(ComPtr<ID3D12Device> device) override;
protected:
	virtual InputLayout CreateInputLayout() override;
	virtual RootParameters CreateRootParameters() override;

	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() override;

	virtual UINT CreateNumOfRenderTarget() override;
	virtual void CreateRTVFormat(const std::span<DXGI_FORMAT>&) override;

	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;
};


class DefferedShader : public GraphicsShaderBase {
public:
	DefferedShader();
	virtual ~DefferedShader() = default;
public:
	virtual void CreateShader(ComPtr<ID3D12Device> device) override;
protected:
	virtual InputLayout CreateInputLayout() override;
	virtual RootParameters CreateRootParameters() override;

	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() override;
	virtual DXGI_FORMAT CreateDSVFormat() override;
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState() override;


	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;
};