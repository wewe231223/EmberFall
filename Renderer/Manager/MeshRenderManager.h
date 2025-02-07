#pragma once 
#include "../Resource/DefaultBuffer.h"
#include "../Core/Shader.h"
#include "../Resource/PlaneMesh.h"
#include "../Utility/Defines.h"
class MeshRenderManager {
public:
	MeshRenderManager(); 
	~MeshRenderManager() = default;
public:
	void AppendPlaneMeshContext(GraphicsShaderBase* shader, PlaneMesh* mesh);
private:
	DefaultBuffer mPlaneMeshBuffer{};
	// DefaultBuffer mBonedMeshBuffer{};
};