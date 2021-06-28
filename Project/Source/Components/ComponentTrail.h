#pragma once
#include "Component.h"

#include "Utils/Pool.h"
#include "Utils/UID.h"

#include "Math/float4.h"
#include "Math/float2.h"
#include "Math/float4x4.h"
#include "Math/Quat.h"
#include "imgui_color_gradient.h"

#define MAX_VERTICES 1500

class ComponentTrail : public Component {
public:
	struct Quad {
		int index = 0;

		float quadInfo[30] = {0.0f};
		float life = 0.0f;
	};

	REGISTER_COMPONENT(ComponentTrail, ComponentType::TRAIL, false);

	~ComponentTrail();

	void Init() override;
	void Update() override;
	void OnEditorUpdate() override;
	void Load(JsonValue jComponent) override;
	void Save(JsonValue jComponent) const override;

	void Draw();
	void InsertVertex(Quad* currentQuad, float3 vertex);
	void InsertTextureCoords(Quad* currentQuad);
	void SpawnQuad(Quad* currentQuad);
	void UpdateQuads();
	void UpdateLife(Quad* currentQuad);
	void DeleteQuads();
	void EditTextureCoords();

private:
	unsigned int quadVBO;
	UID textureID = 0; // ID of the image

	float3 currentPosition = float3(0, 0, 0);
	float3 previousPosition = float3(0, 0, 0);
	float3 currentPositionUp = float3(0, 0, 0);
	float3 currentPositionDown = float3(0, 0, 0);
	float3 previousPositionUp = float3(0, 0, 0);
	float3 previousPositionDown = float3(0, 0, 0);

	bool isStarted = false;

	// Trail Info
	int nTextures = 1;
	int quadsCreated = 0;
	int trailQuads = 50;
	int maxVertices = 1500;
	int textureCreated = 0;
	const static int maxQuads = 100;

	float nRepeats = 1;
	float width = 0.1f;
	float timePoint = 1.0f;
	float textureCords[600] = {0.0f};
	float quadLife = 10.0f;

	// Color Settings
	ImGradient* gradient = nullptr;
	ImGradientMark* draggingGradient = nullptr;
	ImGradientMark* selectedGradient = nullptr;

	bool colorOverTrail = false;
	bool stop = false;

	Quad quads[maxQuads];
};
