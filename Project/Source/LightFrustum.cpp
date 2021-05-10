#include "LightFrustum.h"

#include "Application.h"
#include "GameObject.h"
#include "Modules/ModuleScene.h"
#include "Globals.h"

#include "debugdraw.h"

#include "Geometry/Plane.h"

void LightFrustum::ReconstructFrustum() {

	GameObject* light = App->scene->scene->directionalLight;
	if (!light) return;

	ComponentTransform* transform = light->GetComponent<ComponentTransform>();
	assert(transform);

	Quat lightOrientation = Quat::LookAt(-float3::unitZ, transform->GetFront(), float3::unitY, transform->GetUp());

	AABB lightAABB;
	lightAABB.SetNegativeInfinity();

	for (const GameObject& go : App->scene->scene->gameObjects) {
		const Mask& mask = go.GetMask();
		if ((mask.bitMask & static_cast<int>(MaskType::CAST_SHADOW)) != 0 && go.HasComponent<ComponentMeshRenderer>()) {
			ComponentBoundingBox* componentBBox = go.GetComponent<ComponentBoundingBox>();
			if (componentBBox) {
				AABB boundingBox = componentBBox->GetWorldAABB();
				OBB orientedBoundingBox = boundingBox.Transform(lightOrientation.Inverted());
				lightAABB.Enclose(boundingBox.MinimalEnclosingAABB());
			}
		}
	}

	float3 minPoint = lightAABB.minPoint;
	float3 maxPoint = lightAABB.maxPoint;
	
	float3 position = float3((maxPoint.x - minPoint.x) * 0.5f, ((maxPoint.y - minPoint.y) * 0.5f), minPoint.z);

	frustum.SetOrthographic((maxPoint.x - minPoint.z), (maxPoint.y - minPoint.y));
	frustum.SetUp(float3::unitY);
	frustum.SetFront(float3::unitZ);
	frustum.SetPos(lightOrientation * position);
	frustum.SetViewPlaneDistances(0, maxPoint.z - minPoint.z);

}

void LightFrustum::DrawGizmos() {
	
	dd::frustum(frustum.ViewProjMatrix().Inverted(), dd::colors::Green);
}

Frustum LightFrustum::GetFrustum() const {
	return frustum;
}
