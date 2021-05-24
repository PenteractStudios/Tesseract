#pragma once

#include "Module.h"
#include "Utils/Buffer.h"

#include "Navigation/NavMesh.h"
#include "Resources/ResourceNavMesh.h"
#include "DetourCrowd/DetourCrowd.h"

class ModuleNavigation : public Module {
public:
	bool Init() override;
	UpdateStatus Update() override;
	void ReceiveEvent(TesseractEvent& e) override;

	void BakeNavMesh();
	
	void RenderNavMesh();
	NavMesh& GetNavMesh();
	void ReleaseNavMesh();

private:
	NavMesh navMesh;
};
