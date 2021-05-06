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
	REGISTER_COMPONENT(ComponentTrail, ComponentType::TRAIL, false);

	void Update() override;
	void Init() override;
	void DrawGizmos() override;
	void OnEditorUpdate() override;
	void Load(JsonValue jComponent) override;
	void Save(JsonValue jComponent) const override;
	void Draw();
	void updtadePosition(float3 up, float3 down);
	void DuplicateComponent(GameObject& owner) override;
	void SpawnParticle();
	void UpdateVerticesPosition();
	void insertVertex(float3 vertex);

private:
	UID textureID = 0; // ID of the image
	UID shaderID = 0;  // ID of the shader

	float4 initC;
	int maxVertices = 6012;
	int trianglesCreated = 0;
	float width = 0.1;
	float timePoint = 1.0f;
	/*std::vector<float> particlesPosition;*/
	float verticesPosition[6012];

	float3 currentPosition;
	float3 previousPosition;

	float3 currentPositionUp;
	float3 currentPositionDown;
	float3 previousPositionUp = float3(0, 0, 0);
	float3 previousPositionDown = float3(0, 0, 0);

	bool isStarted = false;
};
