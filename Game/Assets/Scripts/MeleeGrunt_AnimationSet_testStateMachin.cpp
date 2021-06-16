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
	currentState = animation->GetCurrentState();
}

void MeleeGrunt_AnimationSet_testStateMachin::Update() {

	if (currentState != animation->GetCurrentState())
	{
		// TODO: LOG NOT WORKING PROPERLY YET
		std::string logMSG = "Old State:" + currentState->name + ", New State: " + animation->GetCurrentState()->name;
		Debug::Log(logMSG.c_str());
		currentState = animation->GetCurrentState();
	}

	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_0)) {
		animation->SendTriggerSecondary("IdleAttack");
	}
	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_1)) {
		animation->SendTrigger("SpawnIdle");
	}
	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_2)) {
		animation->SendTrigger("IdleRun");
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
		animation->SendTrigger("RunHurt");
	}
	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_8)) {
		animation->SendTrigger("RunAttack");
	}

	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_9)) {
		animation->SendTrigger("RunDeath");
	}
	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_Q)) { //10
		animation->SendTrigger("HurtIdle");
	}
	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_W)) { //11
		animation->SendTrigger("HurtRun");
	}
	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_E)) { //12
		animation->SendTrigger("HurtAttack");
	}
	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_R)) { //13
		animation->SendTrigger("HurtDeath");
	}
	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_T)) { //14
		animation->SendTrigger("AttackIdle");
	}
	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_Y)) { //15
		animation->SendTrigger("AttackRun");
	}
	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_U)) { //16
		animation->SendTrigger("AttackHurt");
	}
	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_I)) { //17
		animation->SendTrigger("AttackDeath");
	}
	if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_O)) { //18
		animation->SendTrigger("DeathSpawn");
	}
}

void MeleeGrunt_AnimationSet_testStateMachin::OnAnimationFinished() {
	Debug::Log("ANIMATION_FINISHED!!");

	if (currentState != animation->GetCurrentState())
	{
		currentState = animation->GetCurrentState();
	}

	if (currentState->name == "Hurt") {
		animation->SendTrigger("HurtIdle");
	}
	if (currentState->name == "Attack") {
		animation->SendTrigger("AttackIdle");
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