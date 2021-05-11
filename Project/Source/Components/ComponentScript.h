#pragma once

#include "Component.h"
#include "Scripting/Member.h"
#include "Utils/UID.h"
#include "Scripting/Script.h"

#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

class ComponentScript : public Component {
public:
	REGISTER_COMPONENT(ComponentScript, ComponentType::SCRIPT, true);

	void OnEditorUpdate() override;
	void Save(JsonValue jComponent) const override;
	void Load(JsonValue jComponent) override;

	void CreateScriptInstance();
	void ReleaseScriptInstance();
	Script* GetScriptInstance() const;

private:
	std::unordered_map<std::string, std::pair<MemberType, MEMBER_VARIANT>> changedValues;

	UID scriptId = 0;
	std::unique_ptr<Script> scriptInstance = nullptr;
};