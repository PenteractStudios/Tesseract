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
	hitPosition = targetPosition;

	if (!navMesh.IsGenerated()) return;

	dtNavMeshQuery* navQuery = navMesh.GetNavMeshQuery();
	if (navQuery == nullptr) return;

	float3 m_polyPickExt = float3(2, 4, 2);
	dtQueryFilter m_filter;
	m_filter.setIncludeFlags(0xffff ^ 0x10);	// SAMPLE_POLYFLAGS_ALL ^ SAMPLE_POLYFLAGS_DISABLED
	m_filter.setExcludeFlags(0);
	m_filter.setAreaCost(0, 1.0f);	// SAMPLE_POLYAREA_GROUND
	m_filter.setAreaCost(1, 10.0f);	// SAMPLE_POLYAREA_WATER
	m_filter.setAreaCost(2, 1.0f);	// SAMPLE_POLYAREA_ROAD
	m_filter.setAreaCost(3, 1.0f);	// SAMPLE_POLYAREA_DOOR
	m_filter.setAreaCost(4, 2.0f);	// SAMPLE_POLYAREA_GRASS
	m_filter.setAreaCost(5, 1.5f);	// SAMPLE_POLYAREA_JUMP

	dtPolyRef m_startRef;
	navQuery->findNearestPoly(startPosition.ptr(), m_polyPickExt.ptr(), &m_filter, &m_startRef, 0);

	dtPolyRef m_endRef;
	navQuery->findNearestPoly(targetPosition.ptr(), m_polyPickExt.ptr(), &m_filter, &m_endRef, 0);

	float m_nstraightPath = 0;
	if (m_startRef) {

		float t = 0;
		int m_npolys = 0;
		int m_nstraightPath = 2;
		static const int MAX_POLYS = 256;
		float m_straightPath[MAX_POLYS * 3];
		m_straightPath[0] = startPosition[0];
		m_straightPath[1] = startPosition[1];
		m_straightPath[2] = startPosition[2];

		dtPolyRef m_polys[MAX_POLYS];

		float3 m_hitNormal;

		navQuery->raycast(m_startRef, startPosition.ptr(), targetPosition.ptr(), &m_filter, &t, m_hitNormal.ptr(), m_polys, &m_npolys, MAX_POLYS);
		if (t > 1) {
			// No hit
			dtVcopy(hitPosition.ptr(), targetPosition.ptr());
			hitResult = false;
		} else {
			// Hit
			dtVlerp(hitPosition.ptr(), startPosition.ptr(), targetPosition.ptr(), t);
			hitResult = true;
		}
		// Adjust height.
		// OPTIONAL
		/*if (m_npolys > 0) {
			float h = 0;
			navQuery->getPolyHeight(m_polys[m_npolys - 1], hitPosition.ptr(), &h);
			hitPosition[1] = h;
		}
		dtVcopy(&m_straightPath[3], hitPosition.ptr());*/
	}
}
