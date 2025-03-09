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
}

void StringRenderer::LoadExternalFont(const std::string& fontName, const std::filesystem::path& path, const std::wstring& fontFamilyName, const std::wstring& locale, DWRITE_FONT_WEIGHT weight, DWRITE_FONT_STYLE style, DWRITE_FONT_STRETCH stretch, float size, DWRITE_TEXT_ALIGNMENT alignment, DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment) {
    ComPtr<IDWriteFontSetBuilder1> fontSetBuilder{};
    CheckHR(mWriteFactory->CreateFontSetBuilder(&fontSetBuilder));

    ComPtr<IDWriteFontFile> fontFile{};
    CheckHR(mWriteFactory->CreateFontFileReference(path.c_str(), nullptr, &fontFile));

    CheckHR(fontSetBuilder->AddFontFile(fontFile.Get()));

    ComPtr<IDWriteFontSet> fontSet{};
    CheckHR(fontSetBuilder->CreateFontSet(&fontSet));

    ComPtr<IDWriteFontCollection1> fontCollection{};
    CheckHR(mWriteFactory->CreateFontCollectionFromFontSet(fontSet.Get(), &fontCollection));

    ComPtr<IDWriteTextFormat> textFormat{};
    CheckHR(mWriteFactory->CreateTextFormat(
        fontFamilyName.c_str(),
        nullptr,
        weight,
        style,
        stretch,
        size,
        locale.c_str(),
        &textFormat
    ));

	textFormat->SetTextAlignment(alignment);
	textFormat->SetParagraphAlignment(paragraphAlignment);

	mFonts[fontName] = textFormat;
}



void StringRenderer::LoadSystemFont(const std::string& fontName, const std::wstring& fontFamilyName, const std::wstring& locale, DWRITE_FONT_WEIGHT weight, DWRITE_FONT_STYLE style, DWRITE_FONT_STRETCH stretch, float size, DWRITE_TEXT_ALIGNMENT alignment, DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment) {
	ComPtr<IDWriteTextFormat> textFormat{};

	CheckHR(mWriteFactory->CreateTextFormat(
		fontFamilyName.c_str(),
		nullptr,
		weight,
		style,
		stretch,
		size,
		locale.c_str(),
		&textFormat
	));

	textFormat->SetTextAlignment(alignment);
	textFormat->SetParagraphAlignment(paragraphAlignment);

	mFonts[fontName] = textFormat;
}

IDWriteTextFormat* StringRenderer::GetFont(const std::string& fontName) {
	return mFonts[fontName].Get();
}

