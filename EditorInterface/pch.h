////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// pch.h
// 2024.12.23 ��¹�   - EditorInterface ������Ʈ�� pch ������ ������. 
//						EditorInterface ������ ���Ǵ� DirectX12 ������� �߰��Ͽ���. 
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

// DirectX12 Library
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

using namespace Microsoft::WRL;