#pragma once
#include "Component.h"

#include "ComponentParticleSystem.h"

#include "Math/float3.h"
#include "Math/float4x4.h"

class ImGradient;
struct ImGradientMark;

class ComponentBillboard : public Component {
public:
	REGISTER_COMPONENT(ComponentBillboard, ComponentType::BILLBOARD, false);

	~ComponentBillboard();

	void OnEditorUpdate() override;
	void Load(JsonValue jComponent) override;
	void Save(JsonValue jComponent) const override;
	void Init() override;
	void Update() override;
	void Draw();

	void ResetColor();

	// Getters
	TESSERACT_ENGINE_API float GetCurrentFrame() const;
	TESSERACT_ENGINE_API float3 GetTextureIntensity() const;
	TESSERACT_ENGINE_API float GetAnimationSpeed() const;

	// Setters
	TESSERACT_ENGINE_API void SetCurrentFrame(float _currentFrame);
	TESSERACT_ENGINE_API void SetIntensity(float3 _textureIntensity);
	TESSERACT_ENGINE_API void SetAnimationSpeed(float _animationSpeed);

private:
	float4x4 modelStretch = float4x4::identity;
	float3 initPos = float3::zero;
	float3 previousPos = float3::zero;
	float3 direction = float3::zero;

	float currentFrame = 0.0f;
	float colorFrame = 0.0f;

	// Render
	BillboardType billboardType = BillboardType::NORMAL;
	ParticleRenderMode renderMode = ParticleRenderMode::ADDITIVE;
	UID textureID = 0; // ID of the image
	float3 textureIntensity = {1.0f, 1.0f, 1.0f};

	bool isHorizontalOrientation = false;
	bool flipTexture[2] = {false, false};

	// Texture Sheet Animation
	unsigned Xtiles = 1;
	unsigned Ytiles = 1;
	float animationSpeed = 0.0f;

	// Color over Lifetime
	bool colorOverLifetime = false;
	float colorLifetime = 10.0f;
	ImGradient* gradient = nullptr;
	ImGradientMark* draggingGradient = nullptr;
	ImGradientMark* selectedGradient = nullptr;
};
