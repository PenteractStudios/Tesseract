#pragma once

#include "Components/Component.h"
#include "Utils/MotionState.h"

#include "Math/float3.h"

class btRigidBody;
class ComponentSphereCollider : public Component {
public:
	/* --- Collider Type ---
	DYNAMIC = the object will respond to collisions, but not to user input (Such as modifying the transform)
	STATIC = the object will never move
	KINEMATIC = the object will not respond to collisions, but ilt will to user input
	TRIGGER = It is like static, but the collisions against it have no physical effect to the colliding object.
	*/
	enum class ColliderType {
		DYNAMIC,
		STATIC,
		KINEMATIC,
		TRIGGER
	};

public:
	REGISTER_COMPONENT(ComponentSphereCollider, ComponentType::SPHERE_COLLIDER, false); // Refer to ComponentType for the Constructor

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
	float mass = 1.f;
	float radius = 1.f;
	float3 centerOffset = float3::inf;
	bool freezeRotation = false;
	bool isTrigger = false;
	bool isStatic = false;
	bool isKinematic = false;
};
