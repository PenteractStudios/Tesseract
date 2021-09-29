#include "ComponentMeshRenderer.h"

#include "Globals.h"
#include "Application.h"
#include "GameObject.h"
#include "Modules/ModulePrograms.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModuleCamera.h"
#include "Modules/ModuleRender.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleTime.h"
#include "Modules/ModuleScene.h"
#include "Resources/ResourceMaterial.h"
#include "Resources/ResourceMesh.h"
#include "Resources/ResourceTexture.h"
#include "Resources/ResourceSkybox.h"
#include "Components/ComponentTransform.h"
#include "Components/ComponentLight.h"
#include "Components/ComponentSkyBox.h"
#include "Components/ComponentBoundingBox.h"
#include "Components/ComponentAnimation.h"
#include "FileSystem/TextureImporter.h"
#include "Utils/ImGuiUtils.h"

#include "Geometry/Sphere.h"
#include "assimp/mesh.h"
#include "GL/glew.h"

#include "Utils/Leaks.h"

#define JSON_TAG_MESH_ID "MeshID"
#define JSON_TAG_MATERIAL_ID "MaterialID"

void ComponentMeshRenderer::OnEditorUpdate() {
	if (ImGui::Checkbox("Active", &active)) {
		if (GetOwner().IsActive()) {
			if (active) {
				Enable();
			} else {
				Disable();
			}
		}
	}
	ImGui::Separator();

	UID oldMeshId = meshId;
	ImGui::ResourceSlot<ResourceMesh>("Mesh", &meshId);
	if (ImGui::Button("Remove##mesh")) {
		if (meshId != 0) {
			App->resources->DecreaseReferenceCount(meshId);
			meshId = 0;
		}
	}

	if (oldMeshId != meshId) {
		ComponentTransform* transform = GetOwner().GetComponent<ComponentTransform>();
		transform->InvalidateHierarchy();
	}

	if (ImGui::TreeNode("Mesh")) {
		ResourceMesh* mesh = App->resources->GetResource<ResourceMesh>(meshId);
		if (mesh != nullptr) {
			ImGui::TextColored(App->editor->titleColor, "Geometry");
			ImGui::TextWrapped("Num Vertices: ");
			ImGui::SameLine();
			ImGui::TextColored(App->editor->textColor, "%d", mesh->vertices.size());
			ImGui::TextWrapped("Num Triangles: ");
			ImGui::SameLine();
			ImGui::TextColored(App->editor->textColor, "%d", mesh->indices.size() / 3);
			ImGui::Separator();
			ImGui::TextColored(App->editor->titleColor, "Bounding Box");

			ImGui::Checkbox("Draw", &bbActive);
			if (bbActive) {
				ComponentBoundingBox* boundingBox = GetOwner().GetComponent<ComponentBoundingBox>();
				boundingBox->DrawBoundingBox();
			}
		}
		ImGui::TreePop();
	}
	ImGui::Separator();

	ImGui::ResourceSlot<ResourceMaterial>(
		"Material",
		&materialId,
		[this]() { DeleteRenderingModeMask(); },
		[this]() { AddRenderingModeMask(); });

	if (ImGui::Button("Remove##material")) {
		if (materialId != 0) {
			DeleteRenderingModeMask();
			App->resources->DecreaseReferenceCount(materialId);
			materialId = 0;
		}
	}
	if (ImGui::TreeNode("Material")) {
		ResourceMaterial* material = App->resources->GetResource<ResourceMaterial>(materialId);
		if (material != nullptr) {
			material->OnEditorUpdate();

			if (material->shaderType == MaterialShader::UNLIT_DISSOLVE || material->shaderType == MaterialShader::STANDARD_DISSOLVE) {
				if (ImGui::Button("Play Dissolve Animation")) {
					PlayDissolveAnimation();
				}

				if (ImGui::Button("Play Dissolve Animation Reverse")) {
					PlayDissolveAnimation(true);
				}

				if (ImGui::Button("Reset Dissolve Animation")) {
					ResetDissolveValues();
				}

				ImGui::Text("For debug only");
				ImGui::Checkbox("Animation finished", &dissolveAnimationFinished);
				ImGui::DragFloat("Threshold", &dissolveThreshold, App->editor->dragSpeed2f, 0, inf);
			}
		}

		ImGui::TreePop();
	}
}

void ComponentMeshRenderer::Init() {
	ResourceMesh* mesh = App->resources->GetResource<ResourceMesh>(meshId);
	if (!mesh) return;

	palette.resize(mesh->bones.size());
	for (unsigned i = 0; i < mesh->bones.size(); ++i) {
		palette[i] = float4x4::identity;
	}
}

