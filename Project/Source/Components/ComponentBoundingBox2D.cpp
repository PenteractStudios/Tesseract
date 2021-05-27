#include "ComponentBoundingBox2D.h"

#include "Application.h"
#include "Modules/ModuleInput.h"
#include "Components/UI/ComponentTransform2D.h"
#include "GameObject.h"
#include "Utils/Logging.h"
#include "Panels/PanelScene.h"
#include "Modules/ModuleWindow.h"
#include "Modules/ModuleRender.h"
#include "Modules/ModuleTime.h"
#include "Modules/ModuleUserInterface.h"

#include "debugdraw.h"
#include "Geometry/AABB.h"
#include "Geometry/OBB.h"
#include "Geometry/Circle.h"
#include "Math/float3x3.h"

#include "Utils/Leaks.h"

#define JSON_TAG_LOCAL_BOUNDING_BOX2D "LocalBoundingBox2D"

void ComponentBoundingBox2D::Init() {
	ComponentTransform2D* transform2D = GetOwner().GetComponent<ComponentTransform2D>();
	ComponentCanvasRenderer* canvasRenderer = GetOwner().GetComponent<ComponentCanvasRenderer>();
	if (transform2D != nullptr) {
		float3 minPoint = float3(-0.5f, -0.5f, 0.0f);
		float3 maxPoint = float3(0.5f, 0.5f, 0.0f);

		SetLocalBoundingBox(AABB(minPoint, maxPoint));
		CalculateWorldBoundingBox();
	}
}

void ComponentBoundingBox2D::Update() {
	CalculateWorldBoundingBox();
}

void ComponentBoundingBox2D::Save(JsonValue jComponent) const {
	JsonValue jLocalBoundingBox = jComponent[JSON_TAG_LOCAL_BOUNDING_BOX2D];
	jLocalBoundingBox[0] = localAABB.minPoint.x;
	jLocalBoundingBox[1] = localAABB.minPoint.y;
	jLocalBoundingBox[2] = localAABB.maxPoint.x;
	jLocalBoundingBox[3] = localAABB.maxPoint.y;
}

void ComponentBoundingBox2D::Load(JsonValue jComponent) {
	JsonValue jLocalBoundingBox = jComponent[JSON_TAG_LOCAL_BOUNDING_BOX2D];
	localAABB.minPoint.Set(jLocalBoundingBox[0], jLocalBoundingBox[1], 0.0f);
	localAABB.maxPoint.Set(jLocalBoundingBox[2], jLocalBoundingBox[3], 0.0f);

	dirty = true;
}

void ComponentBoundingBox2D::SetLocalBoundingBox(const AABB& boundingBox) {
	localAABB = boundingBox;
	dirty = true;
}

void ComponentBoundingBox2D::CalculateWorldBoundingBox(bool force) {
	if (dirty || force) {
		ComponentTransform2D* transform2d = GetOwner().GetComponent<ComponentTransform2D>();
		ComponentCanvasRenderer* canvasRenderer = GetOwner().GetComponent<ComponentCanvasRenderer>();
		float screenFactor = 1.0f;
		float2 screenSize(0, 0);
		float3 position(0, 0, 0);
		if (canvasRenderer != nullptr) {
			float2 size = transform2d->GetSize();
			float3 scale = transform2d->GetScale();
			float screenFactor = canvasRenderer->GetCanvasScreenFactor();
			float2 screenSize = canvasRenderer->GetCanvasSize();
			position = transform2d->GetScreenPosition();

			float3 minPoint = position.xyz().Mul(float3(1.0f, -1.0f, 0.0f).Mul(screenFactor)) + float3(screenSize, 0.0f) / 2.0f
							  - 0.5f * (float3(transform2d->GetSize(), 0.0f).Mul(transform2d->GetScale().xyz()).Mul(screenFactor));
			float3 maxPoint = position.xyz().Mul(float3(1.0f, -1.0f, 0.0f).Mul(screenFactor)) + float3(screenSize, 0.0f) / 2.0f
							  + 0.5f * (float3(transform2d->GetSize(), 0.0f).Mul(transform2d->GetScale().xyz()).Mul(screenFactor));

			SetLocalBoundingBox(AABB(minPoint, maxPoint));
		}

		worldOBB = OBB(localAABB);
		worldOBB.Transform(transform2d->GetGlobalMatrix());
		worldAABB = worldOBB.MinimalEnclosingAABB();

#if GAME
		float2 windowPos = float2(App->window->GetPositionX(), App->window->GetPositionY());
		//worldAABB.minPoint += windowPos;
		//worldOBB.minPoint += windowPos;
		//worldAABB.maxPoint += windowPos;
		//worldOBB.maxPoint += windowPos;
#endif
	}
}

void ComponentBoundingBox2D::DrawGizmos() {
	//if (!App->time->IsGameRunning() && !App->userInterface->IsUsing2D()) {
		float3 points[8];
		GetWorldOBB().GetCornerPoints(points);

		// Reorder points for drawing
		float3 aux;
		aux = points[2];
		points[2] = points[3];
		points[3] = aux;
		aux = points[6];
		points[6] = points[7];
		points[7] = aux;

		// Drawing 0BB
		dd::box(points, dd::colors::GreenYellow);
		// Drawing AABB
		dd::aabb(worldAABB.minPoint, worldAABB.maxPoint, dd::colors::Red);
	//}
}

void ComponentBoundingBox2D::Invalidate() {
	dirty = true;
}

const AABB& ComponentBoundingBox2D::GetWorldAABB() {
	CalculateWorldBoundingBox();
	return worldAABB;
}

const OBB& ComponentBoundingBox2D::GetWorldOBB() {
	CalculateWorldBoundingBox();
	return worldOBB;
}

bool ComponentBoundingBox2D::CanBeRemoved() const {
	return !GetOwner().GetComponent<ComponentSelectable>();
}