#pragma once

#include "Component.h"

class ComponentFog : public Component {
public:
	REGISTER_COMPONENT(ComponentFog, ComponentType::FOG, true);

	void OnEditorUpdate() override;
	void Load(JsonValue jComponent) override;
	void Save(JsonValue jComponent) const override;
	void Draw();

private:
	float falloff = 0.01f;
};
