#include "ModuleNavigation.h"

#include "Globals.h"
#include "Application.h"
#include "GameObject.h"
#include "Modules/ModuleFiles.h"
#include "Modules/ModuleTime.h"
#include "Modules/ModuleScene.h"
#include "Components/ComponentAgent.h"
#include "Scene.h"
#include "Detour/DetourCommon.h"
#include "Utils/Logging.h"

#include "Utils/Leaks.h"

bool ModuleNavigation::Init() {
	App->events->AddObserverToEvent(TesseractEventType::PRESSED_PLAY, this);
	App->events->AddObserverToEvent(TesseractEventType::PRESSED_STOP, this);

	return true;
}

UpdateStatus ModuleNavigation::Update() {
	if (!navMesh.IsGenerated()) {
		return UpdateStatus::CONTINUE;
	}

	navMesh.GetTileCache()->update(App->time->GetDeltaTime(), navMesh.GetNavMesh());	// Update obstacles
	navMesh.GetCrowd()->update(App->time->GetDeltaTime(), nullptr);						// Update agents

	return UpdateStatus::CONTINUE;
}

void ModuleNavigation::ReceiveEvent(TesseractEvent& e) {
	switch (e.type) {
	case TesseractEventType::PRESSED_PLAY:
		for (ComponentAgent& agent : App->scene->scene->agentComponents) {
			agent.AddAgentToCrowd();
		}
		break;
	case TesseractEventType::PRESSED_STOP:
		for (ComponentAgent& agent : App->scene->scene->agentComponents) {
			agent.RemoveAgentFromCrowd();
		}
		break;
	}
}

void ModuleNavigation::BakeNavMesh() {
	MSTimer timer;
	timer.Start();
	LOG("Loading NavMesh");
	bool generated = navMesh.Build();
	unsigned timeMs = timer.Stop();
	if (generated) {
		navMesh.GetTileCache()->update(App->time->GetDeltaTime(), navMesh.GetNavMesh());
		navMesh.GetCrowd()->update(App->time->GetDeltaTime(), nullptr);

		navMesh.RescanCrowd();
		navMesh.RescanObstacles();

		LOG("NavMesh successfully baked in %ums", timeMs);
	} else {
		LOG("NavMesh ERROR. Could not be baked in %ums", timeMs);
	}
}

void ModuleNavigation::DrawGizmos() {
	navMesh.DrawGizmos();
}

NavMesh& ModuleNavigation::GetNavMesh() {
	return navMesh;
}

void ModuleNavigation::Raycast(float3 startPosition, float3 targetPosition, bool& hitResult, float3& hitPosition) {
	hitResult = false;
	hitPosition = startPosition;

	if (!navMesh.IsGenerated()) return;

	dtNavMeshQuery* navQuery = navMesh.GetNavMeshQuery();
	if (navQuery == nullptr) return;

	float3 polyPickExt = float3(2, 4, 2);
	dtQueryFilter filter;
	filter.setIncludeFlags(0xffff ^ 0x10); // SAMPLE_POLYFLAGS_ALL ^ SAMPLE_POLYFLAGS_DISABLED
	filter.setExcludeFlags(0);
	filter.setAreaCost(0, 1.0f);	// SAMPLE_POLYAREA_GROUND
	filter.setAreaCost(1, 10.0f);	// SAMPLE_POLYAREA_WATER
	filter.setAreaCost(2, 1.0f);	// SAMPLE_POLYAREA_ROAD
	filter.setAreaCost(3, 1.0f);	// SAMPLE_POLYAREA_DOOR
	filter.setAreaCost(4, 2.0f);	// SAMPLE_POLYAREA_GRASS
	filter.setAreaCost(5, 1.5f);	// SAMPLE_POLYAREA_JUMP

	dtPolyRef startRef;
	navQuery->findNearestPoly(startPosition.ptr(), polyPickExt.ptr(), &filter, &startRef, 0);

	if (startRef) {
		float t = 0;
		int npolys = 0;
		static const int MAX_POLYS = 256;

		dtPolyRef polys[MAX_POLYS];

		float3 hitNormal;

		navQuery->raycast(startRef, startPosition.ptr(), targetPosition.ptr(), &filter, &t, hitNormal.ptr(), polys, &npolys, MAX_POLYS);
		if (t > 1) {
			// No hit
			dtVcopy(hitPosition.ptr(), targetPosition.ptr());
			hitResult = false;
		} else {
			// Hit
			dtVlerp(hitPosition.ptr(), startPosition.ptr(), targetPosition.ptr(), t);
			hitResult = true;
		}
	}
}
