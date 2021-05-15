#include "ScreenResolutionConfirmer.h"

EXPOSE_MEMBERS(ScreenResolutionConfirmer) {

};

GENERATE_BODY_IMPL(ScreenResolutionConfirmer);

void ScreenResolutionConfirmer::Start() {

}

void ScreenResolutionConfirmer::Update() {

}

void ScreenResolutionConfirmer::OnButtonClick() {
	screenResolutionChangeConfirmationWasRequested = true;
}
