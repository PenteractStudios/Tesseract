#include "ComponentSphereCollider.h"

#include "Application.h"
#include "GameObject.h"
#include "Modules/ModulePhysics.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleTime.h"
#include "Components/ComponentBoundingBox.h"

void ComponentSphereCollider::Init() {
	ComponentBoundingBox* boundingBox = GetOwner().GetComponent<ComponentBoundingBox>();
	if (boundingBox) {
		radius = boundingBox->GetWorldOBB().HalfSize().MaxElement();
		centerOffset = boundingBox->GetWorldOBB().CenterPoint() - GetOwner().GetComponent<ComponentTransform>()->GetGlobalPosition();
	}
	if (App->time->IsGameRunning() && !rigidBody) App->physics->CreateSphereRigidbody(this);
}

void ComponentSphereCollider::DrawGizmos() {
	if (IsActiveInHierarchy()) {
		ComponentTransform* ownerTransform = GetOwner().GetComponent<ComponentTransform>();
		dd::sphere(ownerTransform->GetGlobalPosition() + ownerTransform->GetGlobalRotation()*centerOffset, dd::colors::Green, radius);
	}
}

void ComponentSphereCollider::OnEditorUpdate() {
	if (ImGui::Checkbox("Is Trigger", &isTrigger) && App->time->IsGameRunning()) {
		rigidBody->setCollisionFlags(isTrigger ? btCollisionObject::CF_NO_CONTACT_RESPONSE : 0);
		rigidBody->setMassProps(isTrigger ? 0.f : mass, rigidBody->getLocalInertia());
	}
	if (!isTrigger) {
		if (ImGui::DragFloat("Mass", &mass, App->editor->dragSpeed3f, 0.0f, 100.f) && App->time->IsGameRunning()) {
			rigidBody->setMassProps(mass, btVector3(0, 0, 0));
		}
	}
	if (ImGui::DragFloat("Radius", &radius, App->editor->dragSpeed3f, 0.0f, inf) && App->time->IsGameRunning()) {
		((btSphereShape*) rigidBody->getCollisionShape())->setUnscaledRadius(radius);
	}
	if (ImGui::DragFloat3("Center Offset", centerOffset.ptr(), App->editor->dragSpeed2f, -inf, inf) && App->time->IsGameRunning()) {
		float3 position = GetOwner().GetComponent<ComponentTransform>()->GetGlobalPosition();
		Quat rotation = GetOwner().GetComponent<ComponentTransform>()->GetGlobalRotation();
		rigidBody->setCenterOfMassTransform(btTransform(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w), btVector3(position.x, position.y, position.z)) * btTransform(btQuaternion::getIdentity(), btVector3(centerOffset.x, centerOffset.y, centerOffset.z)));
	}
	if (ImGui::Checkbox("Freeze rotation", &freezeRotation) && App->time->IsGameRunning()) {
		motionState.freezeRotation = freezeRotation;
	}

}

void ComponentSphereCollider::OnCollision() {
	// TODO: Send event...
}