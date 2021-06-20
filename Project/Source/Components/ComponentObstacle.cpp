#include "ComponentObstacle.h"

#include "GameObject.h"
#include "Application.h"
#include "Modules/ModuleNavigation.h"
#include "Modules/ModuleEditor.h"
#include "Navigation/NavMesh.h"

#include "imgui.h"

#include "Utils/Leaks.h"

#define JSON_TAG_RADIUS "Radius"
#define JSON_TAG_HEIGHT "Height"

ComponentObstacle::~ComponentObstacle() {
	RemoveObstacle();
}

void ComponentObstacle::Init() {
	AddObstacle();
	currentPosition = GetOwner().GetComponent<ComponentTransform>()->GetGlobalPosition();
}

void ComponentObstacle::Update() {
	float3 newPosition = GetOwner().GetComponent<ComponentTransform>()->GetGlobalPosition();
	if (!newPosition.Equals(currentPosition)) {
		currentPosition = newPosition;
		AddObstacle();
	}
}

void ComponentObstacle::OnEditorUpdate() {
	if (ImGui::Checkbox("Active", &active)) {
		if (GetOwner().IsActive()) {
			if (active) {
				Enable();
			} else {
				Disable();
			}
		}
	}

	if (ImGui::InputFloat("Obstacle radius", &radius, App->editor->dragSpeed2f, 0)) {
		SetRadius(radius);
	}

	if (ImGui::InputFloat("Obstacle height", &height, App->editor->dragSpeed2f, 0)) {
		SetHeight(height);
	}
}

void ComponentObstacle::OnEnable() {
	AddObstacle();
}

void ComponentObstacle::OnDisable() {
	RemoveObstacle();
}

void ComponentObstacle::Save(JsonValue jComponent) const {
	jComponent[JSON_TAG_RADIUS] = radius;
	jComponent[JSON_TAG_HEIGHT] = height;
}

void ComponentObstacle::Load(JsonValue jComponent) {
	radius = jComponent[JSON_TAG_RADIUS];
	height = jComponent[JSON_TAG_HEIGHT];
}


void ComponentObstacle::AddObstacle() {
	NavMesh& navMesh = App->navigation->GetNavMesh();
	if (!navMesh.IsGenerated()) return;

	dtTileCache* tileCache = navMesh.GetTileCache();
	if (!tileCache)
		return;
	
	RemoveObstacle();

	obstacleReference = new dtObstacleRef;

	float3 position = GetOwner().GetComponent<ComponentTransform>()->GetGlobalPosition();
	tileCache->addObstacle(&position[0], radius, height, obstacleReference);
}

void ComponentObstacle::RemoveObstacle() {
	NavMesh& navMesh = App->navigation->GetNavMesh();
	if (!navMesh.IsGenerated()) return;

	dtTileCache* tileCache = navMesh.GetTileCache();
	if (!tileCache || !obstacleReference)
		return;

	tileCache->removeObstacle(*obstacleReference);
	RELEASE(obstacleReference);
	obstacleReference = nullptr;
}

void ComponentObstacle::SetRadius(float newRadius) {
	radius = newRadius;
	AddObstacle();
}

void ComponentObstacle::SetHeight(float newHeight) {
	height = newHeight;
	AddObstacle();
}