void ComponentMeshRenderer::Update() {
	ResourceMesh* mesh = App->resources->GetResource<ResourceMesh>(meshId);
	if (!mesh) return;

	if (palette.empty()) {
		palette.resize(mesh->bones.size());
		for (unsigned i = 0; i < mesh->bones.size(); ++i) {
			palette[i] = float4x4::identity;
		}
	}

	UpdateDissolveAnimation();

	if (App->time->GetDeltaTime() <= 0) return;

	const GameObject* parent = GetOwner().GetParent();
	const GameObject* rootBone = parent->GetRootBone();
	if (rootBone != nullptr) {
		const GameObject* rootBoneParent = rootBone->GetParent();
		const float4x4& invertedRootBoneTransform = rootBoneParent ? rootBoneParent->GetComponent<ComponentTransform>()->GetGlobalMatrix().Inverted() : float4x4::identity;

		const float4x4& localMatrix = GetOwner().GetComponent<ComponentTransform>()->GetLocalMatrix();
		for (unsigned i = 0; i < mesh->bones.size(); ++i) {
			const GameObject* bone = goBones.at(mesh->bones[i].boneName);
			palette[i] = localMatrix * invertedRootBoneTransform * bone->GetComponent<ComponentTransform>()->GetGlobalMatrix() * mesh->bones[i].transform;
		}
	}
}

void ComponentMeshRenderer::Save(JsonValue jComponent) const {
	jComponent[JSON_TAG_MESH_ID] = meshId;
	jComponent[JSON_TAG_MATERIAL_ID] = materialId;
}

void ComponentMeshRenderer::Load(JsonValue jComponent) {
	meshId = jComponent[JSON_TAG_MESH_ID];
	if (meshId != 0) App->resources->IncreaseReferenceCount(meshId);
	materialId = jComponent[JSON_TAG_MATERIAL_ID];
	if (materialId != 0) {
		AddRenderingModeMask();
		App->resources->IncreaseReferenceCount(materialId);
	}

	ResetDissolveValues();
}

void ComponentMeshRenderer::Start() {

	ResourceMaterial* material = App->resources->GetResource<ResourceMaterial>(materialId);

	if (material == nullptr) return;

	if (material->castShadows) {
		GameObject* owner = &GetOwner();
		if (material->shadowCasterType == ShadowCasterType::STATIC) {
			App->scene->scene->AddStaticShadowCaster(owner);
		} else {
			App->scene->scene->AddDynamicShadowCaster(owner);
		}
	}
}

