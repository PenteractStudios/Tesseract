#include "DialogueManager.h"
#include "DialogueManager.h"

#include "GameplaySystems.h"
#include "Player.h"
#include "PlayerController.h"
#include "GameObject.h"
#include "Components/UI/ComponentText.h"

// TODO:
// IsBlocking to "pause" the gameplay

EXPOSE_MEMBERS(DialogueManager) {
	MEMBER(MemberType::GAME_OBJECT_UID, fangTextObjectUID),
	MEMBER(MemberType::GAME_OBJECT_UID, onimaruTextObjectUID),
	MEMBER(MemberType::GAME_OBJECT_UID, tutorialFangUID),
	MEMBER(MemberType::GAME_OBJECT_UID, tutorialOnimaruUID),
	MEMBER(MemberType::GAME_OBJECT_UID, tutorialSwapUID),
	MEMBER(MemberType::GAME_OBJECT_UID, tutorialUpgrades1UID),
	MEMBER(MemberType::GAME_OBJECT_UID, tutorialUpgrades2UID),
	MEMBER(MemberType::GAME_OBJECT_UID, tutorialUpgrades3UID),
	MEMBER(MemberType::FLOAT3, dialogueStartPosition),
	MEMBER(MemberType::FLOAT3, dialogueEndPosition),
	MEMBER(MemberType::FLOAT3, tutorialStartPosition),
	MEMBER(MemberType::FLOAT3, tutorialEndPosition),
	MEMBER(MemberType::FLOAT, appearAnimationTime),
	MEMBER(MemberType::FLOAT, disappearAnimationTime)
};

GENERATE_BODY_IMPL(DialogueManager);

void DialogueManager::Start() {
	GameObject* fangTextObject = GameplaySystems::GetGameObject(fangTextObjectUID);
	GameObject* onimaruTextObject = GameplaySystems::GetGameObject(onimaruTextObjectUID);
	tutorialFang = GameplaySystems::GetGameObject(tutorialFangUID);
	tutorialOnimaru = GameplaySystems::GetGameObject(tutorialOnimaruUID);
	tutorialSwap = GameplaySystems::GetGameObject(tutorialSwapUID);
	tutorialUpgrades1 = GameplaySystems::GetGameObject(tutorialUpgrades1UID);
	tutorialUpgrades2 = GameplaySystems::GetGameObject(tutorialUpgrades2UID);
	tutorialUpgrades3 = GameplaySystems::GetGameObject(tutorialUpgrades3UID);

	if (fangTextObject && onimaruTextObject) {
		fangTextComponent = fangTextObject->GetComponent<ComponentText>();
		onimaruTextComponent = onimaruTextObject->GetComponent<ComponentText>();
	}

	// UPGRADES
	dialoguesArray[0] = Dialogue(DialogueWindow::ONIMARU, "Hey Fang, look at this.", &dialoguesArray[1]);
	dialoguesArray[1] = Dialogue(DialogueWindow::FANG, "Hmm...\nIt looks like Milibot has been\nresearching in some\nnew technologies...", &dialoguesArray[2]);
	dialoguesArray[2] = Dialogue(DialogueWindow::FANG, "I might be able to\nseize its power for ourselves\nif we find a couple more.", &dialoguesArray[3]);
	dialoguesArray[3] = Dialogue(DialogueWindow::UPGRADES1, "", nullptr);

	dialoguesArray[4] = Dialogue(DialogueWindow::UPGRADES2, "", nullptr);


	dialoguesArray[5] = Dialogue(DialogueWindow::FANG, "I think I got it...\nYou can power it up this way,\nthen connect this here...\nand that there...", &dialoguesArray[6]);
	dialoguesArray[6] = Dialogue(DialogueWindow::FANG, "Plug it in our core and...\nWHOAH! Oni, try this!", &dialoguesArray[7]);
	dialoguesArray[7] = Dialogue(DialogueWindow::ONIMARU, "I am not sure about this Fang...\nBut OK, I trust you.", &dialoguesArray[8]);
	dialoguesArray[8] = Dialogue(DialogueWindow::UPGRADES3, "", nullptr);

	// SWAP DIALOGUE + ONIMARU TUTORIAL
	dialoguesArray[9] = Dialogue(DialogueWindow::FANG, "Oni, you perform better\nin closed spaces.\nTake the lead here!", &dialoguesArray[10]);
	dialoguesArray[10] = Dialogue(DialogueWindow::TUTO_SWAP, "", &dialoguesArray[11]);
	dialoguesArray[11] = Dialogue(DialogueWindow::ONIMARU, "OK Fang.\nWatch how it is done.", &dialoguesArray[12]);
	dialoguesArray[12] = Dialogue(DialogueWindow::TUTO_ONIMARU, "", nullptr);

}

