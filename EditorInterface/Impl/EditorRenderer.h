#pragma once 
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// EditorRenderer.h
// 2025.01.04 김승범   - Editor 창을 렌더링 할 Renderer 클래스 파일을 생성하였음.   
//                      
////////////////////////////////////////////////////////////////////////////////////////////////////////////


class EditorRenderer {
public:
	EditorRenderer();
	~EditorRenderer();

public:
private:
	void InitFactory();
private:
	ComPtr<IDXGIFactory> mFactory{ nullptr };

#ifdef _DEBUG
	ComPtr<ID3D12Debug> mDebugController{ nullptr };
	ComPtr<IDXGIDebug> mDXGIDebug{ nullptr };
#endif 

	ComPtr<ID3D12Device> mDevice{ nullptr };

};
