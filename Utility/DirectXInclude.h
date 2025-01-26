#pragma once 
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DirectXInclude.h
// 2025.01.12 김승범   - DirectX 12 Include 를 간편하게 하기 위해 작성함. 
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
#include "../External/Include/DirectXTK12/SimpleMath.h"
#include "../External/Include/DirectXTK12/SimpleMath.inl"

// DirectX12 Library
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

using namespace Microsoft::WRL;