void ComponentMeshRenderer::Draw(const float4x4& modelMatrix) const {
	if (!IsActive()) return;

	ResourceMesh* mesh = App->resources->GetResource<ResourceMesh>(meshId);
	if (mesh == nullptr) return;

	ResourceMaterial* material = App->resources->GetResource<ResourceMaterial>(materialId);
	if (material == nullptr) return;

	// Specific shader settings
	unsigned glTextureNormal = 0;
	ResourceTexture* normal = App->resources->GetResource<ResourceTexture>(material->normalMapId);
	glTextureNormal = normal ? normal->glTexture : 0;
	int hasNormalMap = normal ? 1 : 0;

	ProgramStandard* standardProgram = nullptr;
	switch (material->shaderType) {
	case MaterialShader::PHONG: {
		// Phong-specific uniform settings
		ProgramStandardPhong* phongProgram = hasNormalMap ? App->programs->phongNormal : App->programs->phongNotNormal;
		if (phongProgram == nullptr) return;

		glUseProgram(phongProgram->program);

		unsigned glTextureSpecular = 0;
		ResourceTexture* specular = App->resources->GetResource<ResourceTexture>(material->specularMapId);
		glTextureSpecular = specular ? specular->glTexture : 0;
		int hasSpecularMap = specular ? 1 : 0;

		glUniform3fv(phongProgram->specularColorLocation, 1, material->specularColor.ptr());
		glUniform1i(phongProgram->hasSpecularMapLocation, hasSpecularMap);

		glUniform1i(phongProgram->specularMapLocation, 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, glTextureSpecular);

		standardProgram = phongProgram;
		break;
	}
	case MaterialShader::STANDARD_SPECULAR: {
		// Specular-specific uniform settings
		ProgramStandardSpecular* specularProgram = hasNormalMap ? App->programs->specularNormal : App->programs->specularNotNormal;
		if (specularProgram == nullptr) return;

		glUseProgram(specularProgram->program);

		unsigned glTextureSpecular = 0;
		ResourceTexture* specular = App->resources->GetResource<ResourceTexture>(material->specularMapId);
		glTextureSpecular = specular ? specular->glTexture : 0;
		int hasSpecularMap = specular ? 1 : 0;

		glUniform3fv(specularProgram->specularColorLocation, 1, material->specularColor.ptr());
		glUniform1i(specularProgram->hasSpecularMapLocation, hasSpecularMap);

		glUniform1i(specularProgram->specularMapLocation, 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, glTextureSpecular);

		standardProgram = specularProgram;
		break;
	}
	case MaterialShader::STANDARD: {
		// Standard-specific uniform settings
		ProgramStandardMetallic* metallicProgram = hasNormalMap ? App->programs->standardNormal : App->programs->standardNotNormal;
		if (metallicProgram == nullptr) return;

		glUseProgram(metallicProgram->program);

		// Standard-specific settings
		unsigned glTextureMetallic = 0;
		ResourceTexture* metallic = App->resources->GetResource<ResourceTexture>(material->metallicMapId);
		glTextureMetallic = metallic ? metallic->glTexture : 0;
		int hasMetallicMap = metallic ? 1 : 0;

		glUniform1f(metallicProgram->metalnessLocation, material->metallic);
		glUniform1i(metallicProgram->hasMetallicMapLocation, hasMetallicMap);

		glUniform1i(metallicProgram->metallicMapLocation, 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, glTextureMetallic);

		standardProgram = metallicProgram;
		break;
	}
	case MaterialShader::STANDARD_DISSOLVE: {
		// Standard-specific uniform settings
		ProgramStandardMetallic* metallicProgram = hasNormalMap ? App->programs->standardNormal : App->programs->standardNotNormal;
		if (metallicProgram == nullptr) return;

		glDisable(GL_CULL_FACE);

		ProgramStandardDissolve* dissolveProgram = App->programs->dissolveStandard;

		glUseProgram(dissolveProgram->program);

		// Standard-specific settings
		unsigned glTextureMetallic = 0;
		ResourceTexture* metallic = App->resources->GetResource<ResourceTexture>(material->metallicMapId);
		glTextureMetallic = metallic ? metallic->glTexture : 0;
		int hasMetallicMap = metallic ? 1 : 0;

		glUniform1f(dissolveProgram->metalnessLocation, material->metallic);
		glUniform1i(dissolveProgram->hasMetallicMapLocation, hasMetallicMap);

		glUniform1i(dissolveProgram->metallicMapLocation, 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, glTextureMetallic);

		// Dissolve settings
		glUniform4fv(dissolveProgram->colorLocation, 1, material->dissolveColor.ptr());
		glUniform1f(dissolveProgram->intensityLocation, material->dissolveIntensity);
		glUniform1f(dissolveProgram->scaleLocation, material->dissolveScale);
		glUniform1f(dissolveProgram->thresholdLocation, GetDissolveValue());
		glUniform2fv(dissolveProgram->offsetLocation, 1, material->dissolveOffset.ptr());
		glUniform1f(dissolveProgram->edgeSizeLocation, material->dissolveEdgeSize);

		standardProgram = dissolveProgram;
		break;
	}
	case MaterialShader::UNLIT: {
		ProgramUnlit* unlitProgram = App->programs->unlit;
		if (unlitProgram == nullptr) return;

		glUseProgram(unlitProgram->program);

		// Matrices
		float4x4 viewMatrix = App->camera->GetViewMatrix();
		float4x4 projMatrix = App->camera->GetProjectionMatrix();

		glUniformMatrix4fv(unlitProgram->modelLocation, 1, GL_TRUE, modelMatrix.ptr());
		glUniformMatrix4fv(unlitProgram->viewLocation, 1, GL_TRUE, viewMatrix.ptr());
		glUniformMatrix4fv(unlitProgram->projLocation, 1, GL_TRUE, projMatrix.ptr());

		if (palette.size() > 0) {
			glUniformMatrix4fv(unlitProgram->paletteLocation, palette.size(), GL_TRUE, palette[0].ptr());
		}

		glUniform1i(unlitProgram->hasBonesLocation, goBones.size());

		// Diffuse
		unsigned glTextureDiffuse = 0;
		ResourceTexture* diffuse = App->resources->GetResource<ResourceTexture>(material->diffuseMapId);
		glTextureDiffuse = diffuse ? diffuse->glTexture : 0;
		int hasDiffuseMap = diffuse ? 1 : 0;

		glUniform1i(unlitProgram->diffuseMapLocation, 0);
		glUniform4fv(unlitProgram->diffuseColorLocation, 1, material->diffuseColor.ptr());
		glUniform1i(unlitProgram->hasDiffuseMapLocation, hasDiffuseMap);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, glTextureDiffuse);

		// Emissive
		unsigned glTextureEmissive = 0;
		ResourceTexture* emissive = App->resources->GetResource<ResourceTexture>(material->emissiveMapId);
		glTextureEmissive = emissive ? emissive->glTexture : 0;
		int hasEmissiveMap = glTextureEmissive ? 1 : 0;

		glUniform1i(unlitProgram->emissiveMapLocation, 1);
		glUniform1i(unlitProgram->hasEmissiveMapLocation, hasEmissiveMap);
		glUniform1f(unlitProgram->emissiveIntensityLocation, material->emissiveIntensity);
		glUniform4fv(unlitProgram->emissiveColorLocation, 1, material->emissiveColor.ptr());
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, glTextureEmissive);

		// Tilling settings
		glUniform2fv(unlitProgram->tilingLocation, 1, ChooseTextureTiling(material->tiling).ptr());
		glUniform2fv(unlitProgram->offsetLocation, 1, ChooseTextureOffset(material->offset).ptr());

		glBindVertexArray(mesh->vao);
		glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);

		break;
	}
	case MaterialShader::UNLIT_DISSOLVE: {
		ProgramUnlitDissolve* unlitProgram = App->programs->dissolveUnlit;
		if (unlitProgram == nullptr) return;

		glUseProgram(unlitProgram->program);

		glDisable(GL_CULL_FACE);

		// Matrices
		float4x4 viewMatrix = App->camera->GetViewMatrix();
		float4x4 projMatrix = App->camera->GetProjectionMatrix();

		glUniformMatrix4fv(unlitProgram->modelLocation, 1, GL_TRUE, modelMatrix.ptr());
		glUniformMatrix4fv(unlitProgram->viewLocation, 1, GL_TRUE, viewMatrix.ptr());
		glUniformMatrix4fv(unlitProgram->projLocation, 1, GL_TRUE, projMatrix.ptr());

		if (palette.size() > 0) {
			glUniformMatrix4fv(unlitProgram->paletteLocation, palette.size(), GL_TRUE, palette[0].ptr());
		}

		glUniform1i(unlitProgram->hasBonesLocation, goBones.size());

		// Diffuse
		unsigned glTextureDiffuse = 0;
		ResourceTexture* diffuse = App->resources->GetResource<ResourceTexture>(material->diffuseMapId);
		glTextureDiffuse = diffuse ? diffuse->glTexture : 0;
		int hasDiffuseMap = diffuse ? 1 : 0;

		glUniform1i(unlitProgram->diffuseMapLocation, 0);
		glUniform4fv(unlitProgram->diffuseColorLocation, 1, material->diffuseColor.ptr());
		glUniform1i(unlitProgram->hasDiffuseMapLocation, hasDiffuseMap);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, glTextureDiffuse);

		// Emissive
		unsigned glTextureEmissive = 0;
		ResourceTexture* emissive = App->resources->GetResource<ResourceTexture>(material->emissiveMapId);
		glTextureEmissive = emissive ? emissive->glTexture : 0;
		int hasEmissiveMap = glTextureEmissive ? 1 : 0;

		glUniform1i(unlitProgram->emissiveMapLocation, 1);
		glUniform1i(unlitProgram->hasEmissiveMapLocation, hasEmissiveMap);
		glUniform1f(unlitProgram->emissiveIntensityLocation, material->emissiveIntensity);
		glUniform4fv(unlitProgram->emissiveColorLocation, 1, material->emissiveColor.ptr());

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, glTextureEmissive);

		// Tilling settings
		glUniform2fv(unlitProgram->tilingLocation, 1, ChooseTextureTiling(material->tiling).ptr());
		glUniform2fv(unlitProgram->offsetLocation, 1, ChooseTextureOffset(material->offset).ptr());

		// Dissolve settings
		glUniform4fv(unlitProgram->colorLocation, 1, material->dissolveColor.ptr());
		glUniform1f(unlitProgram->intensityLocation, material->dissolveIntensity);
		glUniform1f(unlitProgram->scaleLocation, material->dissolveScale);
		glUniform1f(unlitProgram->thresholdLocation, GetDissolveValue());
		glUniform2fv(unlitProgram->offsetLocation, 1, material->dissolveOffset.ptr());
		glUniform1f(unlitProgram->edgeSizeLocation, material->dissolveEdgeSize);

		glBindVertexArray(mesh->vao);
		glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);

		break;
	}
	case MaterialShader::VOLUMETRIC_LIGHT: {
		ProgramVolumetricLight* volumetricLightProgram = App->programs->volumetricLight;
		if (volumetricLightProgram == nullptr) return;

		glUseProgram(volumetricLightProgram->program);

		// Matrices
		float4x4 viewMatrix = App->camera->GetViewMatrix();
		float4x4 projMatrix = App->camera->GetProjectionMatrix();

		glUniformMatrix4fv(volumetricLightProgram->modelLocation, 1, GL_TRUE, modelMatrix.ptr());
		glUniformMatrix4fv(volumetricLightProgram->viewLocation, 1, GL_TRUE, viewMatrix.ptr());
		glUniformMatrix4fv(volumetricLightProgram->projLocation, 1, GL_TRUE, projMatrix.ptr());

		if (palette.size() > 0) {
			glUniformMatrix4fv(volumetricLightProgram->paletteLocation, palette.size(), GL_TRUE, palette[0].ptr());
		}

		glUniform1i(volumetricLightProgram->hasBonesLocation, goBones.size());

		glUniform3fv(volumetricLightProgram->viewPosLocation, 1, App->camera->GetPosition().ptr());

		glUniform1f(volumetricLightProgram->nearLocation, App->camera->GetNearPlane());
		glUniform1f(volumetricLightProgram->farLocation, App->camera->GetFarPlane());

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, App->renderer->depthsTexture);
		glUniform1i(volumetricLightProgram->depthsLocation, 0);

		// Light
		unsigned glTextureLight = 0;
		ResourceTexture* volumetricLightMap = App->resources->GetResource<ResourceTexture>(material->diffuseMapId);
		glTextureLight = volumetricLightMap ? volumetricLightMap->glTexture : 0;
		int hasLightMap = volumetricLightMap ? 1 : 0;

		glUniform1i(volumetricLightProgram->lightMapLocation, 1);
		glUniform4fv(volumetricLightProgram->lightColorLocation, 1, material->diffuseColor.ptr());
		glUniform1i(volumetricLightProgram->hasLightMapLocation, hasLightMap ? 1 : 0);
		glUniform1f(volumetricLightProgram->intensityLocation, material->volumetricLightInstensity);
		glUniform1f(volumetricLightProgram->attenuationExponentLocation, material->volumetricLightAttenuationExponent);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, glTextureLight);

		glUniform1i(volumetricLightProgram->isSoftLocation, material->isSoft ? 1 : 0);
		glUniform1f(volumetricLightProgram->softRangeLocation, material->softRange);

		glBindVertexArray(mesh->vao);
		glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);

		break;
	}
	}

	// Unlit material already set
	if (material->shaderType == MaterialShader::UNLIT || material->shaderType == MaterialShader::UNLIT_DISSOLVE) return;

	// Volumetric Light material already set
	if (material->shaderType == MaterialShader::VOLUMETRIC_LIGHT) return;

	// Common shader settings
	if (standardProgram == nullptr) return;

	// Light settings
	ComponentLight* directionalLight = nullptr;

	Scene* scene = GetOwner().scene;
	for (ComponentLight& light : scene->lightComponents) {
		if (light.lightType == LightType::DIRECTIONAL) {
			// It takes the first actived Directional Light inside the Pool
			if (light.IsActive()) {
				directionalLight = &light;
				break;
			}
		}
	}

	// Matrices
	float4x4 viewMatrix = App->camera->GetViewMatrix();
	float4x4 projMatrix = App->camera->GetProjectionMatrix();

	// Light frustums

	const std::vector<LightFrustum::FrustumInformation>& subsFrustumsStatic = App->renderer->lightFrustumStatic.GetSubFrustums();
	const std::vector<LightFrustum::FrustumInformation>& subsFrustumsDynamic = App->renderer->lightFrustumDynamic.GetSubFrustums();

	std::vector<float4x4> viewOrtoLightsStatic;
	std::vector<float4x4> viewOrtoLightsDynamic;
	std::vector<float4x4> projOrtoLightsStatic;
	std::vector<float4x4> projOrtoLightsDynamic;
	std::vector<float> farPlaneDistancesStatic;
	std::vector<float> farPlaneDistancesDynamic;

	// Static shadow casters
	for (unsigned int i = 0; i < subsFrustumsStatic.size(); ++i) {
		viewOrtoLightsStatic.push_back(subsFrustumsStatic[i].orthographicFrustum.ViewMatrix());
		projOrtoLightsStatic.push_back(subsFrustumsStatic[i].orthographicFrustum.ProjectionMatrix());
		farPlaneDistancesStatic.push_back(subsFrustumsStatic[i].orthographicFrustum.FarPlaneDistance());
	}

	// Dynamic shadow casters
	for (unsigned int i = 0; i < subsFrustumsDynamic.size(); ++i) {
		viewOrtoLightsDynamic.push_back(subsFrustumsDynamic[i].orthographicFrustum.ViewMatrix());
		projOrtoLightsDynamic.push_back(subsFrustumsDynamic[i].orthographicFrustum.ProjectionMatrix());
		farPlaneDistancesDynamic.push_back(subsFrustumsDynamic[i].orthographicFrustum.FarPlaneDistance());
	}

	unsigned glTextureDiffuse = 0;
	ResourceTexture* diffuse = App->resources->GetResource<ResourceTexture>(material->diffuseMapId);
	glTextureDiffuse = diffuse ? diffuse->glTexture : 0;
	int hasDiffuseMap = diffuse ? 1 : 0;

	unsigned glSSAOTexture = App->renderer->ssaoTexture;

	unsigned glTextureEmissive = 0;
	ResourceTexture* emissive = App->resources->GetResource<ResourceTexture>(material->emissiveMapId);
	glTextureEmissive = emissive ? emissive->glTexture : 0;
	int hasEmissiveMap = glTextureEmissive ? 1 : 0;

	unsigned glTextureAmbientOcclusion = 0;
	ResourceTexture* ambientOcclusion = App->resources->GetResource<ResourceTexture>(material->ambientOcclusionMapId);
	glTextureAmbientOcclusion = ambientOcclusion ? ambientOcclusion->glTexture : 0;
	int hasAmbientOcclusionMap = glTextureAmbientOcclusion ? 1 : 0;

	// Common uniform settings
	glUniformMatrix4fv(standardProgram->modelLocation, 1, GL_TRUE, modelMatrix.ptr());
	glUniformMatrix4fv(standardProgram->viewLocation, 1, GL_TRUE, viewMatrix.ptr());
	glUniformMatrix4fv(standardProgram->projLocation, 1, GL_TRUE, projMatrix.ptr());

	// Shadows uniform settings
	if (subsFrustumsDynamic.size() > 0) {
		glUniformMatrix4fv(standardProgram->viewOrtoLightsDynamicLocation, viewOrtoLightsDynamic.size(), GL_TRUE, viewOrtoLightsDynamic[0].ptr());
		glUniformMatrix4fv(standardProgram->projOrtoLightsDynamicLocation, projOrtoLightsDynamic.size(), GL_TRUE, projOrtoLightsDynamic[0].ptr());
	}

	if (subsFrustumsStatic.size() > 0) {
		glUniformMatrix4fv(standardProgram->viewOrtoLightsStaticLocation, viewOrtoLightsStatic.size(), GL_TRUE, viewOrtoLightsStatic[0].ptr());
		glUniformMatrix4fv(standardProgram->projOrtoLightsStaticLocation, projOrtoLightsStatic.size(), GL_TRUE, projOrtoLightsStatic[0].ptr());
	}

	for (unsigned int i = 0; i < farPlaneDistancesStatic.size(); ++i) {
		glUniform1f(standardProgram->depthMaps[i].farPlaneLocationStatic, farPlaneDistancesStatic[i]);
		glUniform1f(standardProgram->depthMaps[i].farPlaneLocationDynamic, farPlaneDistancesDynamic[i]);
	}

	glUniform1ui(standardProgram->shadowCascadesCounterLocation, subsFrustumsDynamic.size());

	// Skinning uniform settings

	if (palette.size() > 0) {
		glUniformMatrix4fv(standardProgram->paletteLocation, palette.size(), GL_TRUE, palette[0].ptr());
	}

	glUniform1i(standardProgram->hasBonesLocation, goBones.size());

	glUniform3fv(standardProgram->viewPosLocation, 1, App->camera->GetPosition().ptr());

	// Diffuse
	glUniform1i(standardProgram->diffuseMapLocation, 0);
	glUniform4fv(standardProgram->diffuseColorLocation, 1, material->diffuseColor.ptr());
	glUniform1i(standardProgram->hasDiffuseMapLocation, hasDiffuseMap);
	glUniform1f(standardProgram->smoothnessLocation, material->smoothness);
	glUniform1i(standardProgram->hasSmoothnessAlphaLocation, material->hasSmoothnessInAlphaChannel);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, glTextureDiffuse);

	// Normal Map
	glUniform1i(standardProgram->normalMapLocation, 2);
	glUniform1i(standardProgram->hasNormalMapLocation, hasNormalMap);
	glUniform1f(standardProgram->normalStrengthLocation, material->normalStrength);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, glTextureNormal);

	// Emissive Map
	glUniform1i(standardProgram->emissiveMapLocation, 3);
	glUniform1i(standardProgram->hasEmissiveMapLocation, hasEmissiveMap);
	glUniform1f(standardProgram->emissiveIntensityLocation, material->emissiveIntensity);
	glUniform4fv(standardProgram->emissiveColorLocation, 1, material->emissiveColor.ptr());

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, glTextureEmissive);

	// Ambient Occlusion Map
	glUniform1i(standardProgram->ambientOcclusionMapLocation, 4);
	glUniform1i(standardProgram->hasAmbientOcclusionMapLocation, hasAmbientOcclusionMap);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, glTextureAmbientOcclusion);

	// SSAO texture
	glUniform1i(standardProgram->ssaoTextureLocation, 5);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, glSSAOTexture);
	glUniform1f(standardProgram->ssaoDirectLightingStrengthLocation, App->renderer->ssaoDirectLightingStrength);

	// Tilling settings
	glUniform2fv(standardProgram->tilingLocation, 1, ChooseTextureTiling(material->tiling).ptr());
	glUniform2fv(standardProgram->offsetLocation, 1, ChooseTextureOffset(material->offset).ptr());

	// IBL textures
	auto skyboxIt = scene->skyboxComponents.begin();
	bool hasIBL = false;
	if (skyboxIt != scene->skyboxComponents.end()) {
		ComponentSkyBox& skyboxComponent = *skyboxIt;
		ResourceSkybox* skyboxResource = App->resources->GetResource<ResourceSkybox>(skyboxComponent.GetSkyboxResourceID());

		if (skyboxResource != nullptr) {
			hasIBL = true;
			
			glUniform1i(standardProgram->diffuseIBLLocation, 6);
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxResource->GetGlIrradianceMap());

			glUniform1i(standardProgram->prefilteredIBLLocation, 7);
			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxResource->GetGlPreFilteredMap());

			glUniform1i(standardProgram->environmentBRDFLocation, 8);
			glActiveTexture(GL_TEXTURE8);
			glBindTexture(GL_TEXTURE_2D, skyboxResource->GetGlEnvironmentBRDF());

			glUniform1i(standardProgram->prefilteredIBLNumLevelsLocation, skyboxResource->GetPreFilteredMapNumLevels());

			glUniform1f(standardProgram->strengthIBLLocation, skyboxComponent.strength);
		}
	}
	glUniform1i(standardProgram->hasIBLLocation, hasIBL ? 1 : 0);

	unsigned int total = 9;
	for (unsigned int i = 0; i < subsFrustumsStatic.size() * 2 && total < 32; i += 2, total += 2) {
		
		unsigned int gldepthMapTexture = App->renderer->depthMapStaticTextures[i];

		glUniform1i(standardProgram->depthMaps[i].depthMapLocationStatic, total);
		glActiveTexture(GL_TEXTURE9 + i);
		glBindTexture(GL_TEXTURE_2D, gldepthMapTexture);

		gldepthMapTexture = App->renderer->depthMapDynamicTextures[i];

		glUniform1i(standardProgram->depthMaps[i].depthMapLocationDynamic, total + 1);
		glActiveTexture(GL_TEXTURE9 + i + 1);
		glBindTexture(GL_TEXTURE_2D, gldepthMapTexture);

	}

	// Lights uniforms settings
	glUniform3fv(standardProgram->ambientColorLocation, 1, App->renderer->ambientColor.ptr());

	if (directionalLight != nullptr) {
		glUniform3fv(standardProgram->dirLightDirectionLocation, 1, directionalLight->direction.ptr());
		glUniform3fv(standardProgram->dirLightColorLocation, 1, directionalLight->color.ptr());
		glUniform1f(standardProgram->dirLightIntensityLocation, directionalLight->intensity);
	}
	glUniform1i(standardProgram->dirLightIsActiveLocation, directionalLight ? 1 : 0);

	glUniform1i(standardProgram->tilesPerRowLocation, App->renderer->GetLightTilesPerRow());

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, App->renderer->lightsStorageBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, App->renderer->lightIndicesStorageBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, App->renderer->lightTilesStorageBuffer);

	glBindVertexArray(mesh->vao);
	glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

