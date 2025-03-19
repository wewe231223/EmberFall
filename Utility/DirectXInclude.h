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
#include <dwrite.h>
#include <dwrite_3.h>
#include <d2d1_3.h>
#include <d3d11on12.h>
#include "../External/Include/DirectXTK12/d3dx12.h"
#include "../External/Include/DirectXTK12/SimpleMath.h"
#include "../External/Include/DirectXTK12/SimpleMath.inl"

// DirectX12 Library
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "dxguid.lib")

#ifdef _DEBUG
#pragma comment(lib, "External/lib/debug/DirectXTK12.lib")
#else 
#pragma comment(lib, "External/lib/release/DirectXTK12.lib")
#endif 
using namespace Microsoft::WRL;
namespace SimpleMath = DirectX::SimpleMath;

using ID3D12Blob = ID3D10Blob;