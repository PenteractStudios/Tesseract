#include "ModuleNavigation.h"

#include "Application.h"
#include "Modules/ModuleFiles.h"
#include "Utils/Buffer.h"

#include "Utils/Logging.h"

#include "Utils/Leaks.h"

bool ModuleNavigation::Init() {
	return true;
}

void ModuleNavigation::BakeNavMesh() {
	generated = navMesh.Build();
	if (generated) {
		LOG("NavMesh successfully baked");
	} else {
		LOG("NavMesh ERROR. Could not be baked");
	}
}

void ModuleNavigation::RenderNavMesh() {
	navMesh.Render();
}

NavMesh& ModuleNavigation::GetNavMesh() {
	return navMesh;
}

void ModuleNavigation::ReleaseNavMesh() {

}
