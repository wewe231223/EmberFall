#include "pch.h"
#include "EditorRenderer.h"

#include "../Utility/Exceptions.h"

EditorRenderer::EditorRenderer()
{
}

EditorRenderer::~EditorRenderer()
{
}

void EditorRenderer::InitFactory()
{
	CheckHR(CreateDXGIFactory(IID_PPV_ARGS(&mFactory)));
#ifdef _DEBUG
	CheckHR(D3D12GetDebugInterface(IID_PPV_ARGS(&mDebugController)));
	CheckHR(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&mDXGIDebug)));

	mDebugController->EnableDebugLayer();
	mDebugController->SetEnableGPUBasedValidation(true);
	
	mDXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
#endif 


}
