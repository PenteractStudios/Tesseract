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
	REGISTER_COMPONENT(ComponentTrail, ComponentType::TRAIL, false);

	void Init() override;
	void Update() override;
	void OnEditorUpdate() override;
	void Load(JsonValue jComponent) override;
	void Save(JsonValue jComponent) const override;

	void Draw();
	void UpdateVerticesPosition();
	void InsertVertex(float3 vertex);
	void InsertTextureCoords();
	void DeleteQuads();
	void EditTextureCoords();
	void ResetColor();

private:
	unsigned int quadVBO;
	UID textureID = 0; // ID of the image

	float3 currentPosition = float3(0, 0, 0);
	float3 previousPosition = float3(0, 0, 0);
	float3 currentPositionUp = float3(0, 0, 0);
	float3 currentPositionDown = float3(0, 0, 0);
	float3 previousPositionUp = float3(0, 0, 0);
	float3 previousPositionDown = float3(0, 0, 0);

	float verticesPosition[1500] = {0.0f};

	float textureCords[600] = {0.0f};
	int nTextures = 1;
	int maxVertices = 1500;
	int quadsCreated = 0;

	int trianglesCreated = 0;
	int textureCreated = 0;

	bool isStarted = false;
	float trailTime = 0.0f;

	// General Settings
	float width = 0.1f;
	int trailQuads = 50;

	int nRepeats = 1;

	float scaleFactor = 1.0f;

	// Color Settings
	ImGradient gradient;
	ImGradientMark* draggingGradient = nullptr;
	ImGradientMark* selectedGradient = nullptr;

	bool colorOverTrail = false;
	float colorLife = 0.0f; // Life (in seconds) to complete the Gradient Bar
};
