////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// pch.h
// 2025.01.09 김승범   -	메인 창에 그림을 그리는 역할을 하는 Renderer 프로젝트를 생성하고, pch 헤더를 만들었음. 
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once


#include <array>

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
#include "../External/Include/DirectXTK12/d3dx12.h"

// DirectX12 Library
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

using namespace Microsoft::WRL;

