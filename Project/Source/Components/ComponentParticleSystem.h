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
	HORITZONTAL,
	VERTICAL
};

class ComponentParticleSystem : public Component {
public:
	struct Particle {
		float4x4 model = float4x4::identity;
		float4x4 modelStrech = float4x4::identity;

		float3 initialPosition = float3(0.0f, 0.0f, 0.0f);
		float3 position = float3(0.0f, 0.0f, 0.0f);
		float3 direction = float3(0.0f, 0.0f, 0.0f);
		float3 scale = float3(0.1f, 0.1f, 0.1f);

		Quat rotation = Quat(0.0f, 0.0f, 0.0f, 0.0f);

		float velocity = 0.0f;
		float life = 0.0F;
		float currentFrame = 0.0f;
		float colorFrame = 0.0f;
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
	float3 CreateVelocity();

private:
	float4 GetTintColor() const; // Gets an additional color that needs to be applied to the image. Currently gets the color of the Button
	void CreateParticles(unsigned nParticles, float vel);

private:
	UID textureID = 0; // ID of the image
	UID shaderID = 0;  // ID of the shader

	bool looping = false;
	bool isPlaying = true;
	bool alphaTransparency = false; // Enables Alpha Transparency of the image and the color
	bool isRandomFrame = false;
	bool randomDirection = false;
	//TODO USE THIS FEATURE
	bool sizeOverTime = false;

	float4 initC = float4::one;
	float4 finalC = float4::one;
	float4 color = float4::one; // Color used as default tainter
	float animationSpeed = 0.0f;
	float scale = 5;
	unsigned maxParticles = 100;
	float velocity = 0.1f;
	float kc = 1.0f; //Keep in one to avoid having denominator less than 1
	float kl = 0.045f;
	float kq = 0.0075f;
	float innerAngle = pi / 12;
	float outerAngle = pi / 6;
	float particleLife = 5;

	unsigned particleSpawned = 0;
	unsigned Xtiles = 1;
	unsigned Ytiles = 1;

	bool flipTexture[2] = {false, false};

	float3 cameraDir = {0.f, 0.f, 0.f};

private:
	EmitterType emitterType = EmitterType::CONE;
	BillboardType billboardType = BillboardType::LOOK_AT;

	Pool<Particle> particles;
	std::vector<Particle*> deadParticles;
	//TODO IMPLEMENT DRAWINSTANCE
	/*float3 particlesPosition[100];*/
};