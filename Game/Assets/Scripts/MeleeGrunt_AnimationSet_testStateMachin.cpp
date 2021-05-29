#include "MeleeGrunt_AnimationSet_testStateMachin.h"

#include "GameplaySystems.h"

#include "GameObject.h"
#include "Components/ComponentAnimation.h"
#include "State.h"
#include "TesseractEvent.h"

EXPOSE_MEMBERS(MeleeGrunt_AnimationSet_testStateMachin) {
    // Add members here to expose them to the engine. Example:
    // MEMBER(MemberType::BOOL, exampleMember1),
    // MEMBER(MemberType::PREFAB_RESOURCE_UID, exampleMember2),
    // MEMBER(MemberType::GAME_OBJECT_UID, exampleMember3)
};

GENERATE_BODY_IMPL(MeleeGrunt_AnimationSet_testStateMachin);

void MeleeGrunt_AnimationSet_testStateMachin::Start() {
	GameObject goAnimated = GetOwner();
	animation = goAnimated.GetComponent<ComponentAnimation>();
	currentState = animation->GetCurrentStatePrincipal();
}

void MeleeGrunt_AnimationSet_testStateMachin::Update() {

	if (currentState != animation->GetCurrentStatePrincipal())
	{
		// TODO: LOG NOT WORKING PROPERLY YET
		std::string logMSG = "Old State:" + currentState->name + ", New State: " + animation->GetCurrentStatePrincipal()->name;
		Debug::Log(logMSG.c_str());
		currentState = animation->GetCurrentStatePrincipal();
	}

	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_0)) {
		animation->SendTriggerSecondary("IdleAttack");
	}
	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_1)) {
		animation->SendTriggerPrincipal("SpawnIdle");
	}
	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_2)) {
		animation->SendTriggerPrincipal("IdleRun");
	}

	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_3)) {
		animation->SendTriggerSecondary("RunAttack");
	}	
	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_4)) {
		animation->SendTriggerSecondary("RunHurt");
	}

	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_5)) {
		animation->SendTriggerSecondary("AttackRun");
	}
	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_6)) {
		animation->SendTriggerSecondary("HurtRun");
	}

	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_7)) {
		animation->SendTriggerPrincipal("RunHurt");
	}
	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_8)) {
		animation->SendTriggerPrincipal("RunAttack");
	}

	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_9)) {
		animation->SendTriggerPrincipal("RunDeath");
	}
	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_Q)) { //10
		animation->SendTriggerPrincipal("HurtIdle");
	}
	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_W)) { //11
		animation->SendTriggerPrincipal("HurtRun");
	}
	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_E)) { //12
		animation->SendTriggerPrincipal("HurtAttack");
	}
	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_R)) { //13
		animation->SendTriggerPrincipal("HurtDeath");
	}
	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_T)) { //14
		animation->SendTriggerPrincipal("AttackIdle");
	}
	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_Y)) { //15
		animation->SendTriggerPrincipal("AttackRun");
	}
	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_U)) { //16
		animation->SendTriggerPrincipal("AttackHurt");
	}
	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_I)) { //17
		animation->SendTriggerPrincipal("AttackDeath");
	}
	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_O)) { //18
		animation->SendTriggerPrincipal("DeathSpawn");
	}
}

void MeleeGrunt_AnimationSet_testStateMachin::OnAnimationFinished() {
	Debug::Log("ANIMATION_FINISHED!!");

	if (currentState != animation->GetCurrentStatePrincipal())
	{
		currentState = animation->GetCurrentStatePrincipal();
	}

	if (currentState->name == "Hurt") {
		animation->SendTriggerPrincipal("HurtIdle");
	}
	if (currentState->name == "Attack") {
		animation->SendTriggerPrincipal("AttackIdle");
	}
}

void MeleeGrunt_AnimationSet_testStateMachin::OnAnimationSecondaryFinished() {
	Debug::Log("ANIMATION_SECONDARY_FINISHED!!");

	if (currentState != animation->GetCurrentStateSecondary())
	{
		currentState = animation->GetCurrentStateSecondary();
	}

	if (currentState->name == "Hurt") {
		animation->SendTriggerSecondary("HurtRun");
	}
	if (currentState->name == "Attack") {
		animation->SendTriggerSecondary("AttackRun");
	}
}