#include "ComponentCanvasRenderer.h"

#include "GameObject.h"
#include "Components/UI/ComponentImage.h"
#include "Components/UI/ComponentText.h"
#include "Components/UI/ComponentCanvas.h"
#include "Components/UI/ComponentTransform2D.h"


#include "debugdraw.h"

#include "Utils/Leaks.h"

void ComponentCanvasRenderer::Save(JsonValue jComponent) const {
}

void ComponentCanvasRenderer::Load(JsonValue jComponent) {
}

void ComponentCanvasRenderer::Render(const GameObject* gameObject) const {
	ComponentTransform2D* transform2D = gameObject->GetComponent<ComponentTransform2D>();
	const ComponentCanvas* parentCanvas = AnyParentHasCanvas(&GetOwner());
	if (transform2D != nullptr && parentCanvas != nullptr) { // Get the Parent in a variable if needed and add canvas customization to render

		//IF OTHER COMPONENTS THAT RENDER IN UI ARE IMPLEMENTED, THEY MUST HAVE THEIR DRAW METHODS CALLED HERE
		ComponentImage* componentImage = gameObject->GetComponent<ComponentImage>();
		if (componentImage != nullptr && componentImage->IsActive()) {
			componentImage->Draw(transform2D);
		}

		ComponentText* componentText = gameObject->GetComponent<ComponentText>();
		if (componentText != nullptr && componentText->IsActive()) {
			componentText->Draw(transform2D);
		}

		ComponentVideo* componentVideo = gameObject->GetComponent<ComponentVideo>();
		if (componentVideo != nullptr && componentVideo->IsActive()) {
			componentVideo->Draw(transform2D);
		}
	}
}

float2 ComponentCanvasRenderer::GetCanvasSize() {
	ComponentCanvas* canvas = AnyParentHasCanvas(&GetOwner());
	return canvas ? canvas->GetSize() : float2(1920, 1080);
}

float2 ComponentCanvasRenderer::GetScreenReferenceSize() const {
	const ComponentCanvas* canvas = AnyParentHasCanvas(&GetOwner());
	return canvas ? canvas->GetScreenReferenceSize() : float2(1920, 1080);
}

float ComponentCanvasRenderer::GetCanvasScreenFactor() {
	ComponentCanvas* canvas = AnyParentHasCanvas(&GetOwner());
	return canvas ? canvas->GetScreenFactor() : 1.0f;
}

ComponentCanvas* ComponentCanvasRenderer::AnyParentHasCanvas(GameObject* current) const {
	ComponentCanvas* currentCanvas = current->GetComponent<ComponentCanvas>();
	if (currentCanvas != nullptr) {
		return currentCanvas;
	} else {
		if (current->GetParent() != nullptr) {
			return AnyParentHasCanvas(current->GetParent());
		}
	}

	return nullptr;
}

bool ComponentCanvasRenderer::CanBeRemoved() const {
	return !(GetOwner().GetComponent<ComponentImage>() || GetOwner().GetComponent<ComponentText>() || GetOwner().GetComponent<ComponentBoundingBox2D>());
}
