#include "ComponentBoxCollider.h"

#include "Application.h"
#include "GameObject.h"
#include "Modules/ModulePhysics.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleTime.h"
#include "Components/ComponentBoundingBox.h"

#define JSON_TAG_MASS "mass"
#define JSON_TAG_SIZE "size"
#define JSON_TAG_CENTER_OFFSET "centerOffset"
#define JSON_TAG_FREEZE_ROTATION "freezeRotation"
#define JSON_TAG_IS_TRIGGER "isTrigger"

void ComponentBoxCollider::Init() {
	if (!centerOffset.IsFinite()) {
		localAABB = (AABB(float3(-0.5f), float3(0.5f)));
		CalculateWorldBoundingBox();
		size = worldAABB.Size();
		centerOffset = worldOBB.CenterPoint() - GetOwner().GetComponent<ComponentTransform>()->GetGlobalPosition();
	}

	if (App->time->IsGameRunning() && !rigidBody) App->physics->CreateBoxRigidbody(this);
}

void ComponentBoxCollider::DrawGizmos() {
	if (IsActiveInHierarchy()) {
		//ComponentTransform* ownerTransform = GetOwner().GetComponent<ComponentTransform>();
		ComponentBoundingBox* bb = GetOwner().GetComponent<ComponentBoundingBox>();

		float3 points[8];
		CalculateWorldBoundingBox();
		worldOBB.GetCornerPoints(points);
		//bb->GetWorldOBB().GetCornerPoints(points);

		// Reorder points for drawing
		float3 aux;
		aux = points[2];
		points[2] = points[3];
		points[3] = aux;
		aux = points[6];
		points[6] = points[7];
		points[7] = aux;

		dd::box(points, dd::colors::White);

		//TODO draw box
		//dd::sphere(ownerTransform->GetGlobalPosition() + ownerTransform->GetGlobalRotation() * centerOffset, dd::colors::Green, radius);
	}
}

void ComponentBoxCollider::OnEditorUpdate() {
	if (ImGui::Checkbox("Is Trigger", &isTrigger) && App->time->IsGameRunning()) {
		rigidBody->setCollisionFlags(isTrigger ? btCollisionObject::CF_NO_CONTACT_RESPONSE : 0);
		rigidBody->setMassProps(isTrigger ? 0.f : mass, rigidBody->getLocalInertia());
	}
	if (!isTrigger) {
		if (ImGui::DragFloat("Mass", &mass, App->editor->dragSpeed3f, 0.0f, 100.f) && App->time->IsGameRunning()) {
			rigidBody->setMassProps(mass, btVector3(0, 0, 0));
		}
	}

	if (ImGui::DragFloat3("Size", size.ptr(), App->editor->dragSpeed3f, 0.0f, inf) && App->time->IsGameRunning()) {
		((btBoxShape*) rigidBody->getCollisionShape())->setLocalScaling(btVector3(size.x, size.y, size.z));
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

void ComponentBoxCollider::Save(JsonValue jComponent) const {
	JsonValue jMass = jComponent[JSON_TAG_MASS];
	jMass = mass;

	JsonValue jSize = jComponent[JSON_TAG_SIZE];
	jSize[0] = size.x;
	jSize[1] = size.y;
	jSize[2] = size.z;

	JsonValue jCenterOffset = jComponent[JSON_TAG_CENTER_OFFSET];
	jCenterOffset[0] = centerOffset.x;
	jCenterOffset[1] = centerOffset.y;
	jCenterOffset[2] = centerOffset.z;

	JsonValue jFreeze = jComponent[JSON_TAG_FREEZE_ROTATION];
	jFreeze = freezeRotation;

	JsonValue jIsTrigger = jComponent[JSON_TAG_IS_TRIGGER];
	jIsTrigger = isTrigger;
}

void ComponentBoxCollider::Load(JsonValue jComponent) {
	JsonValue jMass = jComponent[JSON_TAG_MASS];
	mass = jMass;

	JsonValue jSize = jComponent[JSON_TAG_SIZE];
	size.Set(jSize[0], jSize[1], jSize[2]);

	JsonValue jCenterOffset = jComponent[JSON_TAG_CENTER_OFFSET];
	centerOffset = float3(jCenterOffset[0], jCenterOffset[1], jCenterOffset[2]);

	JsonValue jFreeze = jComponent[JSON_TAG_FREEZE_ROTATION];
	freezeRotation = jFreeze;

	JsonValue jIsTrigger = jComponent[JSON_TAG_IS_TRIGGER];
	isTrigger = jIsTrigger;
}

void ComponentBoxCollider::OnCollision() {
	// TODO: Send event...
}

void ComponentBoxCollider::CalculateWorldBoundingBox() {
	if (dirty) {
		GameObject& owner = GetOwner();
		ComponentTransform* transform = owner.GetComponent<ComponentTransform>();
		worldOBB = OBB(localAABB);
		worldOBB.Transform(transform->GetGlobalMatrix());
		worldAABB = worldOBB.MinimalEnclosingAABB();
		dirty = false;
	}
}

void ComponentBoxCollider::Invalidate() {
	dirty = true;

	//TODO Set physical rb position and rotation to Transform position rotation
}