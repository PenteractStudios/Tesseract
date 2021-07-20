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
		float animationSpeed = 1.0f;
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
	void InitParticleAnimationTexture(Particle* currentParticle);
	void InitStartDelay();
	void InitStartRate();

	TESSERACT_ENGINE_API void UpdatePosition(Particle* currentParticle);
	void UpdateRotation(Particle* currentParticle);
	void UpdateScale(Particle* currentParticle);
	void UpdateLife(Particle* currentParticle);
	void UpdateGravityDirection(Particle* currentParticle);

	TESSERACT_ENGINE_API void KillParticle(Particle* currentParticle);
	void UndertakerParticle(bool force = false);
	void DestroyParticlesColliders();
	void Draw();
	void ImGuiParticlesEffect();

	TESSERACT_ENGINE_API void Play();
	TESSERACT_ENGINE_API void Restart();
	TESSERACT_ENGINE_API void Stop();
	TESSERACT_ENGINE_API void PlayChildParticles();
	TESSERACT_ENGINE_API void RestartChildParticles();
	TESSERACT_ENGINE_API void StopChildParticles();
	float ChildParticlesInfo();

	//--- GETTERS ---

	// Particle System
	TESSERACT_ENGINE_API float GetDuration() const;
	TESSERACT_ENGINE_API bool GetIsLooping() const;
	TESSERACT_ENGINE_API float2 GetLife() const;
	TESSERACT_ENGINE_API float2 GetSpeed() const;
	TESSERACT_ENGINE_API float2 GetRotation() const;
	TESSERACT_ENGINE_API float2 GetScale() const;
	TESSERACT_ENGINE_API bool GetIsReverseEffect() const;
	TESSERACT_ENGINE_API float2 GetReserseDistance() const;
	TESSERACT_ENGINE_API unsigned GetMaxParticles() const;

	// Emision
	TESSERACT_ENGINE_API bool GetIsAttachEmmitter() const;
	TESSERACT_ENGINE_API float2 GetParticlesPerSecond() const;

	// Shape
	TESSERACT_ENGINE_API ParticleEmitterType GetEmmitterType() const;
	// -- Cone
	TESSERACT_ENGINE_API float GetConeRadiusUp() const;
	TESSERACT_ENGINE_API float GetConeRadiusDown() const;
	TESSERACT_ENGINE_API bool GetRandomConeRadiusDown() const;
	TESSERACT_ENGINE_API bool GetRandomConeRadiusUp() const;

	// Rotation over Lifetime
	TESSERACT_ENGINE_API bool GetRotationOverLifetime() const;
	TESSERACT_ENGINE_API float2 GetRotationFactor() const;

	// Size over Lifetime
	TESSERACT_ENGINE_API bool GetSizeOverLifetime() const;
	TESSERACT_ENGINE_API float2 GetScaleFactor() const;

	// Color over Lifetime
	TESSERACT_ENGINE_API bool GetColorOverLifetime() const;

	// Texture Sheet Animation
	TESSERACT_ENGINE_API unsigned GetXtiles() const;
	TESSERACT_ENGINE_API unsigned GetYtiles() const;
	TESSERACT_ENGINE_API float GetAnimationSpeed() const;
	TESSERACT_ENGINE_API bool GetIsRandomFrame() const;
	TESSERACT_ENGINE_API bool GetIsLoopAnimation() const;
	TESSERACT_ENGINE_API float GetNCycles() const;

	// Render
	TESSERACT_ENGINE_API BillboardType GetBillboardType() const;
	TESSERACT_ENGINE_API ParticleRenderMode GetRenderMode() const;
	TESSERACT_ENGINE_API ParticleRenderAlignment GetRenderAlignment() const;
	TESSERACT_ENGINE_API bool GetFlipXTexture() const;
	TESSERACT_ENGINE_API bool GetFlipYTexture() const;

	// Collision
	TESSERACT_ENGINE_API bool GetCollision() const;

	//--- SETTERS ---

	// Particle System
	TESSERACT_ENGINE_API void SetDuration(float _duration);
	TESSERACT_ENGINE_API void SetIsLooping(bool _isLooping);
	TESSERACT_ENGINE_API void SetLife(float2 _life);
	TESSERACT_ENGINE_API void SetSpeed(float2 _speed);
	TESSERACT_ENGINE_API void SetRotation(float2 _rotation);
	TESSERACT_ENGINE_API void SetScale(float2 _scale);
	TESSERACT_ENGINE_API void SetIsReverseEffect(bool _isReverse);
	TESSERACT_ENGINE_API void SetReserseDistance(float2 _reverseDistance);
	TESSERACT_ENGINE_API void SetMaxParticles(unsigned _maxParticle);

	// Emision
	TESSERACT_ENGINE_API void SetIsAttachEmmitter(bool _isAttachEmmiter);
	TESSERACT_ENGINE_API void SetParticlesPerSecond(float2 _particlesPerSecond);

	// Shape
	TESSERACT_ENGINE_API void SetEmmitterType(ParticleEmitterType _emmitterType);
	// -- Cone
	TESSERACT_ENGINE_API void SetConeRadiusUp(float _coneRadiusUp);
	TESSERACT_ENGINE_API void SetConeRadiusDown(float _coneRadiusUp);
	TESSERACT_ENGINE_API void SetRandomConeRadiusDown(bool _randomConeRadiusDown);
	TESSERACT_ENGINE_API void SetRandomConeRadiusUp(bool _randomConeRadiusUp);

	// Rotation over Lifetime
	TESSERACT_ENGINE_API void SetRotationOverLifetime(bool _rotationOverLifeTime);
	TESSERACT_ENGINE_API void SetRotationFactor(float2 _rotationFactor);

	// Size over Lifetime
	TESSERACT_ENGINE_API void SetSizeOverLifetime(bool _sizeOverLifeTime);
	TESSERACT_ENGINE_API void SetScaleFactor(float2 _scaleFactor);

	// Color over Lifetime
	TESSERACT_ENGINE_API void SetColorOverLifetime(bool _colorOverLifeTime);

	// Texture Sheet Animation
	TESSERACT_ENGINE_API void SetXtiles(unsigned _Xtiles);
	TESSERACT_ENGINE_API void SetYtiles(unsigned _Ytiles);
	TESSERACT_ENGINE_API void SetAnimationSpeed(float _animationSpeed);
	TESSERACT_ENGINE_API void SetIsRandomFrame(bool _randomFrame);
	TESSERACT_ENGINE_API void SetIsLoopAnimation(bool _loopAnimation);
	TESSERACT_ENGINE_API void SetNCycles(float _nCycles);

	// Render
	TESSERACT_ENGINE_API void SetBillboardType(BillboardType _bilboardType);
	TESSERACT_ENGINE_API void SetRenderMode(ParticleRenderMode _renderMode);
	TESSERACT_ENGINE_API void SetRenderAlignment(ParticleRenderAlignment _renderAligment);
	TESSERACT_ENGINE_API void SetFlipXTexture(bool _flipX);
	TESSERACT_ENGINE_API void SetFlipYTexture(bool _flipY);
	TESSERACT_ENGINE_API void SetIsSoft(bool _isSoft);

	// Collision
	TESSERACT_ENGINE_API void SetCollision(bool _collision);

public:
	WorldLayers layer;
	int layerIndex = 5;
	float radius = .25f;

private:
	Pool<Particle> particles;
	std::vector<Particle*> deadParticles;
	bool isPlaying = false;

	float3 cameraDir = {0.f, 0.f, 0.f};
	float emitterTime = 0.0f;
	float restDelayTime = 0.f;
	float restParticlesPerSecond = 0.0f;
	float particlesCurrentFrame = 0;

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
	bool loopAnimation = true;
	float nCycles = 1.0f;

	// Render
	UID textureID = 0;
	BillboardType billboardType = BillboardType::NORMAL;
	ParticleRenderMode renderMode = ParticleRenderMode::ADDITIVE;
	ParticleRenderAlignment renderAlignment = ParticleRenderAlignment::VIEW;
	bool flipTexture[2] = {false, false};
	bool isSoft = false;
	float softRange = 1.0f;

	// Collision
	bool collision = false;
};