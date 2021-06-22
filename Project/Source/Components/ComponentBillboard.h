#pragma once
#include "Component.h"

#include "ComponentParticleSystem.h"
#include "Math/float4.h"
#include "Math/float2.h"
#include "Math/float4x4.h"
#include "Math/Quat.h"

class ComponentBillboard : public Component {
public:
	REGISTER_COMPONENT(ComponentBillboard, ComponentType::BILLBOARD, false);

	void OnEditorUpdate() override;
	void Load(JsonValue jComponent) override;
	void Save(JsonValue jComponent) const override;
	void Init() override;
	void Update() override;
	void Draw();

private:
	UID textureID = 0; // ID of the image

	BillboardType billboardType = BillboardType::NORMAL;

	float4x4 modelStretch = float4x4::identity;
	float3 initPos = float3::zero;
	float3 previousPos = float3::zero;
	float3 direction = float3::zero;

	float currentFrame = 0.0f;
	float colorFrame = 0.0f;

	// General Options
	bool isRandomFrame = false;

	// Texture Sheet Animation
	unsigned Xtiles = 1;
	unsigned Ytiles = 1;
	float animationSpeed = 0.0f;

	// Color
	float3 initC = float3::one;
	float3 finalC = float3::one;
	float startTransition = 0.0f;
	float endTransition = 0.0f;

	// Texture Options
	bool flipTexture[2] = {false, false};
};
