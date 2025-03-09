#include "pch.h"
#include "StringRenderer.h"
#include "../Utility/Exceptions.h"

void StringRenderer::Initialize(ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandQueue> commandQueue, std::array<Texture, Config::BACKBUFFER_COUNT<size_t>>& backBuffers) {

	UINT d3d12DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	D2D1_FACTORY_OPTIONS d2dFactoryOptions = {};

#if defined(_DEBUG) || defined(DEBUG)
	d2dFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
	d3d12DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	ID3D11Device* d3d11Device{ nullptr };
	ID3D12CommandQueue* d3d12CommandQueues[] = { commandQueue.Get() };

	CheckHR(::D3D11On12CreateDevice(device.Get(), d3d12DeviceFlags, nullptr, 0, reinterpret_cast<IUnknown**>(d3d12CommandQueues), _countof(d3d12CommandQueues), 0, (ID3D11Device**)&d3d11Device, (ID3D11DeviceContext**)&mDeviceContext, nullptr));

	CheckHR(d3d11Device->QueryInterface(__uuidof(ID3D11On12Device), (void**)&mDevice));
	d3d11Device->Release();

#if defined(_DEBUG) || defined(DEBUG) 
	ID3D12InfoQueue* d3d12InfoQueue;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&d3d12InfoQueue)))) {
		D3D12_MESSAGE_SEVERITY d3d12Severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_MESSAGE_ID d3d12DenyIds[] = { D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE };

		D3D12_INFO_QUEUE_FILTER d3d12InfoQueueFilter = {};
		d3d12InfoQueueFilter.DenyList.NumSeverities = _countof(d3d12Severities);
		d3d12InfoQueueFilter.DenyList.pSeverityList = d3d12Severities;
		d3d12InfoQueueFilter.DenyList.NumIDs = _countof(d3d12DenyIds);
		d3d12InfoQueueFilter.DenyList.pIDList = d3d12DenyIds;

		d3d12InfoQueue->PushStorageFilter(&d3d12InfoQueueFilter);
	}
	d3d12InfoQueue->Release();
#endif

	IDXGIDevice* dxgiDevice{ nullptr };
	mDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);

	CheckHR(::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, (void**)&mFactory));
	CheckHR(mFactory->CreateDevice(dxgiDevice, (ID2D1Device2**)&mD2DDevice));
	CheckHR(mD2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, (ID2D1DeviceContext2**)&mD2DDeviceContext));

	mD2DDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);

	IDWriteFactory* writeFactory{ nullptr };
	CheckHR(::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&writeFactory));
	CheckHR(writeFactory->QueryInterface(IID_PPV_ARGS(mWriteFactory.GetAddressOf())));

	dxgiDevice->Release();

	D2D1_BITMAP_PROPERTIES1 bitmapProperties{ D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)) };

	for (UINT i = 0; i < Config::BACKBUFFER_COUNT<size_t>; i++) {
		D3D11_RESOURCE_FLAGS d3d11Flags{ D3D11_BIND_RENDER_TARGET };
		
		CheckHR(mDevice->CreateWrappedResource(backBuffers[i].GetResource().Get(), &d3d11Flags, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT, IID_PPV_ARGS(&mBackBuffers[i])));
		IDXGISurface* dxgiSurface{ nullptr };
		mBackBuffers[i]->QueryInterface(__uuidof(IDXGISurface), (void**)&dxgiSurface);
		
		CheckHR(mD2DDeviceContext->CreateBitmapFromDxgiSurface(dxgiSurface, &bitmapProperties, &mRenderTargets[i]));
		dxgiSurface->Release();
	}

	InitBrush();
	InitFonts(); 
}

void StringRenderer::Render(const std::wstring& str) {
	ID3D11Resource* renderTargets[] = { mBackBuffers[mCurrentFrameIndex].Get() };
	
	mD2DDeviceContext->SetTarget(mRenderTargets[mCurrentFrameIndex].Get());
	mDevice->AcquireWrappedResources(renderTargets, _countof(renderTargets));

	mD2DDeviceContext->BeginDraw();

	mD2DDeviceContext->DrawText(str.c_str(), static_cast<UINT32>(str.size()), mTestFormat.Get(), D2D1::RectF(0.0f, 0.0f, 800.0f, 600.0f), mBrushes[static_cast<size_t>(StringColor::red)].Get());

	CheckHR(mD2DDeviceContext->EndDraw());

	mDevice->ReleaseWrappedResources(renderTargets, _countof(renderTargets));
	mDeviceContext->Flush(); 

	mCurrentFrameIndex = (mCurrentFrameIndex + 1) % Config::BACKBUFFER_COUNT<size_t>;
}

/*
enum class StringColor : DWORD{
	red, green, blue, yellow, white, black, END
};
*/

void StringRenderer::InitBrush() {
	{
		ComPtr<ID2D1SolidColorBrush> brush{};
		CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &brush));
		mBrushes[static_cast<size_t>(StringColor::red)] = brush;
	}

	{
		ComPtr<ID2D1SolidColorBrush> brush{};
		CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), &brush));
		mBrushes[static_cast<size_t>(StringColor::green)] = brush;
	}

	{
		ComPtr<ID2D1SolidColorBrush> brush{};
		CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Blue), &brush));
		mBrushes[static_cast<size_t>(StringColor::blue)] = brush;
	}

	{
		ComPtr<ID2D1SolidColorBrush> brush{};
		CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow), &brush));
		mBrushes[static_cast<size_t>(StringColor::yellow)] = brush;
	}

	{
		ComPtr<ID2D1SolidColorBrush> brush{};
		CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &brush));
		mBrushes[static_cast<size_t>(StringColor::white)] = brush;
	}

	{
		ComPtr<ID2D1SolidColorBrush> brush{};
		CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &brush));
		mBrushes[static_cast<size_t>(StringColor::black)] = brush;
	}


}

void StringRenderer::InitFonts() {
	ComPtr<IDWriteFontSetBuilder1> fontSetBuilder{};
	CheckHR(mWriteFactory->CreateFontSetBuilder(&fontSetBuilder));

	std::filesystem::path fontPath{ L"Resources/Font/NotoSansKR-Regular-Hestia.otf" };

	ComPtr<IDWriteFontFile> fontFile{};
	CheckHR(mWriteFactory->CreateFontFileReference(fontPath.c_str(), nullptr, &fontFile));

	CheckHR(fontSetBuilder->AddFontFile(fontFile.Get()));

	ComPtr<IDWriteFontSet> fontSet{};
	CheckHR(fontSetBuilder->CreateFontSet(&fontSet));

	ComPtr<IDWriteFontCollection1> fontCollection{};
	CheckHR(mWriteFactory->CreateFontCollectionFromFontSet(fontSet.Get(), &fontCollection));

	CheckHR(mWriteFactory->CreateTextFormat(
		L"Noto Sans KR", 
		fontCollection.Get(), 
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL, 
		DWRITE_FONT_STRETCH_NORMAL, 20.0f,
		L"ko-kr", 
		&mTestFormat)
	);

}
