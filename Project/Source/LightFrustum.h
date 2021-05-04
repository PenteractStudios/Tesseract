#pragma once

#include "Geometry/Frustum.h"

class LightFrustum {
public:

	LightFrustum() {}
	~LightFrustum() {}
	void ReconstructFrustum();

private:

	Frustum frustum;
	float3 position;
	

};