void DialogueManager::Update() {
	if (!fangTextComponent) return;
	if (!onimaruTextComponent) return;
	if (!tutorialFang) return;
	if (!tutorialOnimaru) return;
	if (!tutorialSwap) return;
	if (!tutorialUpgrades1) return;
	if (!tutorialUpgrades2) return;
	if (!tutorialUpgrades3) return;

	if (activeDialogue) {
		if (runOpenAnimation) ActivateDialogue(activeDialogue);

		if (Player::GetInputBool(InputActions::INTERACT, PlayerController::useGamepad) && !(runOpenAnimation || runChangeAnimation || runCloseAnimation) && activeDialogueObject) {
			if (activeDialogue->nextDialogue) {
				runChangeAnimation = true;
				if ((static_cast<int>(activeDialogue->character) < 5 && static_cast<int>(activeDialogue->nextDialogue->character) >= 5) ||
					(static_cast<int>(activeDialogue->character) >= 5 && static_cast<int>(activeDialogue->nextDialogue->character) < 5)) {
					runCloseAnimation = true;
					runSecondaryOpen = true;
				}
			} else runCloseAnimation = true;
		}

		if (runChangeAnimation && !runCloseAnimation) {
			ActivateNextDialogue(activeDialogue);
		}

		if (runCloseAnimation) CloseDialogue(activeDialogue);
	}
}

void DialogueManager::SetActiveDialogue(Dialogue* dialogue, bool runAnimation) {
	if (activeDialogueObject) {
		if (activeDialogueObject->IsActive()) {
			activeDialogueObject->Disable();
		}
	}

	if (dialogue) {
		if (static_cast<int>(dialogue->character) >= 5) {
			currentStartPosition = tutorialStartPosition;
			currentEndPosition = tutorialEndPosition;
		} else {
			currentStartPosition = dialogueStartPosition;
			currentEndPosition = dialogueEndPosition;
		}

		activeDialogue = dialogue;
		switch (dialogue->character) {
		case DialogueWindow::FANG:
			activeDialogueObject = fangTextComponent->GetOwner().GetParent();
			fangTextComponent->SetText(dialogue->text);
			break;
		case DialogueWindow::ONIMARU:
			activeDialogueObject = onimaruTextComponent->GetOwner().GetParent();
			onimaruTextComponent->SetText(dialogue->text);
			break;
		case DialogueWindow::DUKE:
			break;
		case DialogueWindow::ROSAMONDE:
			break;
		case DialogueWindow::TUTO_FANG:
			activeDialogueObject = tutorialFang;
			break;
		case DialogueWindow::TUTO_ONIMARU:
			activeDialogueObject = tutorialOnimaru;
			break;
		case DialogueWindow::TUTO_SWAP:
			activeDialogueObject = tutorialSwap;
			break;
		case DialogueWindow::UPGRADES1:
			activeDialogueObject = tutorialUpgrades1;
			break;
		case DialogueWindow::UPGRADES2:
			activeDialogueObject = tutorialUpgrades2;
			break;
		case DialogueWindow::UPGRADES3:
			activeDialogueObject = tutorialUpgrades3;
			break;
		default:
			break;
		}

		runOpenAnimation = runAnimation;
		uiComponents.clear();
		uiColors.clear();
		RetrieveUIComponents(activeDialogueObject);
	} else {
		activeDialogueObject = nullptr;
	}
}

