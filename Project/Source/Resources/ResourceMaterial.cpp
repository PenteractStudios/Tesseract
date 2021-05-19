#include "ResourceMaterial.h"

#include "Application.h"

#include "Modules/ModuleFiles.h"
#include "FileSystem/JsonValue.h"
#include "Modules/ModuleTime.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModuleEditor.h"
#include "ResourceTexture.h"
#include "Utils/FileDialog.h"
#include "Utils/Logging.h"
#include "Utils/Buffer.h"
#include "Utils/ImGuiUtils.h"

#include "rapidjson/error/en.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/document.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "Utils/Leaks.h"

#define JSON_TAG_SHADER "ShaderType"
#define JSON_TAG_DIFFUSE_COLOR "DiffuseColor"
#define JSON_TAG_DIFFUSE_MAP "DiffuseMap"
#define JSON_TAG_SPECULAR_COLOR "SpecularColor"
#define JSON_TAG_SPECULAR_MAP "SpecularMap"
#define JSON_TAG_METALLIC_MAP "MetallicMap"
#define JSON_TAG_METALLIC "Metalness"
#define JSON_TAG_NORMAL_MAP "NormalMap"
#define JSON_TAG_SMOOTHNESS "Smoothness"
#define JSON_TAG_HAS_SMOOTHNESS_IN_ALPHA_CHANNEL "HasSmoothnessInAlphaChannel"
#define JSON_TAG_TILING "Tiling"
#define JSON_TAG_OFFSET "Offset"

void ResourceMaterial::Load() {
	// Timer to measure loading a material
	MSTimer timer;
	timer.Start();
	std::string filePath = GetResourceFilePath();
	LOG("Loading material from path: \"%s\".", filePath.c_str());

	Buffer<char> buffer = App->files->Load(filePath.c_str());
	if (buffer.Size() == 0) return;

	rapidjson::Document document;
	document.ParseInsitu<rapidjson::kParseNanAndInfFlag>(buffer.Data());
	if (document.HasParseError()) {
		LOG("Error parsing JSON: %s (offset: %u)", rapidjson::GetParseError_En(document.GetParseError()), document.GetErrorOffset());
		return;
	}
	JsonValue jMaterial(document, document);

	shaderType = (MaterialShader)(int) jMaterial[JSON_TAG_SHADER];

	diffuseColor = float4(jMaterial[JSON_TAG_DIFFUSE_COLOR][0], jMaterial[JSON_TAG_DIFFUSE_COLOR][1], jMaterial[JSON_TAG_DIFFUSE_COLOR][2], jMaterial[JSON_TAG_DIFFUSE_COLOR][3]);
	diffuseMapId = jMaterial[JSON_TAG_DIFFUSE_MAP];
	App->resources->IncreaseReferenceCount(diffuseMapId);

	specularColor = float4(jMaterial[JSON_TAG_SPECULAR_COLOR][0], jMaterial[JSON_TAG_SPECULAR_COLOR][1], jMaterial[JSON_TAG_SPECULAR_COLOR][2], jMaterial[JSON_TAG_SPECULAR_COLOR][3]);
	specularMapId = jMaterial[JSON_TAG_SPECULAR_MAP];
	App->resources->IncreaseReferenceCount(specularMapId);

	metallic = jMaterial[JSON_TAG_METALLIC];
	metallicMapId = jMaterial[JSON_TAG_METALLIC_MAP];
	App->resources->IncreaseReferenceCount(metallicMapId);

	normalMapId = jMaterial[JSON_TAG_NORMAL_MAP];
	App->resources->IncreaseReferenceCount(normalMapId);

	smoothness = jMaterial[JSON_TAG_SMOOTHNESS];
	hasSmoothnessInAlphaChannel = jMaterial[JSON_TAG_HAS_SMOOTHNESS_IN_ALPHA_CHANNEL];

	tiling = float2(jMaterial[JSON_TAG_TILING][0], jMaterial[JSON_TAG_TILING][1]);
	offset = float2(jMaterial[JSON_TAG_OFFSET][0], jMaterial[JSON_TAG_OFFSET][1]);

	unsigned timeMs = timer.Stop();
	LOG("Material loaded in %ums", timeMs);
}

void ResourceMaterial::Unload() {
	App->resources->DecreaseReferenceCount(diffuseMapId);
	App->resources->DecreaseReferenceCount(specularMapId);
	App->resources->DecreaseReferenceCount(metallicMapId);
	App->resources->DecreaseReferenceCount(normalMapId);
}

