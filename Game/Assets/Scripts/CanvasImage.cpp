#include "CanvasImage.h"

#include "GameObject.h"
#include "GameplaySystems.h"

EXPOSE_MEMBERS(CanvasImage) {
	// Add members here to expose them to the engine. Example:
	MEMBER(MemberType::GAME_OBJECT_UID, backgroundUID)
};

GENERATE_BODY_IMPL(CanvasImage);

void CanvasImage::Start() {
	background = GameplaySystems::GetGameObject(backgroundUID);

	if (background != nullptr) {
		ComponentTransform2D* transform2D = background->GetComponent<ComponentTransform2D>();
		if (transform2D) {
			float2 newSize = float2(Screen::GetWidth(), Screen::GetHeight());
			transform2D->SetSize(newSize);
		}
	}
}

void CanvasImage::Update() {
	if (background != nullptr) {
		ComponentTransform2D* transform2D = background->GetComponent<ComponentTransform2D>();
		if (transform2D) {
			float2 newSize = float2(Screen::GetWidth(), Screen::GetHeight());
			transform2D->SetSize(newSize);
		}
	}
}