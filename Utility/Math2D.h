#pragma once
#include "../Utility/DirectXInclude.h"

inline DirectX::XMFLOAT3X3 Create2DScaleMatrix(float scaleX, float scaleY) {
	return DirectX::XMFLOAT3X3{
		scaleX, 0.0f,   0.0f,
		0.0f,   scaleY, 0.0f,
		0.0f,   0.0f,   1.0f
	};
}

inline DirectX::XMFLOAT3X3 CreateScreenTransformMatrix(float screenWidth, float screenHeight) {
	// NDC 좌표를 스크린 좌표로 변환하는 행렬
	return DirectX::XMFLOAT3X3{
		2.f / screenWidth ,			0.0f,								0.f,
		0.0f,						-2.f / screenHeight ,				0.f,
		-1.f,						1.f,								1.f
	};
}

inline DirectX::XMFLOAT3X3 CreateScaleMatrix(float x, float y) {
	return DirectX::XMFLOAT3X3{
		x,0.f,0.f,
		0.f,y,0.f,
		0.f,0.f,1.f
	};
}

inline DirectX::XMFLOAT3X3 CreateTranslateMatrix(float x, float y) {
	return DirectX::XMFLOAT3X3{
		1.f,0.f,0.f,
		0.f,1.f,0.f,
		0.f,0.f,1.f
	};
}

inline DirectX::XMFLOAT3X3 Transpose(const DirectX::XMFLOAT3X3& matrix) {
	return DirectX::XMFLOAT3X3{
		matrix._11, matrix._21, matrix._31,
		matrix._12, matrix._22, matrix._32,
		matrix._13, matrix._23, matrix._33
	};
}

inline DirectX::XMFLOAT3X3 Mul3x3(const DirectX::XMFLOAT3X3& mat1, const DirectX::XMFLOAT3X3& mat2)
{
	// XMFLOAT3X3를 XMMATRIX로 변환
	DirectX::XMMATRIX matrix1 = XMLoadFloat3x3(&mat1);
	DirectX::XMMATRIX matrix2 = XMLoadFloat3x3(&mat2);

	// 행렬 곱셈
	DirectX::XMMATRIX resultMatrix = XMMatrixMultiply(matrix1, matrix2);

	// 결과를 XMFLOAT3X3로 변환
	DirectX::XMFLOAT3X3 result;
	DirectX::XMStoreFloat3x3(&result, resultMatrix);

	return result;
}