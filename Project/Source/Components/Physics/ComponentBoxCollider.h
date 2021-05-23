#pragma once

#include "Components/Component.h"
#include "Modules/ModulePhysics.h"
#include "Utils/MotionState.h"

#include "Math/float3.h"
#include "Geometry/AABB.h"
#include "Geometry/OBB.h"

class btRigidBody;
class ComponentBoxCollider : public Component {
public:
	REGISTER_COMPONENT(ComponentBoxCollider, ComponentType::BOX_COLLIDER, false); // Refer to ComponentType for the Constructor

	// ------- Core Functions ------ //
	void Init() override;
	//void Update() override;
	void DrawGizmos() override;
	void OnEditorUpdate() override;
	void Save(JsonValue jComponent) const override;
	void Load(JsonValue jComponent) override;

	// ----- Collider Functions ---- //
	void OnCollision(); // Callback function for scripting
	void CalculateWorldBoundingBox();

public:
	btRigidBody* rigidBody = nullptr;
	MotionState motionState = MotionState(nullptr, float3::zero, false);
	ColliderType colliderType = ColliderType::DYNAMIC;
	int layerIndex = 4;
	WorldLayers layer = WorldLayers::WORLD_ELEMENTS;
	float mass = 1.f;
	float3 size = float3::one;
	AABB localAABB = {float3(0.5f), float3(0.5f)}; // Axis Aligned Bounding Box, local to the GameObject
	OBB worldOBB = {localAABB};
	float3 centerOffset = float3::inf;
	bool freezeRotation = false;
	bool drawGizmo = true;
};
