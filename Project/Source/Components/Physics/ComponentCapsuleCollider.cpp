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
			radius = (boundingBox->GetWorldOBB().HalfSize().x > boundingBox->GetWorldOBB().HalfSize().z) ? boundingBox->GetWorldOBB().HalfSize().x : boundingBox->GetWorldOBB().HalfSize().z;
			height = boundingBox->GetWorldOBB().Size().MaxElement() - 2*radius;
			if (height < 0) height = 1;
			centerOffset = boundingBox->GetWorldOBB().CenterPoint() - GetOwner().GetComponent<ComponentTransform>()->GetGlobalPosition();
		}
	}
	if (App->time->IsGameRunning() && !rigidBody) App->physics->CreateCapsuleRigidbody(this);
}

void ComponentCapsuleCollider::DrawGizmos() {
	if (IsActiveInHierarchy()) {
		ComponentTransform* ownerTransform = GetOwner().GetComponent<ComponentTransform>();	
		// TODO: Find how to draw a capsule.
		dd::cone(ownerTransform->GetGlobalPosition() + ownerTransform->GetGlobalRotation() * centerOffset - float3(0, height / 2, 0), float3(0, height, 0), dd::colors::Green, radius, radius);
		dd::sphere(ownerTransform->GetGlobalPosition() + ownerTransform->GetGlobalRotation() * centerOffset + float3(0, height / 2, 0), dd::colors::Green, radius);
		dd::sphere(ownerTransform->GetGlobalPosition() + ownerTransform->GetGlobalRotation() * centerOffset - float3(0, height / 2, 0), dd::colors::Green, radius);
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
		App->physics->UpdateCapsuleRigidbody(this);
	}

	if (ImGui::DragFloat("Height", &height, App->editor->dragSpeed3f, 0.0f, inf) && App->time->IsGameRunning()) {
		App->physics->UpdateCapsuleRigidbody(this);
	}

	// TODO: Selector upAxis

	if (ImGui::DragFloat3("Center Offset", centerOffset.ptr(), App->editor->dragSpeed2f, -inf, inf) && App->time->IsGameRunning()) {
		float3 position = GetOwner().GetComponent<ComponentTransform>()->GetGlobalPosition();
		Quat rotation = GetOwner().GetComponent<ComponentTransform>()->GetGlobalRotation();
		rigidBody->setCenterOfMassTransform(btTransform(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w), btVector3(position.x, position.y, position.z)) * btTransform(btQuaternion::getIdentity(), btVector3(centerOffset.x, centerOffset.y, centerOffset.z)));
	}

	if (ImGui::Checkbox("Freeze rotation", &freezeRotation) && App->time->IsGameRunning()) {
		motionState.freezeRotation = freezeRotation;
	}
}

void ComponentCapsuleCollider::Save(JsonValue jComponent) const {
	JsonValue jMass = jComponent[JSON_TAG_MASS];
	jMass = mass;

	JsonValue jRadius = jComponent[JSON_TAG_RADIUS];
	jRadius = radius;

	JsonValue jHeight = jComponent[JSON_TAG_HEIGHT];
	jHeight = height;

	JsonValue jCenterOffset = jComponent[JSON_TAG_CENTER_OFFSET];
	jCenterOffset[0] = centerOffset.x;
	jCenterOffset[1] = centerOffset.y;
	jCenterOffset[2] = centerOffset.z;

	JsonValue jFreeze = jComponent[JSON_TAG_FREEZE_ROTATION];
	jFreeze = freezeRotation;

	JsonValue jIsTrigger = jComponent[JSON_TAG_IS_TRIGGER];
	jIsTrigger = isTrigger;
}

void ComponentCapsuleCollider::Load(JsonValue jComponent) {
	JsonValue jMass = jComponent[JSON_TAG_MASS];
	mass = jMass;

	JsonValue jRadius = jComponent[JSON_TAG_RADIUS];
	radius = jRadius;

	JsonValue jHeight = jComponent[JSON_TAG_HEIGHT];
	height = jHeight;

	JsonValue jCenterOffset = jComponent[JSON_TAG_CENTER_OFFSET];
	centerOffset = float3(jCenterOffset[0], jCenterOffset[1], jCenterOffset[2]);

	JsonValue jFreeze = jComponent[JSON_TAG_FREEZE_ROTATION];
	freezeRotation = jFreeze;

	JsonValue jIsTrigger = jComponent[JSON_TAG_IS_TRIGGER];
	isTrigger = jIsTrigger;
}

void ComponentCapsuleCollider::OnCollision() {
	// TODO: Send event...
}
