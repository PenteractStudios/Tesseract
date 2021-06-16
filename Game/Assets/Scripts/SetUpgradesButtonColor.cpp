#include "SetUpgradesButtonColor.h"

#include "GameObject.h"
#include "Components/UI/ComponentImage.h"
#include "GameplaySystems.h"

EXPOSE_MEMBERS(SetUpgradesButtonColor) {
    // Add members here to expose them to the engine. Example:
    MEMBER(MemberType::GAME_OBJECT_UID, onUID),
    MEMBER(MemberType::GAME_OBJECT_UID, offUID)
};

GENERATE_BODY_IMPL(SetUpgradesButtonColor);

void SetUpgradesButtonColor::Start() {
    buttonOn = GameplaySystems::GetGameObject(onUID);
    buttonOff = GameplaySystems::GetGameObject(offUID);

    if (!buttonOn || !buttonOff) return;
    imageOn = buttonOn->GetComponent<ComponentImage>();
    imageOff = buttonOff->GetComponent<ComponentImage>();
}

void SetUpgradesButtonColor::Update() {
	
}

void SetUpgradesButtonColor::OnButtonClick() {
    if (!buttonOn || !buttonOff) return;
    if (!imageOn || !imageOff) return;

    if (GetOwner().GetID() == buttonOn->GetID()) {
        imageOn->SetColor(selected);
        imageOff->SetColor(notSelected);
    }
    else {
        imageOff->SetColor(selected);
        imageOn->SetColor(notSelected);
    }
}
