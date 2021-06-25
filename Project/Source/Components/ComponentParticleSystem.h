#pragma once
#include "Component.h"

#include "Utils/Pool.h"
#include "Utils/UID.h"

#include "Math/float3.h"
#include "Math/float4.h"
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

	void InitParticlePosAndDir(Particle* currentParticle);
	void InitParticleScale(Particle* currentParticle);
	void InitParticleVelocity(Particle* currentParticle);
	void InitParticleLifetime(Particle* currentParticle);

	void UpdatePosition(Particle* currentParticle);
	void UpdateScale(Particle* currentParticle);
	void UpdateLife(Particle* currentParticle);
	void UndertakerParticle();

private:
	void CreateParticles();

private:
	Pool<Particle> particles;
	std::vector<Particle*> deadParticles;
	bool executer = false;
	unsigned particleSpawned = 0;

	float3 cameraDir = {0.f, 0.f, 0.f};
	float emitterTime = 0.0f;

	// Gizmo
	bool drawGizmo = true;

	// Control
	bool isPlaying = true;
	float startDelay = 0.f;
	float restDelayTime = 0.f;

	// Particle System
	float duration = 5.0f; // Emitter duration
	bool looping = false;
	float life = 5.0f;	   // Start life
	float velocity = 1.3f; // Start speed
	float scale = 1.0f;	   // Start size
	bool reverseEffect = false;
	float reverseDistance = 5.0f;
	unsigned maxParticles = 100;

	// Emision
	bool attachEmitter = false;

	// Shape
	ParticleEmitterType emitterType = ParticleEmitterType::CONE;
	// -- Cone
	float coneRadiusUp = 1.f;
	float coneRadiusDown = 0.5f;
	bool randomConeRadiusDown = false;
	bool randomConeRadiusUp = false;

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
};