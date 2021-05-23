#pragma once

#include "Module.h"
#include "Utils/Buffer.h"

#include "Navigation/NavMesh.h"
#include "Resources/ResourceNavMesh.h"

class ModuleNavigation : public Module {
public:
	bool Init() override;
	void BakeNavMesh();
	
	void RenderNavMesh();
	NavMesh& GetNavMesh();

	void ReleaseNavMesh();

private:
	NavMesh navMesh;

	bool generated = false;
};
