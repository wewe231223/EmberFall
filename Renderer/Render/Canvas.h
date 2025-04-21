#pragma once 
#include "../Utility/Defines.h"
#include "../Renderer/Resource/DefaultBuffer.h"
#include "../Renderer/Core/Shader.h"

class Canvas;

struct CanvasRect {
	float LTx{ 0.f };
	float LTy{ 0.f };
	float width{ 0.f };
	float height{ 0.f };
};

class CanvasObject {
public:
	CanvasObject() = default;
	CanvasObject(Canvas* canvas);

	CanvasObject(const CanvasObject&) = default;
	CanvasObject& operator=(const CanvasObject&) = default;

	CanvasObject(CanvasObject&&) = default;
	CanvasObject& operator=(CanvasObject&&) = default;

	virtual ~CanvasObject() = default;
public:
	void Update();

	void ChangeImage(UINT imageIndex);
	void ChangeImage(UINT imageIndex, UINT spriteframeRow, UINT spriteframeCol, float spriteDuration);

	void SetActive(bool active);
	bool GetActive() const;

	CanvasRect& GetRect();
private:
	DirectX::XMFLOAT3X3 Multifly(const DirectX::XMFLOAT3X3& lhs, const DirectX::XMFLOAT3X3& rhs) const;
	DirectX::XMFLOAT3X3 Transpose(const DirectX::XMFLOAT3X3& mat) const;

	UINT GetSpriteIndex(); 
private:
	Canvas* mCanvas{ nullptr };
	
	bool mActive{ true };

	ModelContext2D mContext{};
	UINT mImageIndex{ 0 };

	CanvasRect mRect{};
	float mtheta{ 0.f };

	DirectX::XMFLOAT3X3 mTransform{ 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f };
	DirectX::XMFLOAT3X3 mScreenTransform{ 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f };

#pragma region UISpritable
	bool mSpritable{ false };

	UINT mSpriteFrameInRow{ 0 };
	UINT mSpriteFrameInCol{ 0 };

	float mSpriteDuration{ 0.f };
#pragma endregion 
};

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
	CanvasObject CreateCanvasObject();

	void AppendContext(const ModelContext2D& context);
	void Render(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex); 

private:
	DefaultBuffer mVertexBuffer{};
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView{};
	
	DefaultBuffer mIndexBuffer{};
	D3D12_INDEX_BUFFER_VIEW mIndexBufferView{};

	DefaultBuffer mContextBuffer{};
	DefaultBufferCPUIterator mContextPos{};

	std::unique_ptr<GraphicsShaderBase> mShader{ nullptr };
};
