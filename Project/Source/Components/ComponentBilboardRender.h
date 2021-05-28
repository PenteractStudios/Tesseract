#pragma once
#include "Component.h"

#include "Math/float4.h"
#include "Math/float2.h"
#include "Math/float4x4.h"
#include "Math/Quat.h"
class ComponentBilboardRender : public Component {
public:
	REGISTER_COMPONENT(ComponentBilboardRender, ComponentType::BILBOARD_RENDER, false);
	void Update() override;
	void Init() override;
	void DrawGizmos() override;
	void OnEditorUpdate() override;
	void Load(JsonValue jComponent) override;
	void Save(JsonValue jComponent) const override;
	void Draw();

private:
	UID textureID = 0; // ID of the image
	UID shaderID = 0;  // ID of the shader

	float3 initC = float3::one;
	float3 finalC = float3::one;
	float4 color = float4::one; // Color used as default tainter

	unsigned Xtiles = 1;
	unsigned Ytiles = 1;

	bool isRandomFrame = false;

	float currentFrame = 0.0f;
	float animationSpeed = 0.0f;
	float colorFrame = 0.0f;
};