void ComponentMeshRenderer::DrawDepthPrepass(const float4x4& modelMatrix) const {
	if (!IsActive()) return;

	ResourceMesh* mesh = App->resources->GetResource<ResourceMesh>(meshId);
	if (mesh == nullptr) return;

	ResourceMaterial* material = App->resources->GetResource<ResourceMaterial>(materialId);
	if (material == nullptr) return;

	ProgramDepthPrepass* depthPrepassProgram = App->programs->depthPrepass;
	bool mustUseDissolveProgram = material->shaderType == MaterialShader::UNLIT_DISSOLVE || material->shaderType == MaterialShader::STANDARD_DISSOLVE;

	if (mustUseDissolveProgram) {
		ProgramDepthPrepassDissolve* depthPrepassProgramDissolve = App->programs->depthPrepassDissolve;
		if (depthPrepassProgramDissolve == nullptr) return;

		depthPrepassProgram = depthPrepassProgramDissolve;

		glUseProgram(depthPrepassProgram->program);

		glUniform1f(depthPrepassProgramDissolve->scaleLocation, material->dissolveScale);
		glUniform1f(depthPrepassProgramDissolve->thresholdLocation, GetDissolveValue());
		glUniform2fv(depthPrepassProgramDissolve->offsetLocation, 1, material->dissolveOffset.ptr());
	} else {
		if (depthPrepassProgram == nullptr) return;
		glUseProgram(depthPrepassProgram->program);
	}

	float4x4 viewMatrix = App->camera->GetViewMatrix();
	float4x4 projMatrix = App->camera->GetProjectionMatrix();

	// Common uniform settings
	glUniformMatrix4fv(depthPrepassProgram->modelLocation, 1, GL_TRUE, modelMatrix.ptr());
	glUniformMatrix4fv(depthPrepassProgram->viewLocation, 1, GL_TRUE, viewMatrix.ptr());
	glUniformMatrix4fv(depthPrepassProgram->projLocation, 1, GL_TRUE, projMatrix.ptr());

	// Skinning
	if (palette.size() > 0) {
		glUniformMatrix4fv(depthPrepassProgram->paletteLocation, palette.size(), GL_TRUE, palette[0].ptr());
	}

	glUniform1i(depthPrepassProgram->hasBonesLocation, goBones.size());

	// Diffuse
	unsigned glTextureDiffuse = 0;
	ResourceTexture* diffuse = App->resources->GetResource<ResourceTexture>(material->diffuseMapId);
	glTextureDiffuse = diffuse ? diffuse->glTexture : 0;
	int hasDiffuseMap = diffuse ? 1 : 0;

	glUniform1i(depthPrepassProgram->diffuseMapLocation, 0);
	glUniform4fv(depthPrepassProgram->diffuseColorLocation, 1, material->diffuseColor.ptr());
	glUniform1i(depthPrepassProgram->hasDiffuseMapLocation, hasDiffuseMap);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, glTextureDiffuse);

	// Tiling settings
	glUniform2fv(depthPrepassProgram->tilingLocation, 1, ChooseTextureTiling(material->tiling).ptr());
	glUniform2fv(depthPrepassProgram->offsetLocation, 1, ChooseTextureOffset(material->offset).ptr());

	glBindVertexArray(mesh->vao);
	glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);

	glEnable(GL_CULL_FACE);
}

