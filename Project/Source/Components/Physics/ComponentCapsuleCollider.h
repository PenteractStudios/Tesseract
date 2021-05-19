#pragma once

#include "Components/Component.h"
#include "Modules/ModulePhysics.h"
#include "Utils/MotionState.h"

#include "Math/float3.h"

class btRigidBody;

enum class CapsuleType {
	X,
	Y,
	Z
};

class ComponentCapsuleCollider : public Component {
public:
	REGISTER_COMPONENT(ComponentCapsuleCollider, ComponentType::CAPSULE_COLLIDER, false); // Refer to ComponentType for the Constructor

	// ------- Core Functions ------ //
	void Init() override;
	//void Update() override;
	void DrawGizmos() override;
	void OnEditorUpdate() override;
	void Save(JsonValue jComponent) const override;
	void Load(JsonValue jComponent) override;
	//void DuplicateComponent(GameObject& owner) override;

	// ----- Collider Functions ---- //
	void OnCollision();

public:
	btRigidBody* rigidBody = nullptr;
	MotionState motionState = MotionState(nullptr, float3(0, 0, 0), false);
	ColliderType colliderType = ColliderType::DYNAMIC;
	WorldLayers layer = WorldLayers::WORLD_ELEMENTS;
	float mass = 1.f;
	float radius = 1.f;
	float height = 1.f;
	float3 centerOffset = float3::inf;
	bool freezeRotation = false;
	CapsuleType type = CapsuleType::Y;

};
