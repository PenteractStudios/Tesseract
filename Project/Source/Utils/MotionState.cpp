#include "MotionState.h"

#include "Globals.h"
#include "GameObject.h"
#include "Components/ComponentTransform.h"
#include "Math/float4x4.h"
#include "Math/float3x3.h"
#include "Math/Quat.h"

MotionState::MotionState(Component* componentCollider) : collider(componentCollider) {
	massCenterOffset = btTransform(btMatrix3x3::getIdentity(), btVector3(0, 10, 0));
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

	btTransform transform = centerOfMassWorldTrans * massCenterOffset;
	float3 parentPosition = collider->GetOwner().GetParent()->GetComponent<ComponentTransform>()->GetGlobalPosition();
	Quat parentRotation = collider->GetOwner().GetParent()->GetComponent<ComponentTransform>()->GetGlobalRotation().Inverted();

	// Set Local Position
	collider->GetOwner().GetComponent<ComponentTransform>()->SetPosition(parentRotation.Transform(((float3) transform.getOrigin() - parentPosition)));

	// Set Local Rotation
	if (!freezeRotation) {
		btQuaternion rotation;
		transform.getBasis().getRotation(rotation);
		collider->GetOwner().GetComponent<ComponentTransform>()->SetRotation(parentRotation * (Quat) rotation);
	}
}
