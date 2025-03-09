#pragma once 
#include <array>
#include "../Resource/Texture.h"
#include "../Utility/DirectXInclude.h"
#include "../Config/Config.h"
#include "../Utility/Defines.h"
class StringRenderer {
public:
	StringRenderer() = default;
	~StringRenderer() = default;

public:
	void Initialize(ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandQueue> commandQueue, std::array<Texture, Config::BACKBUFFER_COUNT<size_t>>& backBuffers);
	void Render(const std::wstring& str);
private:
	void InitBrush();
	void InitFonts(); 
private:
	ComPtr<ID3D11DeviceContext> mDeviceContext{};
	ComPtr<ID3D11On12Device> mDevice{};
	ComPtr<IDWriteFactory7> mWriteFactory{};

	ComPtr<ID2D1Factory3> mFactory{};
	ComPtr<ID2D1Device2> mD2DDevice{};

	ComPtr<ID2D1DeviceContext2> mD2DDeviceContext{};

	std::array<ComPtr<ID3D11Resource>, Config::BACKBUFFER_COUNT<size_t>> mBackBuffers{}; 
	std::array<ComPtr<ID2D1Bitmap1>, Config::BACKBUFFER_COUNT<size_t>> mRenderTargets{};
	UINT mCurrentFrameIndex{ 0 };
	
	std::array<ComPtr<ID2D1SolidColorBrush>, static_cast<size_t>(StringColor::END)> mBrushes{};

	ComPtr<IDWriteTextFormat> mTestFormat{};
};