#include "ZombunnyController.h"

#include "GameObject.h"
#include "Components/ComponentAgent.h"

EXPOSE_MEMBERS(ZombunnyController) {
	// Add members here to expose them to the engine. Example:
	// MEMBER(MemberType::BOOL, exampleMember1),
	// MEMBER(MemberType::PREFAB_RESOURCE_UID, exampleMember2),
	MEMBER(MemberType::FLOAT3, targetPosition),
		MEMBER(MemberType::FLOAT, agentSpeed)
};

GENERATE_BODY_IMPL(ZombunnyController);

void ZombunnyController::Start() {
	navAgent = GetOwner().GetComponent<ComponentAgent>();
}

void ZombunnyController::Update() {
	if (!navAgent) {
		return;
	}

	navAgent->SetMaxSpeed(agentSpeed);

	if (!targetPosition.Equals(lastTargetPosition)) {
		lastTargetPosition = targetPosition;
		navAgent->SetMoveTarget(targetPosition);
	}
}