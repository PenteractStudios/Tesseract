#include "ComponentSphereCollider.h"

#include "Application.h"
#include "GameObject.h"
#include "Modules/ModulePhysics.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleTime.h"
#include "Components/ComponentBoundingBox.h"

#include "Utils/Logging.h"

#define JSON_TAG_MASS "mass"
#define JSON_TAG_RADIUS "radius"
#define JSON_TAG_CENTER_OFFSET "centerOffset"
#define JSON_TAG_FREEZE_ROTATION "freezeRotation"
#define JSON_TAG_IS_TRIGGER "isTrigger"

void ComponentSphereCollider::Init() {
	if (!centerOffset.IsFinite()) {
		ComponentBoundingBox* boundingBox = GetOwner().GetComponent<ComponentBoundingBox>();
		if (boundingBox) {
			radius = boundingBox->GetWorldOBB().HalfSize().MaxElement();
			centerOffset = boundingBox->GetWorldOBB().CenterPoint() - GetOwner().GetComponent<ComponentTransform>()->GetGlobalPosition();
		} else {
			radius = 1.f;
			centerOffset = float3::one;
		}
	}
	if (App->time->IsGameRunning() && !rigidBody) App->physics->CreateSphereRigidbody(this);
}

void ComponentSphereCollider::DrawGizmos() {
	if (IsActiveInHierarchy()) {
		ComponentTransform* ownerTransform = GetOwner().GetComponent<ComponentTransform>();
		dd::sphere(ownerTransform->GetGlobalPosition() + ownerTransform->GetGlobalRotation() * centerOffset, dd::colors::Green, radius);
	}
}

void ComponentSphereCollider::OnEditorUpdate() {
	/*if (ImGui::Checkbox("Is Trigger", &isTrigger) && App->time->IsGameRunning()) {
		rigidBody->setCollisionFlags(isTrigger ? btCollisionObject::CF_NO_CONTACT_RESPONSE : 0);
		//rigidBody->setMassProps(isTrigger ? 0.f : mass, rigidBody->getLocalInertia());
	}*/

	// Collider Type combo box
	//TODO: Control if rigidbody
	const char* colliderTypeItems[] = {"Dynamic", "Static", "Kinematic", "Trigger"};
	const char* colliderCurrent = colliderTypeItems[(int) colliderType];
	if (ImGui::BeginCombo("Collider Mode", colliderCurrent)) {
		for (int n = 0; n < IM_ARRAYSIZE(colliderTypeItems); ++n) {
			if (ImGui::Selectable(colliderTypeItems[n])) {
				colliderType = ColliderType(n);
				if (App->time->IsGameRunning()) {
					switch (colliderType) {
					case ColliderType::DYNAMIC:
						rigidBody->setCollisionFlags(0);
						//rigidBody->setActivationState(0);
						rigidBody->setMassProps(mass, rigidBody->getLocalInertia());
						//rigi
						break;
					case ColliderType::STATIC:
						rigidBody->setCollisionFlags(0);
						rigidBody->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
						//rigidBody->setActivationState(0);
						rigidBody->setMassProps(0, rigidBody->getLocalInertia());
						break;
					case ColliderType::KINEMATIC:
						rigidBody->setCollisionFlags(0);
						rigidBody->setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT);
						rigidBody->setActivationState(WANTS_DEACTIVATION);
						rigidBody->setMassProps(0, rigidBody->getLocalInertia());
						break;
					case ColliderType::TRIGGER:
						rigidBody->setCollisionFlags(0);
						rigidBody->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_NO_CONTACT_RESPONSE);
						//rigidBody->setActivationState(0);
						rigidBody->setMassProps(0, rigidBody->getLocalInertia());
						break;
					}
				}
			}
		}
		LOG("%d", rigidBody->getCollisionFlags());
		ImGui::EndCombo();
	}

	/*if (colliderType == ColliderType::DYNAMIC) {
		if (ImGui::DragFloat("Mass", &mass, App->editor->dragSpeed3f, 0.0f, 100.f) && App->time->IsGameRunning()) {
			rigidBody->setMassProps(mass, btVector3(0, 0, 0));
		}
	}*/
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

void ComponentSphereCollider::Save(JsonValue jComponent) const {
	JsonValue jMass = jComponent[JSON_TAG_MASS];
	jMass = mass;

	JsonValue jRadius = jComponent[JSON_TAG_RADIUS];
	jRadius = radius;

	JsonValue jCenterOffset = jComponent[JSON_TAG_CENTER_OFFSET];
	jCenterOffset[0] = centerOffset.x;
	jCenterOffset[1] = centerOffset.y;
	jCenterOffset[2] = centerOffset.z;

	JsonValue jFreeze = jComponent[JSON_TAG_FREEZE_ROTATION];
	jFreeze = freezeRotation;

	JsonValue jIsTrigger = jComponent[JSON_TAG_IS_TRIGGER];
	jIsTrigger = isTrigger;
}

void ComponentSphereCollider::Load(JsonValue jComponent) {
	JsonValue jMass = jComponent[JSON_TAG_MASS];
	mass = jMass;

	JsonValue jRadius = jComponent[JSON_TAG_RADIUS];
	radius = jRadius;

	JsonValue jCenterOffset = jComponent[JSON_TAG_CENTER_OFFSET];
	centerOffset = float3(jCenterOffset[0], jCenterOffset[1], jCenterOffset[2]);

	JsonValue jFreeze = jComponent[JSON_TAG_FREEZE_ROTATION];
	freezeRotation = jFreeze;

	JsonValue jIsTrigger = jComponent[JSON_TAG_IS_TRIGGER];
	isTrigger = jIsTrigger;
}

void ComponentSphereCollider::OnCollision() {
	// TODO: Send event...
}