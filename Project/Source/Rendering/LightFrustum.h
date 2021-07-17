#pragma once

#include "FrustumPlanes.h"

constexpr unsigned int NUM_CASCADE_FRUSTUM = 4;
constexpr float MINIMUM_FAR_DISTANE = 50.f;


enum class CascadeMode {
	FitToScene = 0,
	FitToCascade = 1
};

class LightFrustum {
public:
	struct FrustumInformation {
		Frustum orthographicFrustum; // Light frustum
		Frustum perspectiveFrustum;	 // Camera frustum
		FrustumPlanes planes = FrustumPlanes();
		float3 color = float3(0.0f, 0.0f, 0.0f);
		float multiplier = 1.0f;
	};

	LightFrustum();
	~LightFrustum() {}

	void UpdateFrustums();
	void ReconstructFrustum();

	void DrawGizmos();

	Frustum GetFrustum() const;

	void Invalidate();

private:
	bool dirty = true;
	CascadeMode mode = CascadeMode::FitToScene;
	FrustumInformation subFrustums[NUM_CASCADE_FRUSTUM];
};