void ResourceMaterial::SaveToFile(const char* filePath) {
	LOG("Saving material to path: \"%s\".", filePath);

	MSTimer timer;
	timer.Start();

	// Create document
	rapidjson::Document document;
	JsonValue jMaterial(document, document);

	// Save JSON values
	jMaterial[JSON_TAG_SHADER] = (int) shaderType;

	JsonValue jDiffuseColor = jMaterial[JSON_TAG_DIFFUSE_COLOR];
	jDiffuseColor[0] = diffuseColor.x;
	jDiffuseColor[1] = diffuseColor.y;
	jDiffuseColor[2] = diffuseColor.z;
	jDiffuseColor[3] = diffuseColor.w;
	jMaterial[JSON_TAG_DIFFUSE_MAP] = diffuseMapId;

	JsonValue jSpecularColor = jMaterial[JSON_TAG_SPECULAR_COLOR];
	jSpecularColor[0] = specularColor.x;
	jSpecularColor[1] = specularColor.y;
	jSpecularColor[2] = specularColor.z;
	jSpecularColor[3] = specularColor.w;
	jMaterial[JSON_TAG_SPECULAR_MAP] = specularMapId;

	jMaterial[JSON_TAG_METALLIC] = metallic;
	jMaterial[JSON_TAG_METALLIC_MAP] = metallicMapId;
	jMaterial[JSON_TAG_NORMAL_MAP] = normalMapId;

	jMaterial[JSON_TAG_SMOOTHNESS] = smoothness;
	jMaterial[JSON_TAG_HAS_SMOOTHNESS_IN_ALPHA_CHANNEL] = hasSmoothnessInAlphaChannel;

	JsonValue jTiling = jMaterial[JSON_TAG_TILING];
	jTiling[0] = tiling.x;
	jTiling[1] = tiling.y;
	JsonValue jOffset = jMaterial[JSON_TAG_OFFSET];
	jOffset[0] = offset.x;
	jOffset[1] = offset.y;

	// Write document to buffer
	rapidjson::StringBuffer stringBuffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>, rapidjson::UTF8<>, rapidjson::CrtAllocator, rapidjson::kWriteNanAndInfFlag> writer(stringBuffer);
	document.Accept(writer);

	// Save to file
	bool saved = App->files->Save(filePath, stringBuffer.GetString(), stringBuffer.GetSize());
	if (!saved) {
		LOG("Failed to save material resource.");
		return;
	}

	unsigned timeMs = timer.Stop();
	LOG("Material saved in %ums", timeMs);
}

