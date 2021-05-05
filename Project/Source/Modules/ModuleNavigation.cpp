#include "ModuleNavigation.h"

#include "Utils/Logging.h"

#include "Utils/Leaks.h"

bool ModuleNavigation::Init() {
	return true;
}

void ModuleNavigation::BakeNavMesh() {
	bool baked = navMesh.Build();
	if (baked) {
		LOG("NavMesh successfully baked");
	} else {
		LOG("NavMesh ERROR. Could not be baked");
	}
}

NavMesh& ModuleNavigation::GetNavMesh() {
	return navMesh;
}
