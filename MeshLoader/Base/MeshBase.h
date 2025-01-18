#pragma once 

#include <vector>
#include <DirectXMath.h>

struct IMesh abstract {
	virtual void GetPosition(std::vector<DirectX::XMFLOAT3>&)	= 0;
	virtual void GetNormal(std::vector<DirectX::XMFLOAT3>&)		= 0;
	virtual void GetUV(std::vector<DirectX::XMFLOAT2>&)			= 0;
	virtual void GetTangent(std::vector<DirectX::XMFLOAT3>&)	= 0;
	virtual void GetBiTangent(std::vector<DirectX::XMFLOAT3>&)	= 0;
	virtual void GetBoneIndex(std::vector<DirectX::XMUINT4>&)	= 0;
	virtual void GetBoneWeight(std::vector<DirectX::XMFLOAT4>&) = 0;

	virtual void GetIndex(std::vector<unsigned int>&)			= 0;
};


