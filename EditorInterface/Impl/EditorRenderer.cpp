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

#endif 
}
