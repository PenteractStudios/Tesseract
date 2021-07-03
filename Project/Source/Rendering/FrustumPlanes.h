#pragma once

#include "Geometry/Plane.h"
#include "Geometry/Frustum.h"
#include "Geometry/AABB.h"
#include "Geometry/OBB.h"

class FrustumPlanes {
public:
	FrustumPlanes();

	void CalculateFrustumPlanes(const Frustum& frustum);
	bool CheckIfInsideFrustumPlanes(const AABB& aabb, const OBB& obb) const;

	float3 points[8]; // 0: ftl, 1: ftr, 2: fbl, 3: fbr, 4: ntl, 5: ntr, 6: nbl, 7: nbr. (far/near, top/bottom, left/right).
	Plane planes[6];  // left, right, up, down, front, back
};
