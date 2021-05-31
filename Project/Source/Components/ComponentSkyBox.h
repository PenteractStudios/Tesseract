#pragma once

#include "Component.h"
#include "Utils/UID.h"

class ComponentSkyBox : public Component {
public:
	REGISTER_COMPONENT(ComponentSkyBox, ComponentType::SKYBOX, false);

	void OnEditorUpdate() override;
	void Load(JsonValue jComponent) override;
	void Save(JsonValue jComponent) const override;
	void Draw();

private:
	UID shaderId = 0;
	UID skyboxId = 0;
};
