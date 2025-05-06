#include "pch.h"
#include "Canvas.h"
#include "../Game/System/Timer.h"

Canvas::Canvas(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, HWND renderWindow) {
	std::vector<DirectX::XMFLOAT2> vertices{
		{0.0f, 0.0f},  // Bottom-left
		{1.0f, 0.0f},  // Bottom-right
		{1.0f, 1.0f},  // Top-right
		{0.0f, 1.0f},  // Top-left
	};

	std::vector<UINT> indices{
		0, 1, 3,
		1, 2, 3
	};

	mVertexBuffer = DefaultBuffer(device, commandList, sizeof(DirectX::XMFLOAT2), 4, vertices.data());
	mVertexBufferView.BufferLocation = *mVertexBuffer.GPUBegin();
	mVertexBufferView.SizeInBytes = static_cast<UINT>(vertices.size() * sizeof(DirectX::XMFLOAT2));
	mVertexBufferView.StrideInBytes = sizeof(DirectX::XMFLOAT2);

	mIndexBuffer = DefaultBuffer(device, commandList, sizeof(UINT), 6, indices.data());
	mIndexBufferView.BufferLocation = *mIndexBuffer.GPUBegin();
	mIndexBufferView.SizeInBytes = static_cast<UINT>(indices.size() * sizeof(UINT));
	mIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	mIndexBufferView.SizeInBytes = static_cast<UINT>(indices.size() * sizeof(UINT));

	mContextBuffer = DefaultBuffer(device, sizeof(ModelContext2D), UI_ELEMENT_COUNT<size_t>);

	mShader = std::make_unique<UIShader>(); 
	mShader->CreateShader(device); 

	mContextPos = mContextBuffer.CPUBegin();

	mRenderWindowHandle = renderWindow; 
}

CanvasObject Canvas::CreateCanvasObject() {
	return CanvasObject(this);
}

void Canvas::AppendContext(const ModelContext2D& context) {
	std::memcpy(*mContextPos, &context, sizeof(ModelContext2D));
	mContextPos++;
}

void Canvas::Render(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex) {
	mContextBuffer.Upload(commandList, mContextBuffer.CPUBegin(), mContextPos);

	mShader->SetGPassShader(commandList);
	commandList->SetGraphicsRootShaderResourceView(0, *mContextBuffer.GPUBegin());
	commandList->SetGraphicsRootDescriptorTable(1, tex);

	commandList->IASetVertexBuffers(0, 1, &mVertexBufferView);
	commandList->IASetIndexBuffer(&mIndexBufferView);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT instanceCount = static_cast<UINT>(mContextPos - mContextBuffer.CPUBegin()) / sizeof(ModelContext2D);
	commandList->DrawIndexedInstanced(6, instanceCount, 0, 0, 0);

	mContextPos = mContextBuffer.CPUBegin();
}

RECT Canvas::GetClientRect() {
	RECT result;
	::GetClientRect(mRenderWindowHandle, &result);

	return result; 
}

CanvasObject::CanvasObject(Canvas* canvas) {
	mCanvas = canvas;
	
	mScreenTransform = DirectX::XMFLOAT3X3{
	2.f / Config::WINDOW_WIDTH<float>				,0.0f													,0.f,
	0.0f											,-2.f / Config::WINDOW_HEIGHT<float> 					,0.f,
	-1.f											,1.f													,1.f
	};
}

void CanvasObject::Update() {
	if (mActive) {
		mTransform._11 = mRect.width * cosf(mtheta);
		mTransform._12 = mRect.height * -sinf(mtheta);

		mTransform._21 = mRect.width * sinf(mtheta);
		mTransform._22 = mRect.height * cosf(mtheta);

		mTransform._31 = mRect.LTx;
		mTransform._32 = mRect.LTy;

		if (mSpritable) {
			UINT spriteIndex = GetSpriteIndex();

			float spriteWidth = 1.f / static_cast<float>(mSpriteFrameInRow);
			float spriteHeight = 1.f / static_cast<float>(mSpriteFrameInCol);

			mContext.UVTransform = DirectX::XMFLOAT3X3{
				spriteWidth,	0.f,				(spriteIndex % mSpriteFrameInRow) * spriteWidth,
				0.f,			spriteHeight,		(spriteIndex / mSpriteFrameInCol) * spriteHeight,
				0.f,			0.f,				1.f
			};

		}

		mContext.Transform = Transpose(Multifly(mTransform, mScreenTransform));
		mCanvas->AppendContext(mContext);
	}
}

void CanvasObject::ChangeImage(UINT imageIndex) {
	mImageIndex = imageIndex;
	mContext.ImageIndex = imageIndex;
	mSpritable = false;
}

void CanvasObject::ChangeImage(UINT imageIndex, UINT spriteframeRow, UINT spriteframeCol, float spriteDuration) {
	mImageIndex = imageIndex;
	mContext.ImageIndex = imageIndex;

	mSpritable = true;
	mSpriteFrameInRow = spriteframeRow;
	mSpriteFrameInCol = spriteframeCol;
	mSpriteDuration = spriteDuration;
}

void CanvasObject::SetActive(bool active) {
	mActive = active;
}

bool CanvasObject::GetActive() const {
	return mActive;
}

CanvasRect& CanvasObject::GetRect() {
	return mRect;
}

void CanvasObject::SetGreyScale(float scale) {
	mContext.GreyScale = scale;
}

DirectX::XMFLOAT3X3 CanvasObject::Multifly(const DirectX::XMFLOAT3X3& lhs, const DirectX::XMFLOAT3X3& rhs) const {
	// XMFLOAT3X3를 XMMATRIX로 변환
	DirectX::XMMATRIX matrix1 = DirectX::XMLoadFloat3x3(&lhs);
	DirectX::XMMATRIX matrix2 = DirectX::XMLoadFloat3x3(&rhs);

	// 행렬 곱셈
	DirectX::XMMATRIX resultMatrix = DirectX::XMMatrixMultiply(matrix1, matrix2);

	// 결과를 XMFLOAT3X3로 변환
	DirectX::XMFLOAT3X3 result;
	DirectX::XMStoreFloat3x3(&result, resultMatrix);

	return result;
}

DirectX::XMFLOAT3X3 CanvasObject::Transpose(const DirectX::XMFLOAT3X3& mat) const {
	DirectX::XMMATRIX matrix = DirectX::XMLoadFloat3x3(&mat);
	DirectX::XMMATRIX matrix1 = DirectX::XMMatrixTranspose(matrix);

	DirectX::XMFLOAT3X3 result;
	DirectX::XMStoreFloat3x3(&result, matrix1);

	return result;
}

UINT CanvasObject::GetSpriteIndex() {

	UINT ms = Time.GetTimeSinceStarted<UINT ,std::chrono::milliseconds>();

	float framedurationMS = (1000.f * mSpriteDuration) / (mSpriteFrameInRow * mSpriteFrameInCol);
	return static_cast<UINT>(Time.GetTimeSinceStarted<UINT, std::chrono::milliseconds>() / framedurationMS) % (mSpriteFrameInCol * mSpriteFrameInRow); 
}
