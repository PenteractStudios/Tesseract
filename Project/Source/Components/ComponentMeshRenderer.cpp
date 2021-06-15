#include "ComponentMeshRenderer.h"

#include "Globals.h"
#include "Application.h"
#include "Utils/Logging.h"
#include "Utils/ImGuiUtils.h"
#include "Utils/FileDialog.h"
#include "FileSystem/TextureImporter.h"
#include "GameObject.h"
#include "Resources/ResourceMaterial.h"
#include "Resources/ResourceMesh.h"
#include "Resources/ResourceTexture.h"
#include "Components/ComponentTransform.h"
#include "Components/ComponentLight.h"
#include "Components/ComponentBoundingBox.h"
#include "Components/ComponentAnimation.h"
#include "Modules/ModulePrograms.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModuleCamera.h"
#include "Modules/ModuleRender.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleTime.h"

#include "assimp/mesh.h"
#include "GL/glew.h"
#include "imgui.h"

#include "Utils/Leaks.h"

#define JSON_TAG_MESH_ID "MeshID"
#define JSON_TAG_MATERIAL_ID "MaterialID"

#define POINT_LIGHT_MEMBERS 6
#define POINT_LIGHT_STRING(number)                 \
	{                                              \
		"light.points["##number "].pos",           \
			"light.points["##number "].color",     \
			"light.points["##number "].intensity", \
			"light.points["##number "].kc",        \
			"light.points["##number "].kl",        \
			"light.points["##number "].kq"         \
	}

#define SPOT_LIGHT_MEMBERS 9
#define SPOT_LIGHT_STRING(number)                  \
	{                                              \
		"light.spots["##number "].pos",            \
			"light.spots["##number "].direction",  \
			"light.spots["##number "].color",      \
			"light.spots["##number "].intensity",  \
			"light.spots["##number "].kc",         \
			"light.spots["##number "].kl",         \
			"light.spots["##number "].kq",         \
			"light.spots["##number "].innerAngle", \
			"light.spots["##number "].outerAngle"  \
	}

static const char* pointLightStrings[POINT_LIGHTS][POINT_LIGHT_MEMBERS] = {
	POINT_LIGHT_STRING("0"),
	POINT_LIGHT_STRING("1"),
	POINT_LIGHT_STRING("2"),
	POINT_LIGHT_STRING("3"),
	POINT_LIGHT_STRING("4"),
	POINT_LIGHT_STRING("5"),
	POINT_LIGHT_STRING("6"),
	POINT_LIGHT_STRING("7"),
	POINT_LIGHT_STRING("8"),
	POINT_LIGHT_STRING("9"),
	POINT_LIGHT_STRING("10"),
	POINT_LIGHT_STRING("11"),
	POINT_LIGHT_STRING("12"),
	POINT_LIGHT_STRING("13"),
	POINT_LIGHT_STRING("14"),
	POINT_LIGHT_STRING("15"),
	POINT_LIGHT_STRING("16"),
	POINT_LIGHT_STRING("17"),
	POINT_LIGHT_STRING("18"),
	POINT_LIGHT_STRING("19"),
	POINT_LIGHT_STRING("20"),
	POINT_LIGHT_STRING("21"),
	POINT_LIGHT_STRING("22"),
	POINT_LIGHT_STRING("23"),
	POINT_LIGHT_STRING("24"),
	POINT_LIGHT_STRING("25"),
	POINT_LIGHT_STRING("26"),
	POINT_LIGHT_STRING("27"),
	POINT_LIGHT_STRING("28"),
	POINT_LIGHT_STRING("29"),
	POINT_LIGHT_STRING("30"),
	POINT_LIGHT_STRING("31")};

