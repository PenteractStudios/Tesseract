#include "LightFrustum.h"

#include "Globals.h"
#include "Application.h"
#include "GameObject.h"
#include "Modules/ModuleScene.h"
#include "Modules/ModuleCamera.h"
#include "Utils/Random.h"

#include "debugdraw.h"
#include "Geometry/Plane.h"
#include "Math/float3x3.h"

#include "Utils/Leaks.h"

LightFrustum::LightFrustum() {
	for (unsigned i = 0; i < NUM_CASCADE_FRUSTUM; i++) {
		subFrustums[i].orthographicFrustum.SetKind(FrustumSpaceGL, FrustumRightHanded);
		subFrustums[i].color = float3(Random(), Random(), Random());
	}
}

void LightFrustum::UpdateFrustums() {

	ComponentCamera* gameCamera = App->camera->GetGameCamera();
	if (!gameCamera) return;

	Frustum *gameFrustum = gameCamera->GetFrustum();

	float farDistance = MINIMUM_FAR_DISTANE;

	if (mode == CascadeMode::FitToScene) {
		
		for (unsigned int i = 0; i < NUM_CASCADE_FRUSTUM; i++, farDistance *= 3.f) {
			subFrustums[i].perspectiveFrustum.SetKind(FrustumSpaceGL, FrustumRightHanded);
			subFrustums[i].perspectiveFrustum.SetHorizontalFovAndAspectRatio(gameFrustum->HorizontalFov(), gameFrustum->AspectRatio());
			subFrustums[i].perspectiveFrustum.SetPos(gameFrustum->Pos());
			subFrustums[i].perspectiveFrustum.SetUp(gameFrustum->Up());
			subFrustums[i].perspectiveFrustum.SetFront(gameFrustum->Front());
			subFrustums[i].perspectiveFrustum.SetViewPlaneDistances(0, farDistance);
			subFrustums[i].planes.CalculateFrustumPlanes(subFrustums[i].perspectiveFrustum);
		}

	} else {

		float nearDistance = 0.0f;

		for (unsigned int i = 0; i < NUM_CASCADE_FRUSTUM; i++, nearDistance = farDistance, farDistance *= 2.f) {
			subFrustums[i].perspectiveFrustum.SetKind(FrustumSpaceGL, FrustumRightHanded);
			subFrustums[i].perspectiveFrustum.SetHorizontalFovAndAspectRatio(gameFrustum->HorizontalFov(), gameFrustum->AspectRatio());
			subFrustums[i].perspectiveFrustum.SetPos(gameFrustum->Pos());
			subFrustums[i].perspectiveFrustum.SetUp(gameFrustum->Up());
			subFrustums[i].perspectiveFrustum.SetFront(gameFrustum->Front());
			subFrustums[i].perspectiveFrustum.SetViewPlaneDistances(nearDistance, farDistance);
			subFrustums[i].planes.CalculateFrustumPlanes(subFrustums[i].perspectiveFrustum);
		}
	}

}

void LightFrustum::ReconstructFrustum() {
	if (!dirty) return;

	GameObject* light = App->scene->scene->directionalLight;
	if (!light) return;

	ComponentTransform* transform = light->GetComponent<ComponentTransform>();
	assert(transform);

	float4x4 lightOrientation = transform->GetGlobalMatrix();
	lightOrientation.SetTranslatePart(float3::zero);

	AABB lightAABB;
	lightAABB.SetNegativeInfinity();

	for (GameObject& go : App->scene->scene->gameObjects) {
		Mask mask = go.GetMask();
		if ((mask.bitMask & static_cast<int>(MaskType::CAST_SHADOWS)) != 0 && go.HasComponent<ComponentMeshRenderer>()) {
			ComponentBoundingBox* componentBBox = go.GetComponent<ComponentBoundingBox>();
			if (componentBBox) {
				AABB boundingBox = componentBBox->GetWorldAABB();
				OBB orientedBoundingBox = boundingBox.Transform(lightOrientation.Inverted());
				lightAABB.Enclose(orientedBoundingBox.MinimalEnclosingAABB());
			}
		}
	}

	float3 minPoint = lightAABB.minPoint;
	float3 maxPoint = lightAABB.maxPoint;
	float3 position = lightOrientation.RotatePart() * float3((maxPoint.x + minPoint.x) * 0.5f, ((maxPoint.y + minPoint.y) * 0.5f), minPoint.z);

	subFrustums[0].orthographicFrustum.SetOrthographic((maxPoint.x - minPoint.x), (maxPoint.y - minPoint.y));
	subFrustums[0].orthographicFrustum.SetUp(transform->GetUp());
	subFrustums[0].orthographicFrustum.SetFront(transform->GetFront());
	subFrustums[0].orthographicFrustum.SetPos(position);
	subFrustums[0].orthographicFrustum.SetViewPlaneDistances(0.0f, (maxPoint.z - minPoint.z));

	dirty = true;
}

void LightFrustum::DrawGizmos() {
	for (unsigned i = 0; i < NUM_CASCADE_FRUSTUM; i++) {
		dd::frustum(subFrustums[i].perspectiveFrustum.ViewProjMatrix().Inverted(), dd::colors::Green);
	}
}

Frustum LightFrustum::GetFrustum() const {
	return subFrustums[0].orthographicFrustum;
}

void LightFrustum::Invalidate() {
	dirty = true;
}