void ComponentMeshRenderer::DrawShadow(const float4x4& modelMatrix, unsigned int i, ShadowCasterType lightFrustumType) const {
	if (!IsActive()) return;

	ResourceMesh* mesh = App->resources->GetResource<ResourceMesh>(meshId);
	ResourceMaterial* material = App->resources->GetResource<ResourceMaterial>(materialId);
	if (mesh == nullptr || material == nullptr) return;

	unsigned program = App->programs->shadowMap;
	float4x4 viewMatrix = App->renderer->GetLightViewMatrix(i, lightFrustumType);
	float4x4 projMatrix = App->renderer->GetLightProjectionMatrix(i, lightFrustumType);

	unsigned glTextureDiffuse = 0;
	ResourceTexture* diffuse = App->resources->GetResource<ResourceTexture>(material->diffuseMapId);
	glTextureDiffuse = diffuse ? diffuse->glTexture : 0;
	int hasDiffuseMap = diffuse ? 1 : 0;

	glUseProgram(program);

	// Common uniform settings
	glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_TRUE, modelMatrix.ptr());
	glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_TRUE, viewMatrix.ptr());
	glUniformMatrix4fv(glGetUniformLocation(program, "proj"), 1, GL_TRUE, projMatrix.ptr());

	glUniform1i(glGetUniformLocation(program, "diffuseMap"), 0);
	glUniform4fv(glGetUniformLocation(program, "diffuseColor"), 1, material->diffuseColor.ptr());
	glUniform1i(glGetUniformLocation(program, "hasDiffuseMap"), hasDiffuseMap);

	glUniform2fv(glGetUniformLocation(program, "tiling"), 1, material->tiling.ptr());
	glUniform2fv(glGetUniformLocation(program, "offset"), 1, material->offset.ptr());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, glTextureDiffuse);

	// Skinning
	if (palette.size() > 0) {
		glUniformMatrix4fv(glGetUniformLocation(program, "palette"), palette.size(), GL_TRUE, palette[0].ptr());
	}

	glUniform1i(glGetUniformLocation(program, "hasBones"), goBones.size());

	glBindVertexArray(mesh->vao);
	glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

