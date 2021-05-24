#pragma once

#include "Components/Component.h"

#include "Math/float3.h"

class ComponentAgent : public Component {
public:
	REGISTER_COMPONENT(ComponentAgent, ComponentType::AGENT, false); // Refer to ComponentType for the Constructor
	~ComponentAgent();

	void Update() override;
	void OnEditorUpdate() override;
	void OnEnable() override;
	void OnDisable() override;
	void Save(JsonValue jComponent) const override;
	void Load(JsonValue jComponent) override;

	TESSERACT_ENGINE_API void SetMoveTarget(float3 newTargetPosition, bool usePathfinding = true); // This will set the parameters of the Agent to move to the target position
	TESSERACT_ENGINE_API void SetMaxSpeed(float newSpeed);
	TESSERACT_ENGINE_API void SetMaxAcceleration(float newAcceleration);

	void AddAgentToCrowd();
	void RemoveAgentFromCrowd();

private:
	unsigned int targetPolygon = 0;
	float3 targetPosition = float3::zero;
	int agentId = -1;

	float maxSpeed = 5.0f;
	float maxAcceleration = 8.0f;
};