void ResourceMaterial::OnEditorUpdate() {
	// Save Material
	if (ImGui::Button("Save Material")) {
		SaveToFile(GetResourceFilePath().c_str());
	}

	// Shader types
	ImGui::TextColored(App->editor->titleColor, "Shader");
	const char* shaderTypes[] = {"[Legacy] Phong", "Standard (specular settings)", "Standard"};
	const char* shaderTypesCurrent = shaderTypes[(int) shaderType];
	if (ImGui::BeginCombo("Shader Type", shaderTypesCurrent)) {
		for (int n = 0; n < IM_ARRAYSIZE(shaderTypes); ++n) {
			bool isSelected = (shaderTypesCurrent == shaderTypes[n]);
			if (ImGui::Selectable(shaderTypes[n], isSelected)) {
				shaderType = (MaterialShader) n;
			}
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
		ImGui::Text("");
	}

	//Diffuse
	ImGui::TextColored(App->editor->titleColor, "Albedo");
	ImGui::ColorEdit4("Diffuse Color##color_d", diffuseColor.ptr(), ImGuiColorEditFlags_NoInputs);
	ImGui::ResourceSlot<ResourceTexture>("Diffuse Texture", &diffuseMapId);
	if (ImGui::Button("Remove texture map##diffuse")) {
		if (diffuseMapId != 0) {
			App->resources->DecreaseReferenceCount(diffuseMapId);
			diffuseMapId = 0;
		}
	}

	if (shaderType == MaterialShader::PHONG) {
		// Specular Options
		ImGui::TextColored(App->editor->titleColor, "Specular");
		if (specularMapId == 0) {
			ImGui::ColorEdit4("Specular Color##color_s", specularColor.ptr(), ImGuiColorEditFlags_NoInputs);
		}
		ImGui::ResourceSlot<ResourceTexture>("Specular Texture", &specularMapId);
		if (ImGui::Button("Remove texture map##specular")) {
			if (specularMapId != 0) {
				App->resources->DecreaseReferenceCount(specularMapId);
				specularMapId = 0;
			}
		}

		// Shininess Options
		ImGui::TextColored(App->editor->titleColor, "Shininess");
		const char* shininessItems[] = {"Shininess Value", "Shininess Alpha"};
		const char* shininessItemCurrent = shininessItems[hasSmoothnessInAlphaChannel];
		if (ImGui::BeginCombo("##shininess", shininessItemCurrent)) {
			for (int n = 0; n < IM_ARRAYSIZE(shininessItems); ++n) {
				bool isSelected = (shininessItemCurrent == shininessItems[n]);
				if (ImGui::Selectable(shininessItems[n], isSelected)) {
					hasSmoothnessInAlphaChannel = n ? 1 : 0;
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		if (!hasSmoothnessInAlphaChannel) {
			std::string id_s("##shininess_");
			ImGui::PushID(id_s.c_str());
			ImGui::DragFloat(id_s.c_str(), &smoothness, App->editor->dragSpeed1f, 0, 1000);
			ImGui::PopID();
		}

	} else if (shaderType == MaterialShader::STANDARD_SPECULAR) {
		// Specular Options
		ImGui::TextColored(App->editor->titleColor, "Specular");
		if (specularMapId == 0) {
			ImGui::ColorEdit4("Specular Color##color_s", specularColor.ptr(), ImGuiColorEditFlags_NoInputs);
		}
		ImGui::ResourceSlot<ResourceTexture>("Specular Texture", &specularMapId);
		if (ImGui::Button("Remove texture map##specular")) {
			if (specularMapId != 0) {
				App->resources->DecreaseReferenceCount(specularMapId);
				specularMapId = 0;
			}
		}

		// Smoothness Options
		ImGui::TextColored(App->editor->titleColor, "Smoothness");
		ImGui::Text("Smoothness");

		const char* smoothnessItems[] = {"Diffuse Alpha", "Specular Alpha"};
		const char* smoothnessItemCurrent = smoothnessItems[hasSmoothnessInAlphaChannel];
		if (ImGui::BeginCombo("##smoothness", smoothnessItemCurrent)) {
			for (int n = 0; n < IM_ARRAYSIZE(smoothnessItems); ++n) {
				bool isSelected = (smoothnessItemCurrent == smoothnessItems[n]);
				if (ImGui::Selectable(smoothnessItems[n], isSelected)) {
					hasSmoothnessInAlphaChannel = n;
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		ImGui::SliderFloat("##smooth_", &smoothness, 0.0, 1.0);

	} else if (shaderType == MaterialShader::STANDARD) {
		// Metallic Options
		ImGui::TextColored(App->editor->titleColor, "Metalness");
		if (metallicMapId == 0) {
			ImGui::SliderFloat("##metalness", &metallic, 0.0, 1.0);
		}
		ImGui::ResourceSlot<ResourceTexture>("Metallic Texture", &metallicMapId);
		if (ImGui::Button("Remove texture map##metallic")) {
			if (metallicMapId != 0) {
				App->resources->DecreaseReferenceCount(metallicMapId);
				metallicMapId = 0;
			}
		}

		// Smoothness Options
		ImGui::TextColored(App->editor->titleColor, "Smoothness");
		ImGui::Text("Smoothness");

		const char* smoothnessItems[] = {"Diffuse Alpha", "Specular Alpha"};
		const char* smoothnessItemCurrent = smoothnessItems[hasSmoothnessInAlphaChannel];
		if (ImGui::BeginCombo("##smoothness", smoothnessItemCurrent)) {
			for (int n = 0; n < IM_ARRAYSIZE(smoothnessItems); ++n) {
				bool isSelected = (smoothnessItemCurrent == smoothnessItems[n]);
				if (ImGui::Selectable(smoothnessItems[n], isSelected)) {
					hasSmoothnessInAlphaChannel = n;
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		ImGui::SliderFloat("##smooth_", &smoothness, 0.0, 1.0);
	}

	// Normal Options
	ImGui::TextColored(App->editor->titleColor, "Normal Mapping");
	ImGui::ResourceSlot<ResourceTexture>("Normal Texture", &normalMapId);
	if (ImGui::Button("Remove texture map##normal")) {
		if (normalMapId != 0) {
			App->resources->DecreaseReferenceCount(normalMapId);
			normalMapId = 0;
		}
	}
	if (normalMapId != 0) {
		ImGui::SliderFloat("##strength", &normalStrength, 0.0, 10.0);
	}

	ImGui::NewLine();

	// Tiling Settingsextension
	ImGui::DragFloat2("Tiling", tiling.ptr(), App->editor->dragSpeed1f, 1, inf);
	ImGui::DragFloat2("Offset", offset.ptr(), App->editor->dragSpeed3f, -inf, inf);
}