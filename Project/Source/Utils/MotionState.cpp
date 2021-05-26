#include "MotionState.h"

#include "Globals.h"
#include "Application.h"
#include "GameObject.h"
#include "Modules/ModuleTime.h"
#include "Components/ComponentTransform.h"
#include "Math/float4x4.h"
#include "Math/float3x3.h"
#include "Math/Quat.h"

MotionState::MotionState(Component* componentCollider, float3 centerOffset, bool freezeRot)
	: collider(componentCollider), freezeRotation(freezeRot) {

	massCenterOffset = btTransform(btMatrix3x3::getIdentity(), btVector3(centerOffset.x, centerOffset.y, centerOffset.z)).inverse();
}

MotionState::~MotionState() {
}

void MotionState::getWorldTransform(btTransform& centerOfMassWorldTrans) const {
	float3 position = collider->GetOwner().GetComponent<ComponentTransform>()->GetGlobalPosition();
	Quat rotation = collider->GetOwner().GetComponent<ComponentTransform>()->GetGlobalRotation();

	centerOfMassWorldTrans = btTransform(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w), btVector3(position.x, position.y, position.z)) * massCenterOffset.inverse();
}

///synchronizes world transform from physics to user
///Bullet only calls the update of worldtransform for active objects
void MotionState::setWorldTransform(const btTransform& centerOfMassWorldTrans) {
	if (App->time->IsGameRunning()) {
		btTransform transform = centerOfMassWorldTrans * (freezeRotation? btTransform::getIdentity(): massCenterOffset);
		float3 parentScale = collider->GetOwner().GetParent()->GetComponent<ComponentTransform>()->GetGlobalScale();
		float3 parentPosition = collider->GetOwner().GetParent()->GetComponent<ComponentTransform>()->GetGlobalPosition();
		Quat parentRotation = collider->GetOwner().GetParent()->GetComponent<ComponentTransform>()->GetGlobalRotation().Inverted();

		// Set Local Position
		float3 position = (float3) (transform.getOrigin() +  (freezeRotation ? massCenterOffset.getOrigin() : btVector3(0, 0, 0)));
		collider->GetOwner().GetComponent<ComponentTransform>()->SetPosition(parentRotation.Transform(((position).Div(parentScale) - parentPosition)));

		// Set Local Rotation
		if (!freezeRotation) {
			btQuaternion rotation;
			transform.getBasis().getRotation(rotation);
			collider->GetOwner().GetComponent<ComponentTransform>()->SetRotation(parentRotation * (Quat) rotation);
		}
	}
}
