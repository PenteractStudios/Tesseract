#include "ChangeCharacter.h"

#include "GameObject.h"
#include "Components/UI/ComponentText.h"
#include "GameplaySystems.h"

EXPOSE_MEMBERS(ChangeCharacter) {
	// Add members here to expose them to the engine. Example:
	MEMBER(MemberType::GAME_OBJECT_UID, fangUID),
	MEMBER(MemberType::GAME_OBJECT_UID, robotUID),
	MEMBER(MemberType::GAME_OBJECT_UID, canvasUID)
};

GENERATE_BODY_IMPL(ChangeCharacter);

void ChangeCharacter::Start() {
	fang = GameplaySystems::GetGameObject(fangUID);
	robot = GameplaySystems::GetGameObject(robotUID);
	canvas = GameplaySystems::GetGameObject(canvasUID);

	/* Provisional code to see cooldown until we implement the proper UI */
	if (canvas) {
		canvas->Disable();
		for (GameObject* child : canvas->GetChildren()) {
			if (child->name == "Text") {
				cooldownText = child->GetComponent<ComponentText>();
			}
		}
	}

	/********************************************************************/

	if (robot) {
		robot->Disable();
	}
	
	Debug::Log("Ending Start function");
}

void ChangeCharacter::Update() {
	if (!fang) return;
	if (!robot) return;
	if (!canvas) return;

	timeLeft -= Time::GetDeltaTime();
	changeAvailable = timeLeft < 0.f ? true : false;

	if (changeAvailable) {
		/* Provisional code to see cooldown until we implement the proper UI */
		canvas->Disable();
		/*******************************************************************/
		if (Input::GetKeyCodeDown(Input::KEYCODE::KEY_SPACE)) {
			changeAvailable = false;
			if (fang->IsActive()) {
				Debug::Log("Swaping to robot...");
				fang->Disable();
				robot->Enable();
			}
			else {
				Debug::Log("Swaping to fang...");
				robot->Disable();
				fang->Enable();
			}
			timeLeft = COOLDOWN_TIME;
		}
	}
	else {
		/* Provisional code to see cooldown until we implement the proper UI */
		canvas->Enable();
		if (cooldownText) {
			char time[80];
			sprintf_s(time, "%.2f", timeLeft);
			cooldownText->SetText(time);
		}
		/********************************************************************/
	}
}