void StringRenderer::Render() {
	ID3D11Resource* renderTargets[] = { mBackBuffers[mCurrentFrameIndex].Get() };
	
	mD2DDeviceContext->SetTarget(mRenderTargets[mCurrentFrameIndex].Get());
	mDevice->AcquireWrappedResources(renderTargets, _countof(renderTargets));

	mD2DDeviceContext->BeginDraw();

    // 	mD2DDeviceContext->DrawText(str.c_str(), static_cast<UINT32>(str.size()), mTestFormat.Get(), D2D1::RectF(0.0f, 0.0f, 800.0f, 600.0f), mBrushes[static_cast<size_t>(StringColor::red)].Get());


    for (auto& text : TextBlockManager::GetInstance()) {

        auto& rect = text.GetRect();

        mD2DDeviceContext->DrawText(
            text.GetText().c_str(),
			static_cast<UINT32>(text.GetText().size()),
            text.GetFont(),
            &rect,
            mBrushes[static_cast<size_t>(text.GetColor())].Get()
        );
    }



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
#pragma region InitBrush
void StringRenderer::InitBrush() {
    // AliceBlue
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::AliceBlue), &brush));
        mBrushes[static_cast<size_t>(StringColor::AliceBlue)] = brush;
    }

    // AntiqueWhite
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::AntiqueWhite), &brush));
        mBrushes[static_cast<size_t>(StringColor::AntiqueWhite)] = brush;
    }

    // Aqua
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Aqua), &brush));
        mBrushes[static_cast<size_t>(StringColor::Aqua)] = brush;
    }

    // Aquamarine
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Aquamarine), &brush));
        mBrushes[static_cast<size_t>(StringColor::Aquamarine)] = brush;
    }

    // Azure
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Azure), &brush));
        mBrushes[static_cast<size_t>(StringColor::Azure)] = brush;
    }

    // Beige
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Beige), &brush));
        mBrushes[static_cast<size_t>(StringColor::Beige)] = brush;
    }

    // Bisque
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Bisque), &brush));
        mBrushes[static_cast<size_t>(StringColor::Bisque)] = brush;
    }

    // Black
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &brush));
        mBrushes[static_cast<size_t>(StringColor::Black)] = brush;
    }

    // BlanchedAlmond
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::BlanchedAlmond), &brush));
        mBrushes[static_cast<size_t>(StringColor::BlanchedAlmond)] = brush;
    }

    // Blue
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Blue), &brush));
        mBrushes[static_cast<size_t>(StringColor::Blue)] = brush;
    }

    // BlueViolet
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::BlueViolet), &brush));
        mBrushes[static_cast<size_t>(StringColor::BlueViolet)] = brush;
    }

    // Brown
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Brown), &brush));
        mBrushes[static_cast<size_t>(StringColor::Brown)] = brush;
    }

    // BurlyWood
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::BurlyWood), &brush));
        mBrushes[static_cast<size_t>(StringColor::BurlyWood)] = brush;
    }

    // CadetBlue
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::CadetBlue), &brush));
        mBrushes[static_cast<size_t>(StringColor::CadetBlue)] = brush;
    }

    // Chartreuse
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Chartreuse), &brush));
        mBrushes[static_cast<size_t>(StringColor::Chartreuse)] = brush;
    }

    // Chocolate
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Chocolate), &brush));
        mBrushes[static_cast<size_t>(StringColor::Chocolate)] = brush;
    }

    // Coral
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Coral), &brush));
        mBrushes[static_cast<size_t>(StringColor::Coral)] = brush;
    }

    // CornflowerBlue
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::CornflowerBlue), &brush));
        mBrushes[static_cast<size_t>(StringColor::CornflowerBlue)] = brush;
    }

    // Cornsilk
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Cornsilk), &brush));
        mBrushes[static_cast<size_t>(StringColor::Cornsilk)] = brush;
    }

    // Crimson
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Crimson), &brush));
        mBrushes[static_cast<size_t>(StringColor::Crimson)] = brush;
    }

    // Cyan
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Cyan), &brush));
        mBrushes[static_cast<size_t>(StringColor::Cyan)] = brush;
    }

    // DarkBlue
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkBlue), &brush));
        mBrushes[static_cast<size_t>(StringColor::DarkBlue)] = brush;
    }

    // DarkCyan
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkCyan), &brush));
        mBrushes[static_cast<size_t>(StringColor::DarkCyan)] = brush;
    }

    // DarkGoldenrod
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkGoldenrod), &brush));
        mBrushes[static_cast<size_t>(StringColor::DarkGoldenrod)] = brush;
    }

    // DarkGray
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkGray), &brush));
        mBrushes[static_cast<size_t>(StringColor::DarkGray)] = brush;
    }

    // DarkGreen
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkGreen), &brush));
        mBrushes[static_cast<size_t>(StringColor::DarkGreen)] = brush;
    }

    // DarkKhaki
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkKhaki), &brush));
        mBrushes[static_cast<size_t>(StringColor::DarkKhaki)] = brush;
    }

    // DarkMagenta
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkMagenta), &brush));
        mBrushes[static_cast<size_t>(StringColor::DarkMagenta)] = brush;
    }

    // DarkOliveGreen
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkOliveGreen), &brush));
        mBrushes[static_cast<size_t>(StringColor::DarkOliveGreen)] = brush;
    }

    // DarkOrange
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkOrange), &brush));
        mBrushes[static_cast<size_t>(StringColor::DarkOrange)] = brush;
    }

    // DarkOrchid
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkOrchid), &brush));
        mBrushes[static_cast<size_t>(StringColor::DarkOrchid)] = brush;
    }

    // DarkRed
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkRed), &brush));
        mBrushes[static_cast<size_t>(StringColor::DarkRed)] = brush;
    }

    // DarkSalmon
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkSalmon), &brush));
        mBrushes[static_cast<size_t>(StringColor::DarkSalmon)] = brush;
    }

    // DarkSeaGreen
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkSeaGreen), &brush));
        mBrushes[static_cast<size_t>(StringColor::DarkSeaGreen)] = brush;
    }

    // DarkSlateBlue
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkSlateBlue), &brush));
        mBrushes[static_cast<size_t>(StringColor::DarkSlateBlue)] = brush;
    }

    // DarkSlateGray
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkSlateGray), &brush));
        mBrushes[static_cast<size_t>(StringColor::DarkSlateGray)] = brush;
    }

    // DarkTurquoise
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkTurquoise), &brush));
        mBrushes[static_cast<size_t>(StringColor::DarkTurquoise)] = brush;
    }

    // DarkViolet
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkViolet), &brush));
        mBrushes[static_cast<size_t>(StringColor::DarkViolet)] = brush;
    }

    // DeepPink
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DeepPink), &brush));
        mBrushes[static_cast<size_t>(StringColor::DeepPink)] = brush;
    }

    // DeepSkyBlue
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DeepSkyBlue), &brush));
        mBrushes[static_cast<size_t>(StringColor::DeepSkyBlue)] = brush;
    }

    // DimGray
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DimGray), &brush));
        mBrushes[static_cast<size_t>(StringColor::DimGray)] = brush;
    }

    // DodgerBlue
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DodgerBlue), &brush));
        mBrushes[static_cast<size_t>(StringColor::DodgerBlue)] = brush;
    }

    // Firebrick
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Firebrick), &brush));
        mBrushes[static_cast<size_t>(StringColor::Firebrick)] = brush;
    }

    // FloralWhite
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::FloralWhite), &brush));
        mBrushes[static_cast<size_t>(StringColor::FloralWhite)] = brush;
    }

    // ForestGreen
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::ForestGreen), &brush));
        mBrushes[static_cast<size_t>(StringColor::ForestGreen)] = brush;
    }

    // Fuchsia
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Fuchsia), &brush));
        mBrushes[static_cast<size_t>(StringColor::Fuchsia)] = brush;
    }

    // Gainsboro
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gainsboro), &brush));
        mBrushes[static_cast<size_t>(StringColor::Gainsboro)] = brush;
    }

    // GhostWhite
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::GhostWhite), &brush));
        mBrushes[static_cast<size_t>(StringColor::GhostWhite)] = brush;
    }

    // Gold
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gold), &brush));
        mBrushes[static_cast<size_t>(StringColor::Gold)] = brush;
    }

    // Goldenrod
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Goldenrod), &brush));
        mBrushes[static_cast<size_t>(StringColor::Goldenrod)] = brush;
    }

    // Gray
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray), &brush));
        mBrushes[static_cast<size_t>(StringColor::Gray)] = brush;
    }

    // Green
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), &brush));
        mBrushes[static_cast<size_t>(StringColor::Green)] = brush;
    }

    // GreenYellow
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::GreenYellow), &brush));
        mBrushes[static_cast<size_t>(StringColor::GreenYellow)] = brush;
    }

    // Honeydew
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Honeydew), &brush));
        mBrushes[static_cast<size_t>(StringColor::Honeydew)] = brush;
    }

    // HotPink
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::HotPink), &brush));
        mBrushes[static_cast<size_t>(StringColor::HotPink)] = brush;
    }

    // IndianRed
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::IndianRed), &brush));
        mBrushes[static_cast<size_t>(StringColor::IndianRed)] = brush;
    }

    // Indigo
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Indigo), &brush));
        mBrushes[static_cast<size_t>(StringColor::Indigo)] = brush;
    }

    // Ivory
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Ivory), &brush));
        mBrushes[static_cast<size_t>(StringColor::Ivory)] = brush;
    }

    // Khaki
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Khaki), &brush));
        mBrushes[static_cast<size_t>(StringColor::Khaki)] = brush;
    }

    // Lavender
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Lavender), &brush));
        mBrushes[static_cast<size_t>(StringColor::Lavender)] = brush;
    }

    // LavenderBlush
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LavenderBlush), &brush));
        mBrushes[static_cast<size_t>(StringColor::LavenderBlush)] = brush;
    }

    // LawnGreen
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LawnGreen), &brush));
        mBrushes[static_cast<size_t>(StringColor::LawnGreen)] = brush;
    }

    // LemonChiffon
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LemonChiffon), &brush));
        mBrushes[static_cast<size_t>(StringColor::LemonChiffon)] = brush;
    }

    // LightBlue
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightBlue), &brush));
        mBrushes[static_cast<size_t>(StringColor::LightBlue)] = brush;
    }

    // LightCoral
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightCoral), &brush));
        mBrushes[static_cast<size_t>(StringColor::LightCoral)] = brush;
    }

    // LightCyan
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightCyan), &brush));
        mBrushes[static_cast<size_t>(StringColor::LightCyan)] = brush;
    }

    // LightGoldenrodYellow
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightGoldenrodYellow), &brush));
        mBrushes[static_cast<size_t>(StringColor::LightGoldenrodYellow)] = brush;
    }

    // LightGreen
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightGreen), &brush));
        mBrushes[static_cast<size_t>(StringColor::LightGreen)] = brush;
    }

    // LightGray
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightGray), &brush));
        mBrushes[static_cast<size_t>(StringColor::LightGray)] = brush;
    }

    // LightPink
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightPink), &brush));
        mBrushes[static_cast<size_t>(StringColor::LightPink)] = brush;
    }

    // LightSalmon
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightSalmon), &brush));
        mBrushes[static_cast<size_t>(StringColor::LightSalmon)] = brush;
    }

    // LightSeaGreen
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightSeaGreen), &brush));
        mBrushes[static_cast<size_t>(StringColor::LightSeaGreen)] = brush;
    }

    // LightSkyBlue
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightSkyBlue), &brush));
        mBrushes[static_cast<size_t>(StringColor::LightSkyBlue)] = brush;
    }

    // LightSlateGray
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightSlateGray), &brush));
        mBrushes[static_cast<size_t>(StringColor::LightSlateGray)] = brush;
    }

    // LightSteelBlue
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightSteelBlue), &brush));
        mBrushes[static_cast<size_t>(StringColor::LightSteelBlue)] = brush;
    }

    // LightYellow
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightYellow), &brush));
        mBrushes[static_cast<size_t>(StringColor::LightYellow)] = brush;
    }

    // Lime
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Lime), &brush));
        mBrushes[static_cast<size_t>(StringColor::Lime)] = brush;
    }

    // LimeGreen
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LimeGreen), &brush));
        mBrushes[static_cast<size_t>(StringColor::LimeGreen)] = brush;
    }

    // Linen
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Linen), &brush));
        mBrushes[static_cast<size_t>(StringColor::Linen)] = brush;
    }

    // Magenta
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Magenta), &brush));
        mBrushes[static_cast<size_t>(StringColor::Magenta)] = brush;
    }

    // Maroon
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Maroon), &brush));
        mBrushes[static_cast<size_t>(StringColor::Maroon)] = brush;
    }

    // MediumAquamarine
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::MediumAquamarine), &brush));
        mBrushes[static_cast<size_t>(StringColor::MediumAquamarine)] = brush;
    }

    // MediumBlue
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::MediumBlue), &brush));
        mBrushes[static_cast<size_t>(StringColor::MediumBlue)] = brush;
    }

    // MediumOrchid
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::MediumOrchid), &brush));
        mBrushes[static_cast<size_t>(StringColor::MediumOrchid)] = brush;
    }

    // MediumPurple
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::MediumPurple), &brush));
        mBrushes[static_cast<size_t>(StringColor::MediumPurple)] = brush;
    }

    // MediumSeaGreen
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::MediumSeaGreen), &brush));
        mBrushes[static_cast<size_t>(StringColor::MediumSeaGreen)] = brush;
    }

    // MediumSlateBlue
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::MediumSlateBlue), &brush));
        mBrushes[static_cast<size_t>(StringColor::MediumSlateBlue)] = brush;
    }

    // MediumSpringGreen
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::MediumSpringGreen), &brush));
        mBrushes[static_cast<size_t>(StringColor::MediumSpringGreen)] = brush;
    }

    // MediumTurquoise
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::MediumTurquoise), &brush));
        mBrushes[static_cast<size_t>(StringColor::MediumTurquoise)] = brush;
    }

    // MediumVioletRed
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::MediumVioletRed), &brush));
        mBrushes[static_cast<size_t>(StringColor::MediumVioletRed)] = brush;
    }

    // MidnightBlue
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::MidnightBlue), &brush));
        mBrushes[static_cast<size_t>(StringColor::MidnightBlue)] = brush;
    }

    // MintCream
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::MintCream), &brush));
        mBrushes[static_cast<size_t>(StringColor::MintCream)] = brush;
    }

    // MistyRose
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::MistyRose), &brush));
        mBrushes[static_cast<size_t>(StringColor::MistyRose)] = brush;
    }

    // Moccasin
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Moccasin), &brush));
        mBrushes[static_cast<size_t>(StringColor::Moccasin)] = brush;
    }

    // NavajoWhite
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::NavajoWhite), &brush));
        mBrushes[static_cast<size_t>(StringColor::NavajoWhite)] = brush;
    }

    // Navy
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Navy), &brush));
        mBrushes[static_cast<size_t>(StringColor::Navy)] = brush;
    }

    // OldLace
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::OldLace), &brush));
        mBrushes[static_cast<size_t>(StringColor::OldLace)] = brush;
    }

    // Olive
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Olive), &brush));
        mBrushes[static_cast<size_t>(StringColor::Olive)] = brush;
    }

    // OliveDrab
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::OliveDrab), &brush));
        mBrushes[static_cast<size_t>(StringColor::OliveDrab)] = brush;
    }

    // Orange
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Orange), &brush));
        mBrushes[static_cast<size_t>(StringColor::Orange)] = brush;
    }

    // OrangeRed
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::OrangeRed), &brush));
        mBrushes[static_cast<size_t>(StringColor::OrangeRed)] = brush;
    }

    // Orchid
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Orchid), &brush));
        mBrushes[static_cast<size_t>(StringColor::Orchid)] = brush;
    }

    // PaleGoldenrod
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::PaleGoldenrod), &brush));
        mBrushes[static_cast<size_t>(StringColor::PaleGoldenrod)] = brush;
    }

    // PaleGreen
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::PaleGreen), &brush));
        mBrushes[static_cast<size_t>(StringColor::PaleGreen)] = brush;
    }

    // PaleTurquoise
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::PaleTurquoise), &brush));
        mBrushes[static_cast<size_t>(StringColor::PaleTurquoise)] = brush;
    }

    // PaleVioletRed
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::PaleVioletRed), &brush));
        mBrushes[static_cast<size_t>(StringColor::PaleVioletRed)] = brush;
    }

    // PapayaWhip
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::PapayaWhip), &brush));
        mBrushes[static_cast<size_t>(StringColor::PapayaWhip)] = brush;
    }

    // PeachPuff
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::PeachPuff), &brush));
        mBrushes[static_cast<size_t>(StringColor::PeachPuff)] = brush;
    }

    // Peru
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Peru), &brush));
        mBrushes[static_cast<size_t>(StringColor::Peru)] = brush;
    }

    // Pink
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Pink), &brush));
        mBrushes[static_cast<size_t>(StringColor::Pink)] = brush;
    }

    // Plum
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Plum), &brush));
        mBrushes[static_cast<size_t>(StringColor::Plum)] = brush;
    }

    // PowderBlue
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::PowderBlue), &brush));
        mBrushes[static_cast<size_t>(StringColor::PowderBlue)] = brush;
    }

    // Purple
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Purple), &brush));
        mBrushes[static_cast<size_t>(StringColor::Purple)] = brush;
    }

    // Red
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &brush));
        mBrushes[static_cast<size_t>(StringColor::Red)] = brush;
    }

    // RosyBrown
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::RosyBrown), &brush));
        mBrushes[static_cast<size_t>(StringColor::RosyBrown)] = brush;
    }

    // RoyalBlue
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::RoyalBlue), &brush));
        mBrushes[static_cast<size_t>(StringColor::RoyalBlue)] = brush;
    }

    // SaddleBrown
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::SaddleBrown), &brush));
        mBrushes[static_cast<size_t>(StringColor::SaddleBrown)] = brush;
    }

    // Salmon
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Salmon), &brush));
        mBrushes[static_cast<size_t>(StringColor::Salmon)] = brush;
    }

    // SandyBrown
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::SandyBrown), &brush));
        mBrushes[static_cast<size_t>(StringColor::SandyBrown)] = brush;
    }

    // SeaGreen
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::SeaGreen), &brush));
        mBrushes[static_cast<size_t>(StringColor::SeaGreen)] = brush;
    }

    // Seashell
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::SeaShell), &brush));
        mBrushes[static_cast<size_t>(StringColor::Seashell)] = brush;
    }

    // Sienna
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Sienna), &brush));
        mBrushes[static_cast<size_t>(StringColor::Sienna)] = brush;
    }

    // Silver
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Silver), &brush));
        mBrushes[static_cast<size_t>(StringColor::Silver)] = brush;
    }

    // SkyBlue
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::SkyBlue), &brush));
        mBrushes[static_cast<size_t>(StringColor::SkyBlue)] = brush;
    }

    // SlateBlue
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::SlateBlue), &brush));
        mBrushes[static_cast<size_t>(StringColor::SlateBlue)] = brush;
    }

    // SlateGray
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::SlateGray), &brush));
        mBrushes[static_cast<size_t>(StringColor::SlateGray)] = brush;
    }

    // Snow
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Snow), &brush));
        mBrushes[static_cast<size_t>(StringColor::Snow)] = brush;
    }

    // SpringGreen
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::SpringGreen), &brush));
        mBrushes[static_cast<size_t>(StringColor::SpringGreen)] = brush;
    }

    // SteelBlue
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::SteelBlue), &brush));
        mBrushes[static_cast<size_t>(StringColor::SteelBlue)] = brush;
    }

    // Tan
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Tan), &brush));
        mBrushes[static_cast<size_t>(StringColor::Tan)] = brush;
    }

    // Teal
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Teal), &brush));
        mBrushes[static_cast<size_t>(StringColor::Teal)] = brush;
    }

    // Thistle
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Thistle), &brush));
        mBrushes[static_cast<size_t>(StringColor::Thistle)] = brush;
    }

    // Tomato
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Tomato), &brush));
        mBrushes[static_cast<size_t>(StringColor::Tomato)] = brush;
    }

    // Turquoise
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Turquoise), &brush));
        mBrushes[static_cast<size_t>(StringColor::Turquoise)] = brush;
    }

    // Violet
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Violet), &brush));
        mBrushes[static_cast<size_t>(StringColor::Violet)] = brush;
    }

    // Wheat
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Wheat), &brush));
        mBrushes[static_cast<size_t>(StringColor::Wheat)] = brush;
    }

    // White
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &brush));
        mBrushes[static_cast<size_t>(StringColor::White)] = brush;
    }

    // WhiteSmoke
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::WhiteSmoke), &brush));
        mBrushes[static_cast<size_t>(StringColor::WhiteSmoke)] = brush;
    }

    // Yellow
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow), &brush));
        mBrushes[static_cast<size_t>(StringColor::Yellow)] = brush;
    }

    // YellowGreen
    {
        ComPtr<ID2D1SolidColorBrush> brush{};
        CheckHR(mD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::YellowGreen), &brush));
        mBrushes[static_cast<size_t>(StringColor::YellowGreen)] = brush;
    }
}
#pragma endregion


