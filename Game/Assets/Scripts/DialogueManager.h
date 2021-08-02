#pragma once

#include "Scripting/Script.h"

class ComponentText;
class DialogueManager : public Script
{
	GENERATE_BODY(DialogueManager);

public:
	enum class DialogueWindow {
		NONE = 0,
		FANG = 1,
		ONIMARU = 2,
		DUKE = 3,
		ROSAMONDE = 4,
		TUTO_FANG = 5,
		TUTO_ONIMARU = 6,
		TUTO_SWAP = 7,
		UPGRADES1 = 8,
		UPGRADES2 = 9,
		UPGRADES3 = 10
	};

	struct Dialogue {
		Dialogue() {};
		Dialogue(DialogueWindow character_, const char* text_, Dialogue* nextDialogue_) : character(character_), text(text_), nextDialogue(nextDialogue_) {};

		DialogueWindow character = DialogueWindow::NONE; // 1 = Fang, 2 = Onimaru, 3 = Duke, 4 = Rosamonde, 5 = Tutorial Fang, 6 = Tutorial Oni, 7 = Tutorial Swap, 8 = Upgrades 1/3, 9 = Upgrades 2/3, 10 = Upgrades 3/3
		const char* text = nullptr;
		Dialogue* nextDialogue = nullptr;
	};

public:

	void Start() override;
	void Update() override;

	void SetActiveDialogue(Dialogue* dialogue, bool runAnimation = true);
	void ActivateDialogue(Dialogue* dialogue);
	void ActivateNextDialogue(Dialogue* dialogue);
	void CloseDialogue(Dialogue* dialogue);
	void ActivatePowerUpDialogue();

	void TransitionUIElementsColor(bool appearing = true, bool mustLerp = true);
	void RetrieveUIComponents(GameObject* current);

public:
	Dialogue dialoguesArray[13];
	Dialogue* activeDialogue = nullptr;			// Pointer to the Dialogue Tree
	GameObject* activeDialogueObject = nullptr; // Pointer to the Dialogue GameObject in the scene (DialogueFang, DialogueOnimaru or DialogueDuke)

	// ------ GAMEOBJECT REFS ------ //
	UID fangTextObjectUID = 0;
	UID onimaruTextObjectUID = 0;
	ComponentText* fangTextComponent = 0;
	ComponentText* onimaruTextComponent = 0;

	UID tutorialFangUID = 0;
	UID tutorialOnimaruUID = 0;
	UID tutorialSwapUID = 0;
	GameObject* tutorialFang = nullptr;
	GameObject* tutorialOnimaru = nullptr;
	GameObject* tutorialSwap = nullptr;

	UID tutorialUpgrades1UID = 0;
	UID tutorialUpgrades2UID = 0;
	UID tutorialUpgrades3UID = 0;
	GameObject* tutorialUpgrades1 = nullptr;
	GameObject* tutorialUpgrades2 = nullptr;
	GameObject* tutorialUpgrades3 = nullptr;

	// ------ TRANSITION INFO ------ //
	float3 dialogueStartPosition = float3(0, 0, 0);
	float3 dialogueEndPosition = float3(0, 0, 0);
	float3 tutorialStartPosition = float3(0, 0, 0);
	float3 tutorialEndPosition = float3(0, 0, 0);
	float3 currentStartPosition = float3(0, 0, 0);
	float3 currentEndPosition = float3(0, 0, 0);
	float appearAnimationTime = .5f;
	float disappearAnimationTime = .5f;
	float animationLerpTime = 0.0f;
	std::vector<Component*> uiComponents;
	std::vector<float4> uiColors;
	bool runOpenAnimation = false;
	bool runChangeAnimation = false;
	bool runCloseAnimation = false;
	bool runSecondaryOpen = false;


	int obtainedPowerUps = 0; // PowerUp counter
};
