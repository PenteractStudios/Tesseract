#pragma once
#include "Component.h"

#include "Utils/Pool.h"
#include "Utils/UID.h"
#include "Utils/Collider.h"

#include "Math/float2.h"
#include "Math/float3.h"
#include "Math/float4.h"
#include "Math/float4x4.h"
#include "Math/Quat.h"

#include <vector>

class ComponentTransform;
class ParticleModule;
class btRigidBody;
class ParticleMotionState;
class ImGradient;
class ImGradientMark;

enum WorldLayers;

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

enum class ParticleRenderAlignment {
	VIEW,
	WORLD,
	LOCAL,
	FACING,
	VELOCITY
};

enum class RandomMode {
	CONST,
	CONST_MULT
};

class ComponentParticleSystem : public Component {
public:
	struct Particle {
		float3 initialPosition = float3(0.0f, 0.0f, 0.0f);
		float3 position = float3(0.0f, 0.0f, 0.0f);
		Quat rotation = Quat(0.0f, 0.0f, 0.0f, 0.0f);
		float3 scale = float3(0.1f, 0.1f, 0.1f);
		float3 direction = float3(0.0f, 0.0f, 0.0f);

		float speed = 0.0f;
		float life = 0.0f;
		float initialLife = 0.0f;
		float currentFrame = 0.0f;
		float gravityTime = 0.0f;

		float3 emitterPosition = float3::zero;
		float3 emitterDirection = float3::zero;

		// Collider
		ParticleMotionState* motionState = nullptr;
		btRigidBody* rigidBody = nullptr;
		ComponentParticleSystem* emitter = nullptr;
		Collider col {this, typeid(Particle)};
		float radius = 0;
	};

	REGISTER_COMPONENT(ComponentParticleSystem, ComponentType::PARTICLE, false);

	~ComponentParticleSystem();

	void Init() override;
	void Update() override;
	void DrawGizmos() override;
	void OnEditorUpdate() override;
	void Load(JsonValue jComponent) override;
	void Save(JsonValue jComponent) const override;

	void CreateParticles();
	void SpawnParticles();
	void SpawnParticleUnit();

	void InitParticlePosAndDir(Particle* currentParticle);
	void InitParticleRotation(Particle* currentParticle);
	void InitParticleScale(Particle* currentParticle);
	void InitParticleSpeed(Particle* currentParticle);
	void InitParticleLife(Particle* currentParticle);
	void InitStartDelay();
	void InitStartRate();

	TESSERACT_ENGINE_API void UpdatePosition(Particle* currentParticle);
	void UpdateRotation(Particle* currentParticle);
	void UpdateScale(Particle* currentParticle);
	void UpdateLife(Particle* currentParticle);
	void UpdateGravityDirection(Particle* currentParticle);

	TESSERACT_ENGINE_API void KillParticle(Particle* currentParticle);
	void UndertakerParticle();
	void DestroyParticlesColliders();

	void Draw();
	void ImGuiParticlesEffect();

	TESSERACT_ENGINE_API void Play();
	TESSERACT_ENGINE_API void Restart();
	TESSERACT_ENGINE_API void Stop();
	TESSERACT_ENGINE_API void PlayChildParticles();
	TESSERACT_ENGINE_API void RestartChildParticles();
	TESSERACT_ENGINE_API void StopChildParticles();

public:
	WorldLayers layer;
	int layerIndex = 5;
	float radius = .25f;

private:
	Pool<Particle> particles;
	std::vector<Particle*> deadParticles;
	unsigned particleSpawned = 0;
	bool executer = false;
	bool isPlaying = false;

	float3 cameraDir = {0.f, 0.f, 0.f};
	float emitterTime = 0.0f;
	float restDelayTime = 0.f;
	float restParticlesPerSecond = 0.0f;

	// Gizmo
	bool drawGizmo = true;

	// Particle System
	float duration = 5.0f; // Emitter duration
	bool looping = false;
	RandomMode startDelayRM = RandomMode::CONST;
	float2 startDelay = {0.0f, 0.0f}; // Start Delay
	RandomMode lifeRM = RandomMode::CONST;
	float2 life = {5.0f, 5.0f}; // Start life
	RandomMode speedRM = RandomMode::CONST;
	float2 speed = {1.3f, 1.3f}; // Start speed
	RandomMode rotationRM = RandomMode::CONST;
	float2 rotation = {0.0f, 0.0f}; // Start rotation
	RandomMode scaleRM = RandomMode::CONST;
	float2 scale = {1.0f, 1.0f}; // Start scale
	bool reverseEffect = false;
	RandomMode reverseDistanceRM = RandomMode::CONST;
	float2 reverseDistance = {5.0f, 5.0f};
	unsigned maxParticles = 100;

	// Emision
	bool attachEmitter = true;
	RandomMode particlesPerSecondRM = RandomMode::CONST;
	float2 particlesPerSecond = {10.0f, 10.0f};

	// Gravity
	bool gravityEffect = false;
	RandomMode gravityFactorRM = RandomMode::CONST;
	float2 gravityFactor = {0.0f, 0.0f};

	// Shape
	ParticleEmitterType emitterType = ParticleEmitterType::CONE;
	// -- Cone
	float coneRadiusUp = 1.0f;
	float coneRadiusDown = 0.5f;
	bool randomConeRadiusDown = false;
	bool randomConeRadiusUp = false;

	// Rotation over Lifetime
	bool rotationOverLifetime = false;
	RandomMode rotationFactorRM = RandomMode::CONST;
	float2 rotationFactor = {0.0f, 0.0f};

	// Size over Lifetime
	bool sizeOverLifetime = false;
	RandomMode scaleFactorRM = RandomMode::CONST;
	float2 scaleFactor = {0.0f, 0.0f};

	// Color over Lifetime
	bool colorOverLifetime = false;
	ImGradient* gradient = nullptr;
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
	ParticleRenderAlignment renderAlignment = ParticleRenderAlignment::VIEW;
	bool flipTexture[2] = {false, false};

	// Collision
	bool collision = false;
};