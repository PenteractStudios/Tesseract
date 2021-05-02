#include "ComponentSphereCollider.h"

#include "Application.h"
#include "GameObject.h"
#include "Modules/ModulePhysics.h"
#include "Components/ComponentBoundingBox.h"

void ComponentSphereCollider::Init() {
	ComponentBoundingBox* boundingBox = GetOwner().GetComponent<ComponentBoundingBox>();
	if (boundingBox) {
		radius = boundingBox->GetWorldOBB().HalfSize().MaxElement();
		centerOffset = boundingBox->GetWorldOBB().CenterPoint() - GetOwner().GetComponent<ComponentTransform>()->GetGlobalPosition();
	}
}

void ComponentSphereCollider::DrawGizmos() {
	if (IsActiveInHierarchy()) {
		ComponentTransform* ownerTransform = GetOwner().GetComponent<ComponentTransform>();
		dd::sphere(ownerTransform->GetGlobalPosition() + ownerTransform->GetGlobalRotation()*centerOffset, dd::colors::Green, radius);
	}
}

void ComponentSphereCollider::OnEditorUpdate() {
}

void ComponentSphereCollider::OnCollision() {
}