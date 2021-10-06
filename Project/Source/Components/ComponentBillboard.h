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

	TESSERACT_ENGINE_API void Play();
	TESSERACT_ENGINE_API void Stop();

	// Getters
	TESSERACT_ENGINE_API float GetBillboardLifetime() const;
	TESSERACT_ENGINE_API bool GetPlayOnAwake() const;
	TESSERACT_ENGINE_API float GetCurrentFrame() const;
	TESSERACT_ENGINE_API float3 GetTextureIntensity() const;
	TESSERACT_ENGINE_API float GetAnimationCycles() const;
	TESSERACT_ENGINE_API bool GetAnimationLoop() const;
	TESSERACT_ENGINE_API float GetColorCycles() const;
	TESSERACT_ENGINE_API bool GetColorLoop() const;

	// Setters
	TESSERACT_ENGINE_API void SetBillboardLifetime(float _billboardLifetime);
	TESSERACT_ENGINE_API void SetPlayOnAwake(bool _playOnAwake);
	TESSERACT_ENGINE_API void SetCurrentFrame(float _currentFrame);
	TESSERACT_ENGINE_API void SetTextureIntensity(float3 _textureIntensity);
	TESSERACT_ENGINE_API void SetAnimationCycles(float _animationSpeed);
	TESSERACT_ENGINE_API void SetAnimationLoop(bool _animationLoop);
	TESSERACT_ENGINE_API void SetColorCycles(float _colorCycles);
	TESSERACT_ENGINE_API void SetColorLoop(bool _colorLoop);

private:
	float4x4 modelStretch = float4x4::identity;
	float3 initPos = float3::zero;
	float3 previousPos = float3::zero;
	float3 direction = float3::zero;

	bool isStarted = false;
	bool isPlaying = false;
	float time = 0.0f;
	float currentFrame = 0.0f;
	float colorFrame = 0.0f;

	// General
	float billboardLifetime = 5.0f;
	bool playOnAwake = false;

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
	float animationCycles = 1.0f;
	bool animationLoop = true;

	// Color over Lifetime
	bool colorOverLifetime = false;
	float colorCycles = 1.0f;
	bool colorLoop = true;
	ImGradient* gradient = nullptr;
	ImGradientMark* draggingGradient = nullptr;
	ImGradientMark* selectedGradient = nullptr;
};
