#include "ComponentCapsuleCollider.h"

#include "Application.h"
#include "GameObject.h"
#include "Modules/ModulePhysics.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleTime.h"
#include "Components/ComponentBoundingBox.h"

#define JSON_TAG_MASS "mass"
#define JSON_TAG_RADIUS "radius"
#define JSON_TAG_HEIGHT "height"
#define JSON_TAG_CENTER_OFFSET "centerOffset"
#define JSON_TAG_FREEZE_ROTATION "freezeRotation"
#define JSON_TAG_IS_TRIGGER "isTrigger"

void ComponentCapsuleCollider::Init() {
	if (!centerOffset.IsFinite()) {
		ComponentBoundingBox* boundingBox = GetOwner().GetComponent<ComponentBoundingBox>();
		if (boundingBox) {

		}
	}
	if (App->time->IsGameRunning() && !rigidBody) App->physics->CreateCapsuleRigidbody(this);
}

void ComponentCapsuleCollider::DrawGizmos() {
	if (IsActiveInHierarchy()) {
		ComponentTransform* ownerTransform = GetOwner().GetComponent<ComponentTransform>();	
		
	}
}

void ComponentCapsuleCollider::OnEditorUpdate() {
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
		((btCapsuleShape*) rigidBody->getCollisionShape())->setImplicitShapeDimensions(btVector3(radius, height, 0));
	}
