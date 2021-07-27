#pragma once

#include "Component.h"

#include "Math/float4.h"

class ComponentFog : public Component {
public:
	REGISTER_COMPONENT(ComponentFog, ComponentType::FOG, true);

	void OnEditorUpdate() override;
	void Load(JsonValue jComponent) override;
	void Save(JsonValue jComponent) const override;
	void Draw();

private:
	float density = 0.01f;
	float falloff = 0.01f;
	float4 inscatteringColor = {0.5f, 0.6f, 0.7f, 1.0f};
};
