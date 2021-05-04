#pragma once

#include "Module.h"

#include "Navigation/NavMesh.h"

class ModuleNavigation : public Module {
public:
	bool Init() override;
	void BakeNavMesh();

	NavMesh& GetNavMesh();

private:
	NavMesh navMesh;
};
