#include "LightFrustum.h"

#include "Application.h"
#include "GameObject.h"
#include "Modules/ModuleScene.h"
#include "Globals.h"

#include "debugdraw.h"

#include "Geometry/Plane.h"

LightFrustum::LightFrustum() {
	frustum.SetKind(FrustumSpaceGL, FrustumRightHanded);
}

void LightFrustum::ReconstructFrustum() {

	if (!dirty) return;

	GameObject* light = App->scene->scene->directionalLight;
	if (!light) return;

	ComponentTransform* transform = light->GetComponent<ComponentTransform>();
	assert(transform);

	Quat lightOrientation = Quat::LookAt(float3::unitZ, transform->GetFront(), float3::unitY, transform->GetUp());
	Quat lightRotation = transform->GetGlobalRotation();

	AABB lightAABB;
	lightAABB.SetNegativeInfinity();

	for (const GameObject& go : App->scene->scene->gameObjects) {
		const Mask& mask = go.GetMask();
		if ((mask.bitMask & static_cast<int>(MaskType::CAST_SHADOW)) != 0 && go.HasComponent<ComponentMeshRenderer>()) {
			ComponentBoundingBox* componentBBox = go.GetComponent<ComponentBoundingBox>();
			if (componentBBox) {
				AABB boundingBox = componentBBox->GetWorldAABB();
				//OBB orientedBoundingBox = boundingBox.Transform(lightRotation.Inverted());
				boundingBox.TransformAsAABB(lightRotation.Inverse());
				lightAABB.Enclose(boundingBox.MinimalEnclosingAABB());
			}
		}
	}

	float3 minPoint = lightAABB.minPoint;
	float3 maxPoint = lightAABB.maxPoint;
	
	float3 position = float3((maxPoint.x + minPoint.x) * 0.5f, ((maxPoint.y + minPoint.y) * 0.5f), minPoint.z);

	frustum.SetOrthographic((maxPoint.x - minPoint.z), (maxPoint.y - minPoint.y));
	frustum.SetUp(transform->GetUp());
	frustum.SetFront(transform->GetFront());
	frustum.SetPos(position);
	float absMaxZ = std::abs(maxPoint.z);
	float absMinZ = std::abs(minPoint.z);
	if (absMaxZ < absMinZ) std::swap(absMaxZ, absMinZ);
	frustum.SetViewPlaneDistances(0.0f, absMaxZ - absMinZ);

	dirty = true;
}

void LightFrustum::DrawGizmos() {
	
	dd::frustum(frustum.ViewProjMatrix().Inverted(), dd::colors::Green);
}

Frustum LightFrustum::GetFrustum() const {
	return frustum;
}

void LightFrustum::Invalidate() {
	dirty = true;
}
