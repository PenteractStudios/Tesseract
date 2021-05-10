#pragma once

#include "Geometry/Frustum.h"

class LightFrustum {
public:

	LightFrustum() {}
	~LightFrustum() {}
	void ReconstructFrustum();

	void DrawGizmos();

	Frustum GetFrustum() const;

public:

	bool dirty = true;

private:

	Frustum frustum;
};
