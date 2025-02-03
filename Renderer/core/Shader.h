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

enum class ConstantShaderRegister : WORD {
	NONE = 0xFF'FF,
	c0 = 0,
	c1,
	c2,
	c3,
	c4,
	c5,
	END
};

enum class ShaderResourceRegister : WORD {
	NONE = 0xFF'FF,
	t0 = 0,
	t1,
	t2,
	t3,
	t4,
	t5,
	END
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

extern ShaderManager gShaderManager; 

class GraphicsShaderBase {
protected:
	struct InputLayout {
		std::array<D3D12_INPUT_ELEMENT_DESC, 16> InputElements{};
		UINT ElementCount{ 0 };
	};

	struct RootParameters {
		std::array<D3D12_ROOT_PARAMETER, 16> Parameters{};
		UINT ParameterCount{ 0 };
	};
public:
	GraphicsShaderBase(ComPtr<ID3D12Device> device); 
	virtual ~GraphicsShaderBase(); 

	GraphicsShaderBase(const GraphicsShaderBase&) = delete;
	GraphicsShaderBase& operator=(const GraphicsShaderBase&) = delete;

	GraphicsShaderBase(GraphicsShaderBase&&) = default;
	GraphicsShaderBase& operator=(GraphicsShaderBase&&) = default;
public:
	virtual void AppendConstantBuffer(ConstantShaderRegister regNo, void* data) {};
	virtual void AppendShaderResourceBuffer(ShaderResourceRegister regNo, void* data) {};
private:
	virtual InputLayout CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	virtual RootParameters CreateRootParameters(); 
	virtual D3D12_ROOT_SIGNATURE_FLAGS CreateRootSignatureFlag(); 
	virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE CreatePrimitiveTopologyType();
	virtual UINT CreateNumOfRenderTarget(); 
	virtual void CreateRTVFormat(DXGI_FORMAT* formats);
	virtual DXGI_FORMAT CreateDSVFormat();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader();
	virtual D3D12_SHADER_BYTECODE CreateHullShader();
	virtual D3D12_SHADER_BYTECODE CreateDomainShader();

	virtual void CreateBuffers(ComPtr<ID3D12Device> device) {};
private:
	ComPtr<ID3D12RootSignature> mRootSignature{};
	ComPtr<ID3D12PipelineState> mPipelineState{};
};

