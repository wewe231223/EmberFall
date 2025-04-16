#pragma once 
#include "../Utility/Defines.h"
#include "../Renderer/Resource/DefaultBuffer.h"
#include "../Renderer/Core/Shader.h"

class Canvas {
	template<typename T>
	static constexpr T UI_ELEMENT_COUNT = static_cast<T>(100); 
public:
	Canvas() = default;
	Canvas(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList);

	Canvas(const Canvas&) = default;
	Canvas& operator=(const Canvas&) = default;

	Canvas(Canvas&&) = default;
	Canvas& operator=(Canvas&&) = default;

public:
	void AppendContext(const ModelContext2D& context);
	void Render(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex); 
private:
	DefaultBuffer mVertexBuffer{};
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView{};
	
	DefaultBuffer mIndexBuffer{};
	D3D12_INDEX_BUFFER_VIEW mIndexBufferView{};

	DefaultBuffer mConstantBuffer{};

	std::unique_ptr<GraphicsShaderBase> mShader{ nullptr };
};

class CanvasObject {
public:
	CanvasObject() = default;
	CanvasObject(Canvas* canvas) : mCanvas(canvas) {}
	
	CanvasObject(const CanvasObject&) = delete;
	CanvasObject& operator=(const CanvasObject&) = delete;
	
	CanvasObject(CanvasObject&&) = default;
	CanvasObject& operator=(CanvasObject&&) = default;

	virtual ~CanvasObject() = default;
public:
	virtual void Update() = 0;
	virtual void Render() = 0;
private:
	Canvas* mCanvas{ nullptr }; 
	ModelContext2D mModelContext{};
};