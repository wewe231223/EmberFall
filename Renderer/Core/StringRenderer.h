#pragma once 
#include <array>
#include "../Resource/Texture.h"
#include "../Utility/DirectXInclude.h"
#include "../Config/Config.h"
#include "../Utility/Defines.h"

class StringRenderer; 

class TextBlock {
public:
	TextBlock() = default; 
	TextBlock(StringRenderer* stringRenderer, const std::wstring& text, const D2D1_RECT_F& rect, const StringColor& color, IDWriteTextFormat* font);
	TextBlock(const std::wstring& text, const D2D1_RECT_F& rect, const StringColor& color, const std::string& font);
	~TextBlock() = default;
	
	TextBlock(const TextBlock& other) = default;
	TextBlock& operator=(const TextBlock& other) = default;

	TextBlock(TextBlock&& other) noexcept = default;
	TextBlock& operator=(TextBlock&& other) noexcept = default;
public:
	void SetRenderer(StringRenderer* stringRenderer);

	std::wstring& GetText();
	D2D1_RECT_F& GetRect();
	StringColor& GetColor();
	IDWriteTextFormat* GetFont() const;

	// 일반적인 상황에서 호출하지 말 것. 
	void LoadFont(); 

	void ChangeFont(const std::string& fontName);
private:
	StringRenderer* mStringRenderer{ nullptr };
	std::wstring mText{};

	D2D1_RECT_F mRect{};

	StringColor mColor{ StringColor::Black };
	IDWriteTextFormat* mFont{ nullptr };

	std::string mInitialFontName{};
};

// Renderer 의 초기화가 끝난 뒤 접근해야 함. 
class TextBlockManager {
public:
	TextBlockManager();
	~TextBlockManager() = default;
public:
	static TextBlockManager& GetInstance();
public:
	void Initialize(StringRenderer* stringRenderer);

	std::vector<TextBlock>::iterator begin(); 
	std::vector<TextBlock>::iterator end();

	TextBlock* CreateTextBlock(const std::wstring& text, const D2D1_RECT_F& rect, const StringColor& color, const std::string& font);
private:
	StringRenderer* mStringRenderer{ nullptr };

	std::vector<TextBlock> mTextBlocks{};
};


class StringRenderer {
public:
	StringRenderer() = default;
	~StringRenderer() = default;

public:
	void Initialize(ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandQueue> commandQueue, std::array<Texture, Config::BACKBUFFER_COUNT<size_t>>& backBuffers);

	void LoadExternalFont(const std::string& fontName, const std::filesystem::path& path, const std::wstring& fontFamilyName, const std::wstring& locale, DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE style = DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH stretch = DWRITE_FONT_STRETCH_NORMAL, float size = 20.f, DWRITE_TEXT_ALIGNMENT alignment = DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment = DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

	void LoadSystemFont(const std::string& fontName, const std::wstring& fontFamilyName, const std::wstring& locale, DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE style = DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH stretch = DWRITE_FONT_STRETCH_NORMAL, float size = 20.f, DWRITE_TEXT_ALIGNMENT alignment = DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment = DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

	IDWriteTextFormat* GetFont(const std::string& fontName);

	void Render();
private:
	void InitBrush(); 
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
	std::unordered_map<std::string, ComPtr<IDWriteTextFormat>> mFonts{};
};