void ComponentMeshRenderer::AddRenderingModeMask() {
	ResourceMaterial* material = App->resources->GetResource<ResourceMaterial>(materialId);
	if (material && material->renderingMode == RenderingMode::TRANSPARENT) {
		GameObject& gameObject = GetOwner();
		gameObject.AddMask(MaskType::TRANSPARENT);
	}
}

void ComponentMeshRenderer::DeleteRenderingModeMask() {
	ResourceMaterial* material = App->resources->GetResource<ResourceMaterial>(materialId);
	if (material && material->renderingMode == RenderingMode::TRANSPARENT) {
		GameObject& gameObject = GetOwner();
		gameObject.DeleteMask(MaskType::TRANSPARENT);
	}
}

void ComponentMeshRenderer::PlayDissolveAnimation(bool reverse) {
	dissolveAnimationFinished = false;
	dissolveAnimationReverse = reverse;
	currentTime = 0.0f;
	dissolveThreshold = 0.0f;
}

void ComponentMeshRenderer::ResetDissolveValues() {
	dissolveThreshold = 0.0f;
	currentTime = 0.0f;
	dissolveAnimationFinished = true;
	dissolveAnimationReverse = false;
}

float ComponentMeshRenderer::GetDissolveDuration() const {
	ResourceMaterial* material = App->resources->GetResource<ResourceMaterial>(materialId);
	if (!material) return 0.0f;
	return material->dissolveDuration;
}

