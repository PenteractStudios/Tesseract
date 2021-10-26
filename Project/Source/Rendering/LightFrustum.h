#pragma once

#include "FrustumPlanes.h"

constexpr unsigned int MAX_NUMBER_OF_CASCADES = 4;
constexpr float MINIMUM_FAR_DISTANCE = 50.f;


enum class CascadeMode {
	FitToScene = 0,
	FitToCascade = 1
};

enum class ShadowCasterType {
	STATIC,
	DYNAMIC
};

class LightFrustum {
public:
	struct FrustumInformation {
		Frustum orthographicFrustum; // Light frustum
		Frustum perspectiveFrustum;	 // Camera frustum
		FrustumPlanes planes = FrustumPlanes();
		float3 color = float3(0.0f, 0.0f, 0.0f);
		float multiplier = 1.0f;
		float nearPlane = 0.001f;
		float farPlane = MINIMUM_FAR_DISTANCE;
	};

	LightFrustum();
	~LightFrustum() {}

	void UpdateFrustums();
	void ReconstructFrustum(ShadowCasterType shadowCasterType);
	void ConfigureFrustums(unsigned int value);

	void DrawOrthographicGizmos(unsigned int idx);
	void DrawPerspectiveGizmos(unsigned int idx);

	void SetNumberOfCascades(unsigned int value);
	unsigned int GetNumberOfCascades();
	Frustum GetOrthographicFrustum(unsigned int i) const;
	Frustum GetPersepectiveFrustum(unsigned int i) const;
	std::vector<LightFrustum::FrustumInformation>& GetSubFrustums();

	FrustumInformation& operator[](unsigned int i);

	void Invalidate();

private:
	bool dirty = true;
	unsigned int numberOfCascades = 1;
	CascadeMode mode = CascadeMode::FitToScene;
	std::vector<FrustumInformation> subFrustums;
};
