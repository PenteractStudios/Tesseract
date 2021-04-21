#pragma once

#include "Component.h"

class ComponentScript : public Component {
public:
	REGISTER_COMPONENT(ComponentScript, ComponentType::SCRIPT, true);

	void Init();
	void Update();
	void OnStart();

	void OnEditorUpdate() override;
	void Save(JsonValue jComponent) const;
	void Load(JsonValue jComponent);

	UID GetScriptID() const;

public:
	bool onGame = false;

private:
	UID scriptID = 0;
};