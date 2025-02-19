////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// pch.h
// 2024.12.23 김승범   - EditorInterface 프로젝트의 pch 파일을 생성함. 
//						EditorInterface 구현에 사용되는 DirectX12 헤더들을 추가하였음. 
// 
// 2024.12.31 김승범	  - 프로젝트의 경로 문제를 해결하기 위해 프로젝트를 새로 다시 생성하였음. 
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

// DiectX12 Header
#include <wrl/client.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <comdef.h>
#include <dxgidebug.h>
#include <d3dcompiler.h>

#include "Include/DirectXTK12/SimpleMath.h"

// DirectX12 Library
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

using namespace Microsoft::WRL;

#include <string>