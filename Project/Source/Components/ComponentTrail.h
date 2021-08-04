#pragma once
#include "Component.h"
#include "Utils/Trail.h"
#include "Utils/Pool.h"
#include "Utils/UID.h"

#include "Math/float3.h"
#include "Math/Quat.h"

#define MAX_VERTICES 1500

class ImGradient;
struct ImGradientMark;

class ComponentTrail : public Component {
public:
	REGISTER_COMPONENT(ComponentTrail, ComponentType::TRAIL, false);

	~ComponentTrail();

	void Init() override;
	void Update() override;
	void OnEditorUpdate() override;
	void Load(JsonValue jComponent) override;
	void Save(JsonValue jComponent) const override;

	void Draw();

	TESSERACT_ENGINE_API void Play();
	TESSERACT_ENGINE_API void Stop();
	TESSERACT_ENGINE_API void SetWidth(float w);

public:
	Trail* trail = nullptr;
	// Color Settings
	ImGradient* gradient = nullptr;
	ImGradientMark* draggingGradient = nullptr;
	ImGradientMark* selectedGradient = nullptr;
};
