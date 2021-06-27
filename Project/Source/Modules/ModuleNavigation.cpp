#include "ModuleNavigation.h"

#include "Application.h"
#include "GameObject.h"
#include "Modules/ModuleFiles.h"
#include "Modules/ModuleTime.h"
#include "Modules/ModuleScene.h"
#include "Components/ComponentAgent.h"
#include "Scene.h"

#include "Globals.h"
#include "Utils/Buffer.h"
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

	navMesh.GetCrowd()->update(App->time->GetDeltaTime(), nullptr);						// Update agents
	navMesh.GetTileCache()->update(App->time->GetDeltaTime(), navMesh.GetNavMesh());	// Update obstacles

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