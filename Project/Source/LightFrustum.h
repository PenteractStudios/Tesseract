#pragma once

#include "Geometry/Frustum.h"

class LightFrustum {
public:
	LightFrustum();
	~LightFrustum() {}
	void ReconstructFrustum();

	void DrawGizmos();

	Frustum GetFrustum() const;

	void Invalidate();

private:
	bool dirty = true;
	Frustum frustum;
};