void DialogueManager::ActivateDialogue(Dialogue* dialogue) {
	runSecondaryOpen = false;

	if (activeDialogueObject) {
		if (!activeDialogueObject->IsActive()) {
			activeDialogueObject->Enable();
		}
	}

	animationLerpTime += Time::GetDeltaTime();

	if (runOpenAnimation) {
		if (animationLerpTime < appearAnimationTime) {
			activeDialogueObject->GetComponent<ComponentTransform2D>()->SetPosition(float3::Lerp(currentStartPosition, currentEndPosition, animationLerpTime / appearAnimationTime));
			TransitionUIElementsColor(true);
		} else {
			runOpenAnimation = false;
			animationLerpTime = 0;
			activeDialogueObject->GetComponent<ComponentTransform2D>()->SetPosition(currentEndPosition);
			TransitionUIElementsColor(true, false);
		}
	}
}

void DialogueManager::ActivateNextDialogue(Dialogue* dialogue) {
	activeDialogueObject->GetComponent<ComponentTransform2D>()->SetPosition(currentStartPosition);
	activeDialogueObject->Disable();
	// here should go the transition animations
	SetActiveDialogue(dialogue->nextDialogue, false);
	activeDialogueObject->Enable();
	if (runSecondaryOpen) {
		runOpenAnimation = true;
	} else {
		activeDialogueObject->GetComponent<ComponentTransform2D>()->SetPosition(currentEndPosition);
	}
	runChangeAnimation = false;
}

void DialogueManager::CloseDialogue(Dialogue* dialogue) {
	animationLerpTime += Time::GetDeltaTime();
	if (runCloseAnimation) {
		if (animationLerpTime < appearAnimationTime) {
			activeDialogueObject->GetComponent<ComponentTransform2D>()->SetPosition(float3::Lerp(currentEndPosition, currentStartPosition, animationLerpTime / appearAnimationTime));
			TransitionUIElementsColor(false);
		} else {
			runCloseAnimation = false;
			animationLerpTime = 0;
			activeDialogueObject->GetComponent<ComponentTransform2D>()->SetPosition(currentStartPosition);
			TransitionUIElementsColor(true, false);

			if (activeDialogueObject) {
				if (activeDialogueObject->IsActive()) {
					activeDialogueObject->Disable();
				}
			}
			if (!dialogue->nextDialogue) SetActiveDialogue(nullptr);
		}
	}
}

void DialogueManager::ActivatePowerUpDialogue() {
	obtainedPowerUps += 1;
	int nextDialogue;
	switch (obtainedPowerUps) {
	case 1:
		nextDialogue = 0;
		break;
	case 2:
		nextDialogue = 4;
		break;
	case 3:
		nextDialogue = 5;
		break;
	default:
		break;
	}
	SetActiveDialogue(&dialoguesArray[nextDialogue]);

}

void DialogueManager::TransitionUIElementsColor(bool appearing, bool mustLerp) {
	int i = 0;
	float maximumTime = appearing ? appearAnimationTime : disappearAnimationTime;

	for (Component* component : uiComponents) {
		float4 originColor = appearing ? float4(uiColors[i].x, uiColors[i].y, uiColors[i].z, 0.0f) : uiColors[i];
		float4 targetColor = appearing ? uiColors[i] : float4(uiColors[i].x, uiColors[i].y, uiColors[i].z, 0.0f);
		if (component->GetType() == ComponentType::IMAGE) {
			static_cast<ComponentImage*>(component)->SetColor(float4::Lerp(originColor, targetColor, mustLerp ? (animationLerpTime / maximumTime) : 1.0f));
		} else {
			static_cast<ComponentText*>(component)->SetFontColor(float4::Lerp(originColor, targetColor, mustLerp ? (animationLerpTime / maximumTime) : 1.0f));
		}
		i++;
	}
}

void DialogueManager::RetrieveUIComponents(GameObject* current) {
	if (!current) return;

	ComponentImage* image = current->GetComponent<ComponentImage>();
	ComponentText* text = current->GetComponent<ComponentText>();
	if (image) {
		uiComponents.push_back(static_cast<Component*>(image));
		uiColors.push_back(image->GetMainColor());
	} else if (text) {
		uiComponents.push_back(static_cast<Component*>(text));
		uiColors.push_back(text->GetFontColor());
	}
	for (GameObject* child : current->GetChildren()) {
		RetrieveUIComponents(child);
	}
}
