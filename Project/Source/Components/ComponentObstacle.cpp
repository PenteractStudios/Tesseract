#include "ComponentObstacle.h"

#include "GameObject.h"
#include "Application.h"
#include "Modules/ModuleNavigation.h"
#include "Modules/ModuleEditor.h"
#include "Navigation/NavMesh.h"

#include "imgui.h"

#include "Utils/Leaks.h"

#define JSON_TAG_SIZE "Size"
#define JSON_TAG_TYPE "Type"

ComponentObstacle::~ComponentObstacle() {
	RemoveObstacle();
}

void ComponentObstacle::Init() {
	currentPosition = GetOwner().GetComponent<ComponentTransform>()->GetGlobalPosition();
	AddObstacle();
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

	ImGui::Text("");
	ImGui::Text("Obstacle type");
	bool mustBeAdded = ImGui::RadioButton("Cylinder", &obstacleType, ObstacleType::DT_OBSTACLE_CYLINDER);
	ImGui::SameLine();
	mustBeAdded |= ImGui::RadioButton("Box", &obstacleType, ObstacleType::DT_OBSTACLE_BOX);
	ImGui::Text("");
	if (mustBeAdded) {
		SetObstacleType(static_cast<ObstacleType>(obstacleType));
	}

	if (obstacleType == ObstacleType::DT_OBSTACLE_CYLINDER) {
		if (ImGui::InputFloat("Cylinder radius", &boxSize.x, App->editor->dragSpeed2f, 0)) {
			SetRadius(boxSize.x);
		}

		if (ImGui::InputFloat("Cylinder height", &boxSize.y, App->editor->dragSpeed2f, 0)) {
			SetHeight(boxSize.y);
		}
	} else {
		if (ImGui::DragFloat3("Box Size", boxSize.ptr(), App->editor->dragSpeed2f, -inf, inf)) {
			SetBoxSize(boxSize);
		}
	}
}

void ComponentObstacle::OnEnable() {
	AddObstacle();
}

void ComponentObstacle::OnDisable() {
	RemoveObstacle();
}

void ComponentObstacle::Save(JsonValue jComponent) const {
	JsonValue jSize = jComponent[JSON_TAG_SIZE];
	jSize[0] = boxSize.x;
	jSize[1] = boxSize.y;
	jSize[2] = boxSize.z;

	jComponent[JSON_TAG_TYPE] = obstacleType;
}

void ComponentObstacle::Load(JsonValue jComponent) {
	obstacleType = jComponent[JSON_TAG_TYPE];

	JsonValue jSize = jComponent[JSON_TAG_SIZE];
	boxSize.Set(jSize[0], jSize[1], jSize[2]);
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

	switch (obstacleType) {
	case ObstacleType::DT_OBSTACLE_CYLINDER:
		tileCache->addObstacle(&position[0], boxSize.x, boxSize.y, obstacleReference);
		break;
	case ObstacleType::DT_OBSTACLE_BOX: {
		float3 bmin = position - (boxSize / 2);
		float3 bmax = position + (boxSize / 2);
		tileCache->addBoxObstacle(&bmin[0], &bmax[0], obstacleReference);
	}
		break;
	default:
		break;
	}
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
	if (obstacleType == ObstacleType::DT_OBSTACLE_CYLINDER) {
		boxSize.x = newRadius;
		AddObstacle();
	}
}

void ComponentObstacle::SetHeight(float newHeight) {
	if (obstacleType == ObstacleType::DT_OBSTACLE_CYLINDER) {
		boxSize.y = newHeight;
		AddObstacle();
	}
}

void ComponentObstacle::SetBoxSize(float3 size) {
	if (obstacleType == ObstacleType::DT_OBSTACLE_BOX || obstacleType == ObstacleType::DT_OBSTACLE_ORIENTED_BOX) {
		boxSize = size;
		AddObstacle();
	}
}

void ComponentObstacle::SetObstacleType(ObstacleType newType) {
	obstacleType = newType;
	ResetSize();
	AddObstacle();
}

void ComponentObstacle::ResetSize() {
	if (obstacleType == ObstacleType::DT_OBSTACLE_CYLINDER) {
		boxSize.x = 1.0f;
		boxSize.y = 2.0f;
		boxSize.z = 0;
	} else {
		boxSize = float3::one;
	}
}
