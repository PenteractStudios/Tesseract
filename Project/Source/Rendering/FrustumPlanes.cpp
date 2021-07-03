#include "FrustumPlanes.h"

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

	points[0] = farCenter + (up * hFar / 2) - (right * wFar / 2);
	points[1] = farCenter + (up * hFar / 2) + (right * wFar / 2);
	points[2] = farCenter - (up * hFar / 2) - (right * wFar / 2);
	points[3] = farCenter - (up * hFar / 2) + (right * wFar / 2);
	points[4] = nearCenter + (up * hNear / 2) - (right * wNear / 2);
	points[5] = nearCenter + (up * hNear / 2) + (right * wNear / 2);
	points[6] = nearCenter - (up * hNear / 2) - (right * wNear / 2);
	points[7] = nearCenter - (up * hNear / 2) + (right * wNear / 2);

	planes[0] = frustum.LeftPlane();
	planes[1] = frustum.RightPlane();
	planes[2] = frustum.TopPlane();
	planes[3] = frustum.BottomPlane();
	planes[4] = frustum.FarPlane();
	planes[5] = frustum.NearPlane();

}

bool FrustumPlanes::CheckIfInsideFrustum(const AABB& aabb, const OBB& obb) const {

	float3 points[8] {
		obb.pos - obb.r.x * obb.axis[0] - obb.r.y * obb.axis[1] - obb.r.z * obb.axis[2],
		obb.pos - obb.r.x * obb.axis[0] - obb.r.y * obb.axis[1] + obb.r.z * obb.axis[2],
		obb.pos - obb.r.x * obb.axis[0] + obb.r.y * obb.axis[1] - obb.r.z * obb.axis[2],
		obb.pos - obb.r.x * obb.axis[0] + obb.r.y * obb.axis[1] + obb.r.z * obb.axis[2],
		obb.pos + obb.r.x * obb.axis[0] - obb.r.y * obb.axis[1] - obb.r.z * obb.axis[2],
		obb.pos + obb.r.x * obb.axis[0] - obb.r.y * obb.axis[1] + obb.r.z * obb.axis[2],
		obb.pos + obb.r.x * obb.axis[0] + obb.r.y * obb.axis[1] - obb.r.z * obb.axis[2],
		obb.pos + obb.r.x * obb.axis[0] + obb.r.y * obb.axis[1] + obb.r.z * obb.axis[2]};

	for (const Plane& plane : planes) {
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
	for (int i = 0; i < 8; i++) out += ((points[i].x > aabb.MaxX()) ? 1 : 0);
	if (out == 8) return false;
	out = 0;
	for (int i = 0; i < 8; i++) out += ((points[i].x < aabb.MinX()) ? 1 : 0);
	if (out == 8) return false;
	out = 0;
	for (int i = 0; i < 8; i++) out += ((points[i].y > aabb.MaxY()) ? 1 : 0);
	if (out == 8) return false;
	out = 0;
	for (int i = 0; i < 8; i++) out += ((points[i].y < aabb.MinY()) ? 1 : 0);
	if (out == 8) return false;
	out = 0;
	for (int i = 0; i < 8; i++) out += ((points[i].z > aabb.MaxZ()) ? 1 : 0);
	if (out == 8) return false;
	out = 0;
	for (int i = 0; i < 8; i++) out += ((points[i].z < aabb.MinZ()) ? 1 : 0);
	if (out == 8) return false;

	return true;
}
