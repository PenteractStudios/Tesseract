#pragma once
#include "Component.h"

#include "Utils/Pool.h"
#include "Utils/UID.h"
#include "Math/float4.h"
#include "Math/float2.h"
#include "Math/float4x4.h"
#include "Math/Quat.h"

#include <vector>

class ComponentTransform;
class ParticleModule;
class btRigidBody;
class ParticleMotionState;

enum WorldLayers;

enum class EmitterType {
	CONE,
	SPHERE,
	HEMISPHERE,
	DONUT,
	CIRCLE,
	RECTANGLE
};

enum class BillboardType {
	LOOK_AT,
	STRETCH,
	HORIZONTAL,
	VERTICAL
};

class ComponentParticleSystem : public Component {
public:
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
		float colorFrame = 0.0f;

		float3 emitterPosition = float3(0.0f, 0.0f, 0.0f);

		// Collider
		ParticleMotionState* motionState = nullptr;
		btRigidBody* rigidBody = nullptr;
	};

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

	TESSERACT_ENGINE_API void UpdatePosition(Particle* currentParticle);
	TESSERACT_ENGINE_API void UpdateVelocity(Particle* currentParticle);
	void UpdateScale(Particle* currentParticle);
	void UpdateLife(Particle* currentParticle);
	TESSERACT_ENGINE_API void KillParticle(Particle* currentParticle);
	void UndertakerParticle();

private:
	float4 GetTintColor() const; // Gets an additional color that needs to be applied to the image. Currently gets the color of the Button
	void CreateParticles(unsigned nParticles, float vel);

public:
	WorldLayers layer;
	int layerIndex = 0;

private:
	UID textureID = 0; // ID of the image

	EmitterType emitterType = EmitterType::CONE;
	BillboardType billboardType = BillboardType::LOOK_AT;

	Pool<Particle> particles;
	std::vector<Particle*> deadParticles;

	float3 cameraDir = {0.f, 0.f, 0.f};

	// General Options

	bool looping = false;
	bool isPlaying = true;
	bool isRandomFrame = false;
	bool sizeOverTime = false;
	bool reverseEffect = false;
	bool attachEmitter = false;
	bool executer = false;
	float scale = 5.f;
	float distanceReverse = 0.f;
	float startDelay = 0.f;
	float restDelayTime = 0.f;
	unsigned maxParticles = 100;
	unsigned particleSpawned = 0;
	float velocity = 0.1f;
	float particleLife = 5.f;
	float scaleFactor = 0.f;
	float coneRadiusUp = 1.f;
	float coneRadiusDown = 0.5f;
	bool collisions = false;

	// Texture Sheet Animation
	unsigned Xtiles = 1;
	unsigned Ytiles = 1;
	float animationSpeed = 0.0f;

	// Color Options
	float4 initC = float4::one;
	float4 finalC = float4::one;
	float startTransition = 0.0f;
	float endTransition = 0.0f;

	// Texture Options
	bool flipTexture[2] = {false, false};

	// Guizmos Options
	float kc = 1.0f; //Keep in one to avoid having denominator less than 1
	float kl = 0.045f;
	float kq = 0.0075f;
	float innerAngle = pi / 12;
	float outerAngle = pi / 6;
};