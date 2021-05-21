#pragma once

#include "Module.h"

#include "Navigation/NavMesh.h"

class ModuleNavigation : public Module {
public:
	bool Init() override;
	void BakeNavMesh();
	
	void RenderNavMesh();
	NavMesh& GetNavMesh();

private:
	NavMesh navMesh;
	bool generated = false;
};