//void StringRenderer::InitFonts() {
//	ComPtr<IDWriteFontSetBuilder1> fontSetBuilder{};
//	CheckHR(mWriteFactory->CreateFontSetBuilder(&fontSetBuilder));
//
//	std::filesystem::path fontPath{ L"Resources/Font/NotoSansKR-Regular-Hestia.otf" };
//
//	ComPtr<IDWriteFontFile> fontFile{};
//	CheckHR(mWriteFactory->CreateFontFileReference(fontPath.c_str(), nullptr, &fontFile));
//
//	CheckHR(fontSetBuilder->AddFontFile(fontFile.Get()));
//
//	ComPtr<IDWriteFontSet> fontSet{};
//	CheckHR(fontSetBuilder->CreateFontSet(&fontSet));
//
//	ComPtr<IDWriteFontCollection1> fontCollection{};
//	CheckHR(mWriteFactory->CreateFontCollectionFromFontSet(fontSet.Get(), &fontCollection));
//
//	CheckHR(mWriteFactory->CreateTextFormat(
//		L"Noto Sans KR", 
//		fontCollection.Get(), 
//		DWRITE_FONT_WEIGHT_NORMAL,
//        DWRITE_FONT_STYLE_NORMAL,
//		DWRITE_FONT_STRETCH_NORMAL, 20.0f,
//		L"ko-kr", 
//		&mTestFormat)
//	);
//
//}

