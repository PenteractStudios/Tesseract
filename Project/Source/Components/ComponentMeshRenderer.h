#pragma once

#include "Component.h"

#include "Math/float4x4.h"
#include <unordered_map>

struct aiMesh;

class ComponentMeshRenderer : public Component {
public:
	REGISTER_COMPONENT(ComponentMeshRenderer, ComponentType::MESH_RENDERER, true);
	~ComponentMeshRenderer();

	void Init() override;
	void Start() override;
	void Update() override;

	void OnEditorUpdate() override;
	void Save(JsonValue jComponent) const override;
	void Load(JsonValue jComponent) override;

	void Draw(const float4x4& modelMatrix) const;
	void DrawDepthPrepass(const float4x4& modelMatrix) const;
	void DrawShadow(const float4x4& modelMatrix) const;

	void AddRenderingModeMask();
	void DeleteRenderingModeMask();

	TESSERACT_ENGINE_API UID GetMeshID() const;
	TESSERACT_ENGINE_API UID GetMaterialID() const;
	void SetMeshID(UID meshId);
	void SetMaterialID(UID materialId);
	TESSERACT_ENGINE_API void ChangeMesh(UID meshId);
	TESSERACT_ENGINE_API void ChangeMaterial(UID materialId);

	void SetGameObjectBones(const std::unordered_map<std::string, GameObject*>& goBones);

	// Dissolve
	TESSERACT_ENGINE_API void PlayDissolveAnimation(bool reverse = false);
	TESSERACT_ENGINE_API void ResetDissolveValues();

private:
	void UpdateDissolveAnimation();
	float GetDissolveValue() const;

private:
	UID meshId = 0;
	UID materialId = 0;

	std::vector<float4x4> palette;

	std::unordered_map<std::string, GameObject*> goBones;

	bool bbActive = false;

	// Dissolve variables
	float currentTime = 0.0f;
	float dissolveThreshold = 0.0f;
	bool dissolveAnimationFinished = true;
	bool dissolveAnimationReverse = false;
};
