#pragma once

#include "Component.h"

#include "Geometry/Frustum.h"

class ComponentCamera : public Component {
public:
	REGISTER_COMPONENT(ComponentCamera, ComponentType::CAMERA, false); // Refer to ComponentType for the Constructor

	// ------- Core Functions ------ //
	void Init() override;
	void Update() override;
	void DrawGizmos() override;
	void OnEditorUpdate() override;
	void Save(JsonValue jComponent) const override;
	void Load(JsonValue jComponent) override;

	void UpdateFrustum();
	Frustum BuildDefaultFrustum() const; // Builds a frustum object, at origin, facing Z direction, with specified FOV.

	// ------- Getters ------ //
	TESSERACT_ENGINE_API Frustum* GetFrustum();

public:
	Frustum frustum = BuildDefaultFrustum(); // Frustum geometry of the camera
};