TextBlock::TextBlock(StringRenderer* stringRenderer, const std::wstring& text, const D2D1_RECT_F& rect, const StringColor& color, IDWriteTextFormat* font) {
	mStringRenderer = stringRenderer;
	mText = text;
	mRect = rect;
	mColor = color;
	mFont = font;
}

TextBlock::TextBlock(const std::wstring& text, const D2D1_RECT_F& rect, const StringColor& color, const std::string& font) {
	mText = text;
	mRect = rect;
	mColor = color;
	mInitialFontName = font;
}

void TextBlock::SetRenderer(StringRenderer* stringRenderer) {
    if (mStringRenderer == nullptr) {
        mStringRenderer = stringRenderer;
    }
}

const std::wstring& TextBlock::GetText() const {
	return mText;
}

const D2D1_RECT_F& TextBlock::GetRect() const {
	return mRect;
}

const StringColor& TextBlock::GetColor() const {
	return mColor;
}

IDWriteTextFormat* TextBlock::GetFont() const {
    return mFont;
}

void TextBlock::LoadFont() {
    if (mFont == nullptr) {
        mFont = mStringRenderer->GetFont(mInitialFontName);
    }
}

void TextBlock::ChangeFont(const std::string& fontName) {
    if (mStringRenderer != nullptr) {
        mFont = mStringRenderer->GetFont(fontName); 
    }
}

TextBlockManager::TextBlockManager() {
	mTextBlocks.reserve(1024);
}

TextBlockManager& TextBlockManager::GetInstance() {
	static TextBlockManager instance{};
	return instance;
}

void TextBlockManager::Initialize(StringRenderer* stringRenderer) {
	mStringRenderer = stringRenderer;

	for (auto& textblock : mTextBlocks) {
		textblock.SetRenderer(stringRenderer);
		textblock.LoadFont();
	}
}

std::vector<TextBlock>::iterator TextBlockManager::begin() {
    return mTextBlocks.begin();
}

std::vector<TextBlock>::iterator TextBlockManager::end() {
	return mTextBlocks.end();
}

TextBlock* TextBlockManager::CreateTextBlock(const std::wstring& text, const D2D1_RECT_F& rect, const StringColor& color, const std::string& font) {
	if (mStringRenderer != nullptr) {
		auto& textblock = mTextBlocks.emplace_back(mStringRenderer, text, rect, color, mStringRenderer->GetFont(font));
		return &textblock;
    } 
    auto& textblock = mTextBlocks.emplace_back(text, rect, color, font);
    return &textblock;

}
