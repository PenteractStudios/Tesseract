#pragma once
#include "Component.h"

#include "Utils/Pool.h"
#include "Utils/UID.h"
#include "Math/float4.h"
#include "Math/float2.h"
#include "Math/float4x4.h"
#include "Math/Quat.h"

class ComponentTrail : public Component {
public:
	struct Quad {
		float4x4 model = float4x4::identity;
		float4x4 modelStretch = float4x4::identity;

		float3 initialPosition = float3(0.0f, 0.0f, 0.0f);
		float3 position = float3(0.0f, 0.0f, 0.0f);
		//float3 direction = float3(0.0f, 0.0f, 0.0f);
		float3 scale = float3(0.1f, 0.1f, 0.1f);

		Quat rotation = Quat(0.0f, 0.0f, 0.0f, 0.0f);

		int index = 0;

		float quadInfo[30] = {0.0f};
		float life = 0.0f;
		float currentFrame = 0.0f;
		float colorFrame = 0.0f;
	};

	REGISTER_COMPONENT(ComponentTrail, ComponentType::TRAIL, false);

	void Init() override;
	void Update() override;
	void OnEditorUpdate() override;
	void Load(JsonValue jComponent) override;
	void Save(JsonValue jComponent) const override;

	void Draw();
	void UpdateVerticesPosition();
	void InsertVertex(Quad* currentQuad, float3 vertex);
	void InsertTextureCoords(Quad* currentQuad);
	void CreateQuads(unsigned nQuads);
	void SpawnQuad(Quad* currentQuad);
	void UpdateQuads();
	void UpdateLife(Quad* currentQuad);
	void DeleteQuads();
	void EditTextureCoords();
	void ResetColor();

private:
	unsigned int quadVBO;
	UID textureID = 0; // ID of the image

	int nTextures = 1;
	int quadsCreated = 0;
	int trailQuads = 50;
	int maxVertices = 1500;
	int trianglesCreated = 0;
	int textureCreated = 0;
	int maxQuads = 100;

	float nRepeats = 1;
	float width = 0.1f;
	float timePoint = 1.0f;
	float minDistance = 2.0f;
	float verticesPosition[1500] = {0.0f};
	float textureCords[600] = {0.0f};
	float scaleFactor = 1.0f;
	float colorFrame = 0.0f;
	float colorSpeed = 0.0f;
	float quadLife = 10.0f;

	float3 currentPosition = float3(0, 0, 0);
	float3 previousPosition = float3(0, 0, 0);
	float3 previousVectorUp = float3(0, 0, 0);
	float3 currentPositionUp = float3(0, 0, 0);
	float3 currentPositionDown = float3(0, 0, 0);
	float3 previousPositionUp = float3(0, 0, 0);
	float3 previousPositionDown = float3(0, 0, 0);

	float4 initC = float4::one;
	float4 mediumC = float4::one;
	float4 finalC = float4::one;

	bool isStarted = false;
	bool colorOverTrail = false;
	bool stop = false;
	Quad quads[100];
	//std::vector<Quad*> quads;
	std::vector<Quad*> deadQuads;
};
