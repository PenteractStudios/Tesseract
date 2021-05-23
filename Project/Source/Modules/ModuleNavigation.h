#pragma once

#include "Module.h"
#include "Utils/Buffer.h"

#include "Navigation/NavMesh.h"

class ModuleNavigation : public Module {
public:
	bool Init() override;
	void BakeNavMesh();
	
	void RenderNavMesh();
	NavMesh& GetNavMesh();
	void SaveNavMesh();
	void LoadNavMesh(const char* filePath);

private:
	NavMesh navMesh;
	bool generated = false;
};
