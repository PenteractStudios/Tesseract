#include "ComponentSphereCollider.h"

#include "Application.h"
#include "GameObject.h"
#include "Modules/ModulePhysics.h"

void ComponentSphereCollider::Init() {
	motionState = MotionState(this);
	float4x4 transform = GetOwner().GetComponent<ComponentTransform>()->GetGlobalMatrix();
	rigidBody = App->physics->AddSphereBody(&motionState, radius, mass);
}

void ComponentSphereCollider::OnCollision() {
}
