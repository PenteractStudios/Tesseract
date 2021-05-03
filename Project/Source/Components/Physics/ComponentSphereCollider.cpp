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
	if (ImGui::DragFloat("Radius", &radius, App->editor->dragSpeed3f, 0.0f, inf) && App->time->IsGameRunning()) {
		App->physics->UpdateSphereRigidbody(this);
	}
	if (ImGui::DragFloat3("Center Offset", centerOffset.ptr(), App->editor->dragSpeed2f, -inf, inf) && App->time->IsGameRunning()) {
		App->physics->UpdateSphereRigidbody(this);
	}
}

void ComponentSphereCollider::OnCollision() {
	// TODO: Send event...
}