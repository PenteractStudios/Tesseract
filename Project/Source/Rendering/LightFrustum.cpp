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

	subFrustums.resize(numberCascades);

	float farDistance = MINIMUM_FAR_DISTANE;

	for (unsigned i = 0; i < numberCascades; i++, farDistance *= 2.f) {
		subFrustums[i].orthographicFrustum.SetKind(FrustumSpaceGL, FrustumRightHanded);
		subFrustums[i].perspectiveFrustum.SetKind(FrustumSpaceGL, FrustumRightHanded);
		subFrustums[i].farDistance = farDistance;
	}

	subFrustums[0].color = float3(1, 0, 0);
	subFrustums[1].color = float3(0, 1, 0);
	subFrustums[2].color = float3(0, 0, 1);
	subFrustums[3].color = float3(1, 0, 1);

	subFrustums[0].multiplier = 1.0f;
	subFrustums[1].multiplier = 1.0f;
	subFrustums[2].multiplier = 1.0f;
	subFrustums[3].multiplier = 1.0f;

}

void LightFrustum::UpdateFrustums() {

	ComponentCamera* gameCamera = App->camera->GetGameCamera();
	if (!gameCamera) return;

	Frustum *gameFrustum = gameCamera->GetFrustum();

	if (mode == CascadeMode::FitToScene) {
		
		for (unsigned int i = 0; i < numberCascades; i++) {
			subFrustums[i].perspectiveFrustum.SetKind(FrustumSpaceGL, FrustumRightHanded);
			subFrustums[i].perspectiveFrustum.SetHorizontalFovAndAspectRatio(gameFrustum->HorizontalFov(), gameFrustum->AspectRatio());
			subFrustums[i].perspectiveFrustum.SetPos(gameFrustum->Pos());
			subFrustums[i].perspectiveFrustum.SetUp(gameFrustum->Up());
			subFrustums[i].perspectiveFrustum.SetFront(gameFrustum->Front());
			subFrustums[i].perspectiveFrustum.SetViewPlaneDistances(gameFrustum->NearPlaneDistance(), subFrustums[i].farDistance);
			subFrustums[i].planes.CalculateFrustumPlanes(subFrustums[i].perspectiveFrustum);
		}

	} else {

		float nearDistance = 0.0f;

		for (unsigned int i = 0; i < numberCascades; i++, nearDistance = subFrustums[i].farDistance) {
			subFrustums[i].perspectiveFrustum.SetKind(FrustumSpaceGL, FrustumRightHanded);
			subFrustums[i].perspectiveFrustum.SetHorizontalFovAndAspectRatio(gameFrustum->HorizontalFov(), gameFrustum->AspectRatio());
			subFrustums[i].perspectiveFrustum.SetPos(gameFrustum->Pos());
			subFrustums[i].perspectiveFrustum.SetUp(gameFrustum->Up());
			subFrustums[i].perspectiveFrustum.SetFront(gameFrustum->Front());
			subFrustums[i].perspectiveFrustum.SetViewPlaneDistances(nearDistance, subFrustums[i].farDistance);
			subFrustums[i].planes.CalculateFrustumPlanes(subFrustums[i].perspectiveFrustum);
		}
	}

}

void LightFrustum::ReconstructFrustum(ShadowCasterType shadowCasterType) {
	if (!dirty) return;

	UpdateFrustums();

	GameObject* light = App->scene->scene->directionalLight;
	if (!light) return;

	ComponentTransform* transform = light->GetComponent<ComponentTransform>();
	assert(transform);

	for (unsigned int i = 0; i < numberCascades; i++) {
		float4x4 lightOrientation = transform->GetGlobalMatrix();
		lightOrientation.SetTranslatePart(float3::zero);
		
		AABB lightAABB;
		lightAABB.SetNegativeInfinity();

		std::vector<GameObject*> gameObjects = (shadowCasterType == ShadowCasterType::STATIC) ? App->scene->scene->GetStaticCulledShadowCasters(subFrustums[i].planes) : App->scene->scene->GetDynamicCulledShadowCasters(subFrustums[i].planes);

		for (GameObject* go : gameObjects) {
			ComponentBoundingBox* componentBBox = go->GetComponent<ComponentBoundingBox>();
			if (componentBBox) {
				AABB boundingBox = componentBBox->GetWorldAABB();
				OBB orientedBoundingBox = boundingBox.Transform(lightOrientation.Inverted());
				lightAABB.Enclose(orientedBoundingBox.MinimalEnclosingAABB());
			}
		}

		float3 minPoint = lightAABB.minPoint;
		float3 maxPoint = lightAABB.maxPoint;
		float3 position = lightOrientation.RotatePart() * float3((maxPoint.x + minPoint.x) * 0.5f, ((maxPoint.y + minPoint.y) * 0.5f), minPoint.z);

		subFrustums[i].orthographicFrustum.SetOrthographic((maxPoint.x - minPoint.x), (maxPoint.y - minPoint.y));
		subFrustums[i].orthographicFrustum.SetUp(transform->GetUp());
		subFrustums[i].orthographicFrustum.SetFront(transform->GetFront());
		subFrustums[i].orthographicFrustum.SetPos(position);
		subFrustums[i].orthographicFrustum.SetViewPlaneDistances(0.0f, (maxPoint.z - minPoint.z));
	}

	dirty = false;
}

void LightFrustum::DrawOrthographicGizmos(int idx) {
	
	if (idx == INT_MAX) {
		for (unsigned i = 0; i < numberCascades; i++) {
			dd::frustum(subFrustums[i].orthographicFrustum.ViewProjMatrix().Inverted(), subFrustums[i].color);
		}
	} else if (idx >= 0 && idx < numberCascades) {
		dd::frustum(subFrustums[idx].orthographicFrustum.ViewProjMatrix().Inverted(), subFrustums[idx].color);
	}
}

void LightFrustum::DrawPerspectiveGizmos(int idx) {
	if (idx == INT_MAX) {
		for (unsigned i = 0; i < numberCascades; i++) {
			dd::frustum(subFrustums[i].perspectiveFrustum.ViewProjMatrix().Inverted(), subFrustums[i].color);
		}
	} else if (idx >= 0 && idx < numberCascades) {
		dd::frustum(subFrustums[idx].perspectiveFrustum.ViewProjMatrix().Inverted(), subFrustums[idx].color);
	}
}

Frustum LightFrustum::GetOrthographicFrustum(unsigned int i) const {
	return subFrustums[i].orthographicFrustum;
}

Frustum LightFrustum::GetPersepectiveFrustum(unsigned int i) const {
	return subFrustums[i].perspectiveFrustum;
}

const std::vector<LightFrustum::FrustumInformation>& LightFrustum::GetSubFrustums() const {
	return subFrustums;
}

LightFrustum::FrustumInformation& LightFrustum::operator[](int i) {
	
	assert(i < 0 || i > numberCascades && "Out of range");

	return subFrustums[i];

}

void LightFrustum::Invalidate() {
	dirty = true;
}
