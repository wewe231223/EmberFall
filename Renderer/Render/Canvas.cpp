#include "pch.h"
#include "Canvas.h"

Canvas::Canvas(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
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
		mTransform._11 = mRect.width;
		mTransform._22 = mRect.height;

		mTransform._31 = mRect.LTx;
		mTransform._32 = mRect.LTy;

		if (mSpritable) {
			mContext.UVTransform._11 = 1.f / static_cast<float>(mSpriteFrameInRow);
			mContext.UVTransform._22 = 1.f / static_cast<float>(mSpriteFrameInCol);

			mContext.UVTransform._13 = static_cast<float>(mSpriteCoord.first) * mContext.UVTransform._11;
			mContext.UVTransform._23 = static_cast<float>(mSpriteCoord.second) * mContext.UVTransform._22;
		}

		mContext.Transform = Transpose(Multifly(mTransform, mScreenTransform));
		mCanvas->AppendContext(mContext);
	}
}

void CanvasObject::ChangeImage(UINT imageIndex) {
	mImageIndex = imageIndex;
}

void CanvasObject::ChangeImage(UINT imageIndex, const std::pair<UINT, UINT>& imageWidthHeight, const std::pair<UINT, UINT>& imageUnit) {
	mImageIndex = imageIndex;
	mContext.ImageIndex = imageIndex;

	mImageWidthHeight = imageWidthHeight;
	mImageUnit = imageUnit;
	
	mSpritable = false;

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

void CanvasObject::AdvanceSpriteFrame() {
	if (mSpritable) {
		mSpriteCoord.first++;
		if (mSpriteCoord.first >= mSpriteFrameInRow) {
			mSpriteCoord.first -= mSpriteFrameInRow;
			mSpriteCoord.second += 1;
			if (mSpriteCoord.second >= mSpriteFrameInCol) {
				mSpriteCoord.second = 0;
			}
		}
	}
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
