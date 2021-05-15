#include "ScreenResolutionSetter.h"

#include "GameplaySystems.h"
#include "Components/UI/ComponentText.h"
#include "Modules/ModuleWindow.h"
#include "GameObject.h"
#include "ScreenResolutionConfirmer.h"

bool screenResolutionChangeConfirmationWasRequested;
int preSelectedScreenResolutionPreset;

EXPOSE_MEMBERS(ScreenResolutionSetter) {

	MEMBER(MemberType::GAME_OBJECT_UID, textObjectID),
		MEMBER(MemberType::BOOL, increasing)
};

GENERATE_BODY_IMPL(ScreenResolutionSetter);

void ScreenResolutionSetter::Start() {
	GameObject* textObj = GameplaySystems::GetGameObject(textObjectID);

	if (textObj) {
		text = textObj->GetComponent<ComponentText>();
	}

	UpdateText();
	screenResolutionChangeConfirmationWasRequested = false;
	preSelectedScreenResolutionPreset = 2;
}

void ScreenResolutionSetter::Update() {
	if (screenResolutionChangeConfirmationWasRequested) {
		if (Screen::GetResolutionPreset() != preSelectedScreenResolutionPreset) {

			Screen::SetResolutionPreset(preSelectedScreenResolutionPreset);

		}
		screenResolutionChangeConfirmationWasRequested = false;
	}
}

void ScreenResolutionSetter::OnButtonClick() {
	IncreaseResolution(increasing ? 1 : -1);
}

void ScreenResolutionSetter::IncreaseResolution(int multiplier) {
	preSelectedScreenResolutionPreset = preSelectedScreenResolutionPreset + multiplier;

	//Avoid getting out of bounds
	if (preSelectedScreenResolutionPreset < 0) {
		preSelectedScreenResolutionPreset = 0;
	} else if (preSelectedScreenResolutionPreset >= Screen::RESOLUTION_PRESET::MAX) {
		preSelectedScreenResolutionPreset = Screen::RESOLUTION_PRESET::MAX - 1;
	}

	UpdateText();
}

void ScreenResolutionSetter::UpdateText() {
	if (!text) return;

	text->SetText(std::to_string(GetPreSelectedWidth()) + "x" + std::to_string(GetPreSelectedHeight()));
}

int ScreenResolutionSetter::GetPreSelectedWidth() {
	switch (preSelectedScreenResolutionPreset) {
	case Screen::RESOLUTION_PRESET::m_1920x1080:
		return 1920;
	case Screen::RESOLUTION_PRESET::m_1280x720:
		return 1280;
	case Screen::RESOLUTION_PRESET::m_1024x576:
		return 1024;
	}
	return 0;
}

int ScreenResolutionSetter::GetPreSelectedHeight() {
	switch (preSelectedScreenResolutionPreset) {
	case Screen::RESOLUTION_PRESET::m_1920x1080:
		return 1080;
	case Screen::RESOLUTION_PRESET::m_1280x720:
		return 720;
	case Screen::RESOLUTION_PRESET::m_1024x576:
		return 576;
	}
	return 0;
}