////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface.h
// 2024.12.23 김승범   - Interface 프로젝트 메인 인터페이스 파일을 작성함 
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once 
#include "Include/DirectXTK12/SimpleMath.h"
#include <string>

// Editor Interface 중, Transform Editor 가 사용할 Transform 에 대한 인터페이스 이다. 
__interface IEditorTransformBase {
public:
	virtual DirectX::SimpleMath::Vector3& GetPosition()						PURE;
	virtual DirectX::SimpleMath::Quaternion& GetRotation()					PURE;
	virtual DirectX::SimpleMath::Vector3& GetScale()						PURE;

	virtual const DirectX::SimpleMath::Vector3& GetPosition() const			PURE;
	virtual const DirectX::SimpleMath::Quaternion& GetRotation() const		PURE;
	virtual const DirectX::SimpleMath::Vector3& GetScale() const			PURE;
};



