#include "DialogAnimation.h"

#include "GameplaySystems.h"

#include "Components/UI/ComponentImage.h"
#include "Components/UI/ComponentText.h"

#include <string>

EXPOSE_MEMBERS(DialogAnimation) {
	MEMBER(MemberType::FLOAT3, startPosition),
	MEMBER(MemberType::FLOAT3, endPosition),
	MEMBER(MemberType::FLOAT, appearAnimationTime),
	MEMBER(MemberType::FLOAT, disappearAnimationTime),
	MEMBER(MemberType::FLOAT, duration)
};

GENERATE_BODY_IMPL(DialogAnimation);

void DialogAnimation::Start() {
	RetrieveUIComponents(&GetOwner());
}

void DialogAnimation::Update() {
	currentTime += Time::GetDeltaTime();
	if (!startAnimationFinished) {
		if (currentTime < appearAnimationTime) {
			GetOwner().GetComponent<ComponentTransform2D>()->SetPosition(float3::Lerp(startPosition, endPosition, currentTime / appearAnimationTime));
			TransitionUIElementsColor(true);
		}
		else {
			startAnimationFinished = true;
			GetOwner().GetComponent<ComponentTransform2D>()->SetPosition(endPosition);
			TransitionUIElementsColor(true, false);
		}
	}
	if (startAnimationFinished && !endAnimationFinished && currentTime > duration) {
		if (currentTime < (duration + disappearAnimationTime)) {
			TransitionUIElementsColor(false);
		}
		else {
			endAnimationFinished = true;
			TransitionUIElementsColor(false, false);
		}
	}
}

void DialogAnimation::TransitionUIElementsColor(bool appearing, bool mustLerp)
{
	int i = 0;
	float maximumTime = appearing ? appearAnimationTime : disappearAnimationTime;
	float currentTimeToLerp = appearing ? currentTime : (currentTime - duration);

	for (Component* component : uiComponents) {
		float4 originColor = appearing ? float4(uiColors[i].x, uiColors[i].y, uiColors[i].z, 0.0f) : uiColors[i];
		float4 targetColor = appearing ? uiColors[i] : float4(uiColors[i].x, uiColors[i].y, uiColors[i].z, 0.0f);
		if (component->GetType() == ComponentType::IMAGE) {
			static_cast<ComponentImage*>(component)->SetColor(float4::Lerp(originColor, targetColor, mustLerp ? (currentTimeToLerp / maximumTime) : 1.0f));
		}
		else {
			static_cast<ComponentText*>(component)->SetFontColor(float4::Lerp(originColor, targetColor, mustLerp ? (currentTimeToLerp / maximumTime) : 1.0f));
		}
		i++;
	}	
}

void DialogAnimation::RetrieveUIComponents(GameObject* current)
{
	if (current == nullptr) {
		return;
	}

	ComponentImage* image = current->GetComponent<ComponentImage>();
	ComponentText* text = current->GetComponent<ComponentText>();
	if (image) {
		uiComponents.push_back(static_cast<Component*>(image));
		uiColors.push_back(image->GetMainColor());
	}
	else if (text) {
		uiComponents.push_back(static_cast<Component*>(text));
		uiColors.push_back(text->GetFontColor());
	}
	for (GameObject* child : current->GetChildren()) {
		RetrieveUIComponents(child);
	}
}
