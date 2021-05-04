#include "ModuleNavigation.h"

#include "Utils/Leaks.h"

bool ModuleNavigation::Init() {
	return true;
}

void ModuleNavigation::BakeNavMesh() {

}

NavMesh& ModuleNavigation::GetNavMesh() {
	return navMesh;
}
