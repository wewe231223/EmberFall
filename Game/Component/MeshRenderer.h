#pragma once 
#include "../Game/Component/ComponentBase.h"

class MeshRenderer : public ComponentBase {
public:
	static constexpr size_t index = 1;
public:
	MeshRenderer();
	virtual ~MeshRenderer();

private:

};