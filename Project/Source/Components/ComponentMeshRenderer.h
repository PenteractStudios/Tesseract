#pragma once

#include "Component.h"

#include "Math/float4x4.h"
#include <unordered_map>

struct aiMesh;

class ComponentMeshRenderer : public Component {
public:
	REGISTER_COMPONENT(ComponentMeshRenderer, ComponentType::MESH_RENDERER, true);

	void OnEditorUpdate() override;
	void Init() override;
	void Update() override;
	void Save(JsonValue jComponent) const override;
	void Load(JsonValue jComponent) override;

	void Draw(const float4x4& modelMatrix) const;
	void DrawShadow(const float4x4& modelMatrix) const;

public:
	UID meshId = 0;
	UID materialId = 0;
	std::vector<float4x4> palette;

	std::unordered_map<std::string, GameObject*> goBones;

private:
	bool bbActive = false;
};
