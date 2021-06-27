#pragma once

#include "Component.h"

#include "Math/float3.h"
#include "DetourTileCache/DetourTileCache.h"

class ComponentObstacle : public Component {
public:
	REGISTER_COMPONENT(ComponentObstacle, ComponentType::OBSTACLE, false); // Refer to ComponentType for the Constructor
	~ComponentObstacle();

	void Init() override;							// Add Obstacle to Mesh
	void Update() override;							// If position changed, reposition by Adding Obstacle
	void OnEditorUpdate() override;					// Radius and Height can be udpated
	void OnEnable() override;						// Calls AddObstacle
	void OnDisable() override;						// Calls RemoveObstacle
	void Save(JsonValue jComponent) const override; // Serialize
	void Load(JsonValue jComponent) override;		// Deserialize

	TESSERACT_ENGINE_API void AddObstacle();
	TESSERACT_ENGINE_API void RemoveObstacle();

	TESSERACT_ENGINE_API void SetRadius(float newRadius);
	TESSERACT_ENGINE_API void SetHeight(float newHeight);
	TESSERACT_ENGINE_API void SetBoxSize(float3 size);
	TESSERACT_ENGINE_API void SetObstacleType(ObstacleType newType);
	TESSERACT_ENGINE_API void ResetSize();

private:
	dtObstacleRef* obstacleReference = nullptr;
	float3 boxSize = float3::one;
	float3 currentPosition = float3::zero;
	float3 currentRotation = float3::zero;
	int obstacleType = ObstacleType::DT_OBSTACLE_CYLINDER;
	bool mustBeDrawnGizmo = true;

private:
	void SetDrawGizmo(bool value);
};
