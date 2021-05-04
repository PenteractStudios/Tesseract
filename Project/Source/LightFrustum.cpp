#include "LightFrustum.h"

#include "Application.h"
#include "GameObject.h"
#include "Modules/ModuleScene.h"
#include "Globals.h"

#include "Geometry/Plane.h"

void LightFrustum::ReconstructFrustum() {

	float3 lightDir = float3::unitZ;
	float3 lightUp = float3::unitY;
	Quat lightOrientation = Quat::LookAt(-float3::unitZ, lightDir, float3::unitY, lightUp);

	AABB lightAABB;
	lightAABB.SetNegativeInfinity();

	for (const GameObject& go : App->scene->scene->gameObjects) {
		int layerMask = go.GetLayerMask();
		if ((layerMask & static_cast<int>(LayerMask::CAST_SHADOW)) != 0 && go.HasComponent<ComponentMeshRenderer>()) {
			ComponentBoundingBox* componentBBox = go.GetComponent<ComponentBoundingBox>();
			if (componentBBox) {
				//AABB bouding_box = AABB(componentBBox->GetWorldAABB());
				AABB boundingBox = componentBBox->GetWorldAABB();
				OBB orientedBoundingBox = boundingBox.Transform(lightOrientation.Inverted());
				lightAABB.Enclose(boundingBox.MinimalEnclosingAABB());
			}
		}
	}

	float3 minPoint = lightAABB.minPoint;
	float3 maxPoint = lightAABB.maxPoint;
	
	float3 position = float3((maxPoint.x - minPoint.x) * 0.5, ((maxPoint.y - minPoint.y) * 0.5), minPoint.z);

	frustum.SetOrthographic((maxPoint.x - minPoint.z), (maxPoint.y - minPoint.y));
	frustum.SetPos(lightOrientation * position);
	frustum.SetViewPlaneDistances(0, maxPoint.z - minPoint.z);
}
