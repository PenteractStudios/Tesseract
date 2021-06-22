#pragma once
#include "Component.h"

#include "Utils/Pool.h"
#include "Utils/UID.h"
#include "Math/float4.h"
#include "Math/float2.h"
#include "Math/float4x4.h"
#include "Math/Quat.h"
#include "imgui_color_gradient.h"

#include <vector>

enum class ParticleEmitterType {
	CONE,
	SPHERE,
	HEMISPHERE,
	DONUT,
	CIRCLE,
	RECTANGLE
};

enum class ParticleRenderMode {
	ADDITIVE,
	TRANSPARENT
};

enum class BillboardType {
	NORMAL,
	STRETCH,
	HORIZONTAL,
	VERTICAL
};

struct Particle {
	float4x4 model = float4x4::identity;
	float4x4 modelStretch = float4x4::identity;

	float3 initialPosition = float3(0.0f, 0.0f, 0.0f);
	float3 position = float3(0.0f, 0.0f, 0.0f);
	float3 direction = float3(0.0f, 0.0f, 0.0f);
	float3 scale = float3(0.1f, 0.1f, 0.1f);

	Quat rotation = Quat(0.0f, 0.0f, 0.0f, 0.0f);

	float velocity = 0.0f;
	float life = 0.0f;
	float currentFrame = 0.0f;

	float3 emitterPosition = float3(0.0f, 0.0f, 0.0f);
};

class ComponentParticleSystem : public Component {
public:
	REGISTER_COMPONENT(ComponentParticleSystem, ComponentType::PARTICLE, false);

	void Update() override;
	void Init() override;
	void DrawGizmos() override;
	void OnEditorUpdate() override;
	void Load(JsonValue jComponent) override;
	void Save(JsonValue jComponent) const override;

	void Draw();
	TESSERACT_ENGINE_API void Play();
	TESSERACT_ENGINE_API void Stop();
	void SpawnParticle();
	void killParticles();

	float3 CreatePosition();
	float3 CreateDirection();

	void UpdatePosition(Particle* currentParticle);
	void UpdateVelocity(Particle* currentParticle);
	void UpdateScale(Particle* currentParticle);
	void UpdateLife(Particle* currentParticle);
	void UndertakerParticle();

private:
	void CreateParticles(unsigned nParticles, float vel);

private:
	Pool<Particle> particles;
	std::vector<Particle*> deadParticles;
	bool executer = false;
	unsigned particleSpawned = 0;

	float3 cameraDir = {0.f, 0.f, 0.f};
	float emitterTime = 0.0f;

	// Control
	bool isPlaying = true;
	float startDelay = 0.f;
	float restDelayTime = 0.f;

	// Particle System
	float duration = 5.0f; // Emitter duration
	bool looping = false;
	float life = 5.f;	   // Start life
	float velocity = 1.3f; // Start speed
	float scale = 3.f;	   // Start size
	bool reverseEffect = false;
	float reverseDistance = 0.f;
	unsigned maxParticles = 100;

	// Emision
	bool attachEmitter = false;

	// Shape
	ParticleEmitterType emitterType = ParticleEmitterType::CONE;
	float coneRadiusUp = 1.f;
	float coneRadiusDown = 0.5f;

	// Size over Lifetime
	bool sizeOverLifetime = false;
	float scaleFactor = 0.f;

	// Color over Lifetime
	bool colorOverLifetime = false;
	ImGradient gradient;
	ImGradientMark* draggingGradient = nullptr;
	ImGradientMark* selectedGradient = nullptr;

	// Texture Sheet Animation
	unsigned Xtiles = 1;
	unsigned Ytiles = 1;
	float animationSpeed = 0.0f;
	bool isRandomFrame = false;

	// Render
	UID textureID = 0;
	BillboardType billboardType = BillboardType::NORMAL;
	ParticleRenderMode renderMode = ParticleRenderMode::ADDITIVE;
	bool flipTexture[2] = {false, false};

	// Guizmos Options
	float kc = 1.0f; //Keep in one to avoid having denominator less than 1
	float kl = 0.045f;
	float kq = 0.0075f;
	float innerAngle = pi / 12;
	float outerAngle = pi / 6;
};