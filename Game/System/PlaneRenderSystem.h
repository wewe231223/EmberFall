#pragma once 
#include "../Renderer/Resource/DefaultBuffer.h"
#include "../Game/System/GameObject.h"
#include "../Game/System/PartIdentifier.h"
#include "../Utility/DirectXInclude.h"
#include <vector>

// Material Batching... 


/*
1. 정규 업데이트를 거친다.

2. 정규 업데이트 결과를 토대로 월드 행렬을 계산하고 프러스텀 컬링을 진행한다.
3. 프러스텀 컬링의 결과를 Material 별로 나열된 ModelContext 리스트로 만들어 리턴한다.
-> 다른 Material 이라도 같은 Shader 를 사용하는 경우...
4. 그 순서대로 ModelContext 를 버퍼에 카피

3,4 를 RenderSystem 에서 한번에 처리?

MeshRenderSystem 은 

*/

// PlaneMeshRenderSystem, BonnedMeshRenderSystem 은 독립적인 시스템으로 존재한다. 
class PlaneMeshRenderSystem {
public:
	PlaneMeshRenderSystem();
	PlaneMeshRenderSystem(ComPtr<ID3D12Device>& device);
	~PlaneMeshRenderSystem() = default;
public:
	// Scene 의 모든 ArcheType 을 순회하며 적용. Camera 가 있는 ArcheType 과, PlaneMesh + Material 이 조합된 ArcheType 을 찾아낸다. 
	void SubscribeArcheType(ArcheType* archeType);
	void PrepareRender();
	void Render(ComPtr<ID3D12GraphicsCommandList>& commandList);
	void FinishRender(); 
private:
	PartIdentifier mPartIdentifier{};
	std::vector<ArcheType*> mArcheTypes{};

	ArcheType* mCameraArcheType{ nullptr };
};