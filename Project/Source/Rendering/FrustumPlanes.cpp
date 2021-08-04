#include "FrustumPlanes.h"

#include "Utils/Leaks.h"

FrustumPlanes::FrustumPlanes() {

}

void FrustumPlanes::CalculateFrustumPlanes(const Frustum& frustum) {

	float3 pos = frustum.Pos();
	float3 up = frustum.Up().Normalized();
	float3 front = frustum.Front();
	float3 right = frustum.WorldRight().Normalized();
	float farDistance = frustum.FarPlaneDistance();
	float nearDistance = frustum.NearPlaneDistance();
	float aspectRatio = frustum.AspectRatio();
	float vFov = frustum.VerticalFov();

	float hFar = 2 * tan(vFov / 2) * farDistance;
	float wFar = hFar * aspectRatio;
	float hNear = 2 * tan(vFov / 2) * nearDistance;
	float wNear = hNear * aspectRatio;
	float3 farCenter = pos + front * farDistance;
	float3 nearCenter = pos + front * nearDistance;

	frustumPoints[0] = farCenter + (up * hFar / 2) - (right * wFar / 2);
	frustumPoints[1] = farCenter + (up * hFar / 2) + (right * wFar / 2);
	frustumPoints[2] = farCenter - (up * hFar / 2) - (right * wFar / 2);
	frustumPoints[3] = farCenter - (up * hFar / 2) + (right * wFar / 2);
	frustumPoints[4] = nearCenter + (up * hNear / 2) - (right * wNear / 2);
	frustumPoints[5] = nearCenter + (up * hNear / 2) + (right * wNear / 2);
	frustumPoints[6] = nearCenter - (up * hNear / 2) - (right * wNear / 2);
	frustumPoints[7] = nearCenter - (up * hNear / 2) + (right * wNear / 2);

	frustumPlanes[0] = frustum.LeftPlane();
	frustumPlanes[1] = frustum.RightPlane();
	frustumPlanes[2] = frustum.TopPlane();
	frustumPlanes[3] = frustum.BottomPlane();
	frustumPlanes[4] = frustum.FarPlane();
	frustumPlanes[5] = frustum.NearPlane();

}

bool FrustumPlanes::CheckIfInsideFrustumPlanes(const AABB& aabb, const OBB& obb) const {

	float3 points[8] {
		obb.pos - obb.r.x * obb.axis[0] - obb.r.y * obb.axis[1] - obb.r.z * obb.axis[2],
		obb.pos - obb.r.x * obb.axis[0] - obb.r.y * obb.axis[1] + obb.r.z * obb.axis[2],
		obb.pos - obb.r.x * obb.axis[0] + obb.r.y * obb.axis[1] - obb.r.z * obb.axis[2],
		obb.pos - obb.r.x * obb.axis[0] + obb.r.y * obb.axis[1] + obb.r.z * obb.axis[2],
		obb.pos + obb.r.x * obb.axis[0] - obb.r.y * obb.axis[1] - obb.r.z * obb.axis[2],
		obb.pos + obb.r.x * obb.axis[0] - obb.r.y * obb.axis[1] + obb.r.z * obb.axis[2],
		obb.pos + obb.r.x * obb.axis[0] + obb.r.y * obb.axis[1] - obb.r.z * obb.axis[2],
		obb.pos + obb.r.x * obb.axis[0] + obb.r.y * obb.axis[1] + obb.r.z * obb.axis[2]};
	
	for (const Plane& plane : frustumPlanes) {
		// check box outside/inside of frustum
		int out = 0;
		for (int i = 0; i < 8; i++) {
			out += (plane.normal.Dot(points[i]) - plane.d > 0 ? 1 : 0);
		}
		if (out == 8) return false;
	}

	// check frustum outside/inside box
	int out;
	out = 0;
	for (int i = 0; i < 8; i++) out += ((frustumPoints[i].x > aabb.MaxX()) ? 1 : 0);
	if (out == 8) return false;
	out = 0;
	for (int i = 0; i < 8; i++) out += ((frustumPoints[i].x < aabb.MinX()) ? 1 : 0);
	if (out == 8) return false;
	out = 0;
	for (int i = 0; i < 8; i++) out += ((frustumPoints[i].y > aabb.MaxY()) ? 1 : 0);
	if (out == 8) return false;
	out = 0;
	for (int i = 0; i < 8; i++) out += ((frustumPoints[i].y < aabb.MinY()) ? 1 : 0);
	if (out == 8) return false;
	out = 0;
	for (int i = 0; i < 8; i++) out += ((frustumPoints[i].z > aabb.MaxZ()) ? 1 : 0);
	if (out == 8) return false;
	out = 0;
	for (int i = 0; i < 8; i++) out += ((frustumPoints[i].z < aabb.MinZ()) ? 1 : 0);
	if (out == 8) return false;

	return true;
}