static const char* spotLightStrings[SPOT_LIGHTS][SPOT_LIGHT_MEMBERS] = {
	SPOT_LIGHT_STRING("0"),
	SPOT_LIGHT_STRING("1"),
	SPOT_LIGHT_STRING("2"),
	SPOT_LIGHT_STRING("3"),
	SPOT_LIGHT_STRING("4"),
	SPOT_LIGHT_STRING("5"),
	SPOT_LIGHT_STRING("6"),
	SPOT_LIGHT_STRING("7")};

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
			ImGui::TextColored(App->editor->textColor, "%d", mesh->numVertices);
			ImGui::TextWrapped("Num Triangles: ");
			ImGui::SameLine();
			ImGui::TextColored(App->editor->textColor, "%d", mesh->numIndices / 3);
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
		}
		ImGui::TreePop();
	}
}

void ComponentMeshRenderer::Init() {
	ResourceMesh* mesh = App->resources->GetResource<ResourceMesh>(meshId);
	if (!mesh) return;

	palette.resize(mesh->numBones);
	for (unsigned i = 0; i < mesh->numBones; ++i) {
		palette[i] = float4x4::identity;
	}
}

void ComponentMeshRenderer::Update() {
	ResourceMesh* mesh = App->resources->GetResource<ResourceMesh>(meshId);
	if (!mesh) return;

	if (palette.empty()) {
		palette.resize(mesh->numBones);
		for (unsigned i = 0; i < mesh->numBones; ++i) {
			palette[i] = float4x4::identity;
		}
	}

	if (App->time->GetDeltaTime() <= 0) return;

	const GameObject* parent = GetOwner().GetParent();
	const GameObject* rootBone = parent->GetRootBone();
	if (rootBone != nullptr) {
		const GameObject* rootBoneParent = rootBone->GetParent();
		const float4x4& invertedRootBoneTransform = rootBoneParent ? rootBoneParent->GetComponent<ComponentTransform>()->GetGlobalMatrix().Inverted() : float4x4::identity;

		const float4x4& localMatrix = GetOwner().GetComponent<ComponentTransform>()->GetLocalMatrix();
		for (unsigned i = 0; i < mesh->numBones; ++i) {
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
}

void ComponentMeshRenderer::Draw(const float4x4& modelMatrix) const {
	if (!IsActive()) return;

	ResourceMesh* mesh = App->resources->GetResource<ResourceMesh>(meshId);
	if (mesh == nullptr) return;

	ResourceMaterial* material = App->resources->GetResource<ResourceMaterial>(materialId);
	if (material == nullptr) return;

	// Light settings
	ComponentLight* directionalLight = nullptr;
	ComponentLight* pointLightsArray[POINT_LIGHTS];
	float pointDistancesArray[POINT_LIGHTS];
	unsigned pointLightsArraySize = 0;
	ComponentLight* spotLightsArray[SPOT_LIGHTS];
	float spotDistancesArray[SPOT_LIGHTS];
	unsigned spotLightsArraySize = 0;

	float farPointDistance = 0;
	ComponentLight* farPointLight = nullptr;
	float farSpotDistance = 0;
	ComponentLight* farSpotLight = nullptr;

	for (ComponentLight& light : GetOwner().scene->lightComponents) {
		if (light.lightType == LightType::DIRECTIONAL) {
			// It takes the first actived Directional Light inside the Pool
			if (light.IsActive() && directionalLight == nullptr) {
				directionalLight = &light;
				continue;
			}
		} else if (light.lightType == LightType::POINT) {
			if (light.IsActive()) {
				float3 meshPosition = GetOwner().GetComponent<ComponentTransform>()->GetGlobalPosition();
				float3 lightPosition = light.GetOwner().GetComponent<ComponentTransform>()->GetGlobalPosition();
				float distance = Distance(meshPosition, lightPosition);
				if (pointLightsArraySize < POINT_LIGHTS) {
					pointDistancesArray[pointLightsArraySize] = distance;
					pointLightsArray[pointLightsArraySize] = &light;
					pointLightsArraySize += 1;

					if (distance > farPointDistance) {
						farPointLight = &light;
						farPointDistance = distance;
					}
				} else {
					if (distance < farPointDistance) {
						int count = 0;
						int selected = -1;
						for (float pointDistance : pointDistancesArray) {
							if (pointDistance == farPointDistance) selected = count;
							count += 1;
						}

						pointLightsArray[selected] = &light;
						pointDistancesArray[selected] = distance;

						count = 0;
						selected = -1;
						float maxDistance = 0;
						for (float pointDistance : pointDistancesArray) {
							if (pointDistance > maxDistance) {
								maxDistance = pointDistance;
								selected = count;
							}
							count += 1;
						}

						farPointDistance = maxDistance;
						farPointLight = pointLightsArray[selected];
					}
				}
			}
		} else if (light.lightType == LightType::SPOT) {
			if (light.IsActive()) {
				float3 meshPosition = GetOwner().GetComponent<ComponentTransform>()->GetGlobalPosition();
				float3 lightPosition = light.GetOwner().GetComponent<ComponentTransform>()->GetGlobalPosition();
				float distance = Distance(meshPosition, lightPosition);
				if (spotLightsArraySize < SPOT_LIGHTS) {
					spotDistancesArray[spotLightsArraySize] = distance;
					spotLightsArray[spotLightsArraySize] = &light;
					spotLightsArraySize += 1;

					if (distance > farSpotDistance) {
						farSpotLight = &light;
						farSpotDistance = distance;
					}
				} else {
					if (distance < farSpotDistance) {
						int count = 0;
						int selected = -1;
						for (float spotDistance : spotDistancesArray) {
							if (spotDistance == farSpotDistance) selected = count;
							count += 1;
						}

						spotLightsArray[selected] = &light;
						spotDistancesArray[selected] = distance;

						count = 0;
						selected = -1;
						float maxDistance = 0;
						for (float spotDistance : spotDistancesArray) {
							if (spotDistance > maxDistance) {
								maxDistance = spotDistance;
								selected = count;
							}
							count += 1;
						}

						farSpotDistance = maxDistance;
						farSpotLight = spotLightsArray[selected];
					}
				}
			}
		}
	}

	// Common shader settings

	unsigned program = 0;
	float4x4 viewMatrix = App->camera->GetViewMatrix();
	float4x4 projMatrix = App->camera->GetProjectionMatrix();

	// Light frustum
	float4x4 viewLight = App->renderer->GetLightViewMatrix();
	float4x4 projLight = App->renderer->GetLightProjectionMatrix();

	unsigned glTextureDiffuse = 0;
	ResourceTexture* diffuse = App->resources->GetResource<ResourceTexture>(material->diffuseMapId);
	glTextureDiffuse = diffuse ? diffuse->glTexture : 0;
	int hasDiffuseMap = diffuse ? 1 : 0;

	unsigned glTextureNormal = 0;
	ResourceTexture* normal = App->resources->GetResource<ResourceTexture>(material->normalMapId);
	glTextureNormal = normal ? normal->glTexture : 0;
	int hasNormalMap = normal ? 1 : 0;

	unsigned gldepthMapTexture = App->renderer->depthMapTexture;

	unsigned glSSAOTexture = App->renderer->ssaoTexture;

	unsigned glTextureEmissive = 0;
	ResourceTexture* emissive = App->resources->GetResource<ResourceTexture>(material->emissiveMapId);
	glTextureEmissive = emissive ? emissive->glTexture : 0;
	int hasEmissiveMap = glTextureEmissive ? 1 : 0;

	unsigned glTextureAmbientOcclusion = 0;
	ResourceTexture* ambientOcclusion = App->resources->GetResource<ResourceTexture>(material->ambientOcclusionMapId);
	glTextureAmbientOcclusion = ambientOcclusion ? ambientOcclusion->glTexture : 0;
	int hasAmbientOcclusionMap = glTextureAmbientOcclusion ? 1 : 0;

	if (material->shaderType == MaterialShader::PHONG) {
		// Phong-specific settings
		unsigned glTextureSpecular = 0;
		ResourceTexture* specular = App->resources->GetResource<ResourceTexture>(material->specularMapId);
		glTextureSpecular = specular ? specular->glTexture : 0;
		int hasSpecularMap = specular ? 1 : 0;

		// Phong-specific uniform settings
		if (hasNormalMap) {
			program = App->programs->phongNormal;
		} else {
			program = App->programs->phongNotNormal;
		}

		glUseProgram(program);

		glUniform3fv(glGetUniformLocation(program, "specularColor"), 1, material->specularColor.ptr());
		glUniform1i(glGetUniformLocation(program, "hasSpecularMap"), hasSpecularMap);

		glUniform1i(glGetUniformLocation(program, "specularMap"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, glTextureSpecular);

	} else if (material->shaderType == MaterialShader::STANDARD_SPECULAR) {
		// Specular-specific settings
		unsigned glTextureSpecular = 0;
		ResourceTexture* specular = App->resources->GetResource<ResourceTexture>(material->specularMapId);
		glTextureSpecular = specular ? specular->glTexture : 0;
		int hasSpecularMap = specular ? 1 : 0;

		// Specular-specific uniform settings
		if (hasNormalMap) {
			program = App->programs->specularNormal;
		} else {
			program = App->programs->specularNotNormal;
		}

		glUseProgram(program);

		glUniform3fv(glGetUniformLocation(program, "specularColor"), 1, material->specularColor.ptr());
		glUniform1i(glGetUniformLocation(program, "hasSpecularMap"), hasSpecularMap);

		glUniform1i(glGetUniformLocation(program, "specularMap"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, glTextureSpecular);

	} else if (material->shaderType == MaterialShader::STANDARD) {
		// Standard-specific settings
		unsigned glTextureMetallic = 0;
		ResourceTexture* metallic = App->resources->GetResource<ResourceTexture>(material->metallicMapId);
		glTextureMetallic = metallic ? metallic->glTexture : 0;
		int hasMetallicMap = metallic ? 1 : 0;

		// Standard-specific uniform settings
		if (hasNormalMap) {
			program = App->programs->standardNormal;
		} else {
			program = App->programs->standardNotNormal;
		}

		glUseProgram(program);

		glUniform1f(glGetUniformLocation(program, "metalness"), material->metallic);
		glUniform1i(glGetUniformLocation(program, "hasMetallicMap"), hasMetallicMap);

		glUniform1i(glGetUniformLocation(program, "metallicMap"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, glTextureMetallic);

	} else {
		glUseProgram(program);
	}

	// Common uniform settings
	glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_TRUE, modelMatrix.ptr());
	glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_TRUE, viewMatrix.ptr());
	glUniformMatrix4fv(glGetUniformLocation(program, "proj"), 1, GL_TRUE, projMatrix.ptr());

	glUniformMatrix4fv(glGetUniformLocation(program, "viewLight"), 1, GL_TRUE, viewLight.ptr());
	glUniformMatrix4fv(glGetUniformLocation(program, "projLight"), 1, GL_TRUE, projLight.ptr());

	if (palette.size() > 0) {
		glUniformMatrix4fv(glGetUniformLocation(program, "palette"), palette.size(), GL_TRUE, palette[0].ptr());
	}

	glUniform1i(glGetUniformLocation(program, "hasBones"), goBones.size());

	glUniform1i(glGetUniformLocation(program, "light.numSpots"), spotLightsArraySize);
	glUniform3fv(glGetUniformLocation(program, "viewPos"), 1, App->camera->GetPosition().ptr());

	// Diffuse
	glUniform1i(glGetUniformLocation(program, "diffuseMap"), 0);
	glUniform4fv(glGetUniformLocation(program, "diffuseColor"), 1, material->diffuseColor.ptr());
	glUniform1i(glGetUniformLocation(program, "hasDiffuseMap"), hasDiffuseMap);
	glUniform1f(glGetUniformLocation(program, "smoothness"), material->smoothness);
	glUniform1i(glGetUniformLocation(program, "hasSmoothnessAlpha"), material->hasSmoothnessInAlphaChannel);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, glTextureDiffuse);

	// Normal Map
	glUniform1i(glGetUniformLocation(program, "normalMap"), 2);
	glUniform1i(glGetUniformLocation(program, "hasNormalMap"), hasNormalMap);
	glUniform1f(glGetUniformLocation(program, "normalStrength"), material->normalStrength);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, glTextureNormal);

	// Emissive Map
	glUniform1i(glGetUniformLocation(program, "emissiveMap"), 3);
	glUniform1i(glGetUniformLocation(program, "hasEmissiveMap"), hasEmissiveMap);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, glTextureEmissive);

	// Ambient Occlusion Map
	glUniform1i(glGetUniformLocation(program, "ambientOcclusionMap"), 4);
	glUniform1i(glGetUniformLocation(program, "hasAmbientOcclusionMap"), hasAmbientOcclusionMap);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, glTextureAmbientOcclusion);

	// Depth Map
	glUniform1i(glGetUniformLocation(program, "depthMapTexture"), 5);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, gldepthMapTexture);

	// SSAO texture
	glUniform1i(glGetUniformLocation(program, "ssaoTexture"), 6);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, glSSAOTexture);

	// Tilling settings
	glUniform2fv(glGetUniformLocation(program, "tiling"), 1, material->tiling.ptr());
	glUniform2fv(glGetUniformLocation(program, "offset"), 1, material->offset.ptr());

	// Lights uniforms settings
	glUniform3fv(glGetUniformLocation(program, "light.ambient.color"), 1, App->renderer->ambientColor.ptr());

	if (directionalLight != nullptr) {
		glUniform3fv(glGetUniformLocation(program, "light.directional.direction"), 1, directionalLight->direction.ptr());
		glUniform3fv(glGetUniformLocation(program, "light.directional.color"), 1, directionalLight->color.ptr());
		glUniform1f(glGetUniformLocation(program, "light.directional.intensity"), directionalLight->intensity);
	}
	glUniform1i(glGetUniformLocation(program, "light.directional.isActive"), directionalLight ? 1 : 0);

	for (unsigned i = 0; i < pointLightsArraySize; ++i) {
		glUniform3fv(glGetUniformLocation(program, pointLightStrings[i][0]), 1, pointLightsArray[i]->pos.ptr());
		glUniform3fv(glGetUniformLocation(program, pointLightStrings[i][1]), 1, pointLightsArray[i]->color.ptr());
		glUniform1f(glGetUniformLocation(program, pointLightStrings[i][2]), pointLightsArray[i]->intensity);
		glUniform1f(glGetUniformLocation(program, pointLightStrings[i][3]), pointLightsArray[i]->kc);
		glUniform1f(glGetUniformLocation(program, pointLightStrings[i][4]), pointLightsArray[i]->kl);
		glUniform1f(glGetUniformLocation(program, pointLightStrings[i][5]), pointLightsArray[i]->kq);
	}
	glUniform1i(glGetUniformLocation(program, "light.numPoints"), pointLightsArraySize);

	for (unsigned i = 0; i < spotLightsArraySize; ++i) {
		glUniform3fv(glGetUniformLocation(program, spotLightStrings[i][0]), 1, spotLightsArray[i]->pos.ptr());
		glUniform3fv(glGetUniformLocation(program, spotLightStrings[i][1]), 1, spotLightsArray[i]->direction.ptr());
		glUniform3fv(glGetUniformLocation(program, spotLightStrings[i][2]), 1, spotLightsArray[i]->color.ptr());
		glUniform1f(glGetUniformLocation(program, spotLightStrings[i][3]), spotLightsArray[i]->intensity);
		glUniform1f(glGetUniformLocation(program, spotLightStrings[i][4]), spotLightsArray[i]->kc);
		glUniform1f(glGetUniformLocation(program, spotLightStrings[i][5]), spotLightsArray[i]->kl);
		glUniform1f(glGetUniformLocation(program, spotLightStrings[i][6]), spotLightsArray[i]->kq);
		glUniform1f(glGetUniformLocation(program, spotLightStrings[i][7]), spotLightsArray[i]->innerAngle);
		glUniform1f(glGetUniformLocation(program, spotLightStrings[i][8]), spotLightsArray[i]->outerAngle);
	}
	glUniform1i(glGetUniformLocation(program, "light.numSpots"), spotLightsArraySize);

	glBindVertexArray(mesh->vao);
	glDrawElements(GL_TRIANGLES, mesh->numIndices, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

void ComponentMeshRenderer::DrawDepthPrepass(const float4x4& modelMatrix) const {
	if (!IsActive()) return;

	ResourceMesh* mesh = App->resources->GetResource<ResourceMesh>(meshId);
	if (mesh == nullptr) return;

	ResourceMaterial* material = App->resources->GetResource<ResourceMaterial>(materialId);
	if (material == nullptr) return;

	unsigned program = App->programs->depthPrepass;
	float4x4 viewMatrix = App->camera->GetViewMatrix();
	float4x4 projMatrix = App->camera->GetProjectionMatrix();

	glUseProgram(program);


	// Common uniform settings
	glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_TRUE, modelMatrix.ptr());
	glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_TRUE, viewMatrix.ptr());
	glUniformMatrix4fv(glGetUniformLocation(program, "proj"), 1, GL_TRUE, projMatrix.ptr());

	// Skinning
	if (palette.size() > 0) {
		glUniformMatrix4fv(glGetUniformLocation(program, "palette"), palette.size(), GL_TRUE, palette[0].ptr());
	}

	glUniform1i(glGetUniformLocation(program, "hasBones"), goBones.size());

	// Diffuse
	unsigned glTextureDiffuse = 0;
	ResourceTexture* diffuse = App->resources->GetResource<ResourceTexture>(material->diffuseMapId);
	glTextureDiffuse = diffuse ? diffuse->glTexture : 0;
	int hasDiffuseMap = diffuse ? 1 : 0;

	glUniform1i(glGetUniformLocation(program, "diffuseMap"), 0);
	glUniform4fv(glGetUniformLocation(program, "diffuseColor"), 1, material->diffuseColor.ptr());
	glUniform1i(glGetUniformLocation(program, "hasDiffuseMap"), hasDiffuseMap);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, glTextureDiffuse);

	// Tiling settings
	glUniform2fv(glGetUniformLocation(program, "tiling"), 1, material->tiling.ptr());
	glUniform2fv(glGetUniformLocation(program, "offset"), 1, material->offset.ptr());

	glBindVertexArray(mesh->vao);
	glDrawElements(GL_TRIANGLES, mesh->numIndices, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

void ComponentMeshRenderer::DrawShadow(const float4x4& modelMatrix) const {
	if (!IsActive()) return;

	ResourceMesh* mesh = App->resources->GetResource<ResourceMesh>(meshId);
	ResourceMaterial* material = App->resources->GetResource<ResourceMaterial>(materialId);
	if (mesh == nullptr || material == nullptr) return;

	unsigned program = App->programs->shadowMap;
	float4x4 viewMatrix = App->renderer->GetLightViewMatrix();
	float4x4 projMatrix = App->renderer->GetLightProjectionMatrix();

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
	glDrawElements(GL_TRIANGLES, mesh->numIndices, GL_UNSIGNED_INT, nullptr);
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