float ComponentMeshRenderer::GetDissolveThreshold() const {
	return dissolveThreshold;
}

bool ComponentMeshRenderer::HasDissolveAnimationFinished() const {
	return GetDissolveValue() == 1.0f;
}

bool ComponentMeshRenderer::IsDissolvePlayingOnReverse() const {
	return dissolveAnimationReverse;
}

float2 ComponentMeshRenderer::GetTextureTiling() const {
	return textureTiling;
}

float2 ComponentMeshRenderer::GetTextureOffset() const {
	return textureOffset;
}

void ComponentMeshRenderer::SetTextureTiling(float2 _tiling) {
	textureTiling = _tiling;
}

void ComponentMeshRenderer::SetTextureOffset(float2 _offset) {
	textureOffset = _offset;
}

void ComponentMeshRenderer::UpdateDissolveAnimation() {
	ResourceMaterial* material = App->resources->GetResource<ResourceMaterial>(materialId);
	if (!material) return;

	if (!dissolveAnimationFinished && material->dissolveDuration > 0) {
		currentTime += App->time->GetDeltaTimeOrRealDeltaTime();
		if (currentTime < material->dissolveDuration) {
			dissolveThreshold = currentTime / material->dissolveDuration;
		} else {
			dissolveAnimationFinished = true;
			dissolveThreshold = 1.0f;
		}
	}
}

float ComponentMeshRenderer::GetDissolveValue() const {
	return dissolveAnimationReverse ? 1.0f - dissolveThreshold : dissolveThreshold;
}

float2 ComponentMeshRenderer::ChooseTextureTiling(float2 value) const {
	if (textureTiling[0] - 1.0f > 0.0001f || textureTiling[1] - 1.0f > 0.0001f) {
		return textureTiling;
	} else {
		return value;
	}
}

float2 ComponentMeshRenderer::ChooseTextureOffset(float2 value) const {
	if (textureOffset[0] > 0.0001f || textureOffset[1] > 0.0001f) {
		return textureOffset;
	} else {
		return value;
	}
}
