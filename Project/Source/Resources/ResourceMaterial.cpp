#include "ResourceMaterial.h"

#include "Application.h"

#include "GameObject.h"
#include "Modules/ModuleFiles.h"
#include "FileSystem/JsonValue.h"
#include "Modules/ModuleTime.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleScene.h"
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
#define JSON_TAG_RENDERING_MODE "RenderingMode"
#define JSON_TAG_DIFFUSE_COLOR "DiffuseColor"
#define JSON_TAG_DIFFUSE_MAP "DiffuseMap"
#define JSON_TAG_SPECULAR_COLOR "SpecularColor"
#define JSON_TAG_SPECULAR_MAP "SpecularMap"
#define JSON_TAG_METALLIC_MAP "MetallicMap"
#define JSON_TAG_METALLIC "Metalness"
#define JSON_TAG_NORMAL_MAP "NormalMap"
#define JSON_TAG_NORMAL_STRENGTH "NormalStrength"
#define JSON_TAG_EMISSIVE_COLOR "EmissiveColor"
#define JSON_TAG_EMISSIVE_MAP "EmissiveMap"
#define JSON_TAG_EMISSIVE_INTENSITY "Emissive"
#define JSON_TAG_AMBIENT_OCCLUSION_MAP "AmbientOcclusionMap"
#define JSON_TAG_SMOOTHNESS "Smoothness"
#define JSON_TAG_HAS_SMOOTHNESS_IN_ALPHA_CHANNEL "HasSmoothnessInAlphaChannel"
#define JSON_TAG_TILING "Tiling"
#define JSON_TAG_OFFSET "Offset"
#define JSON_TAG_DISSOLVE_SCALE "DissolveScale"
#define JSON_TAG_DISSOLVE_OFFSET "DissolveOffset"
#define JSON_TAG_DISSOLVE_DURATION "DissolveDuration"
#define JSON_TAG_DISSOLVE_BLEND_THRESHOLD "DissolveBlendThreshold"
#define JSON_TAG_DISSOLVE_EDGE_SIZE "DissolveEdgeSize"

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

	renderingMode = (RenderingMode)(int) jMaterial[JSON_TAG_RENDERING_MODE];

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
	normalStrength = jMaterial[JSON_TAG_NORMAL_STRENGTH];

	emissiveColor = float4(jMaterial[JSON_TAG_EMISSIVE_COLOR][0], jMaterial[JSON_TAG_EMISSIVE_COLOR][1], jMaterial[JSON_TAG_EMISSIVE_COLOR][2], jMaterial[JSON_TAG_EMISSIVE_COLOR][3]);
	emissiveMapId = jMaterial[JSON_TAG_EMISSIVE_MAP];
	App->resources->IncreaseReferenceCount(emissiveMapId);

	emissiveIntensity = jMaterial[JSON_TAG_EMISSIVE_INTENSITY];

	ambientOcclusionMapId = jMaterial[JSON_TAG_AMBIENT_OCCLUSION_MAP];
	App->resources->IncreaseReferenceCount(ambientOcclusionMapId);

	smoothness = jMaterial[JSON_TAG_SMOOTHNESS];
	hasSmoothnessInAlphaChannel = jMaterial[JSON_TAG_HAS_SMOOTHNESS_IN_ALPHA_CHANNEL];

	tiling = float2(jMaterial[JSON_TAG_TILING][0], jMaterial[JSON_TAG_TILING][1]);
	offset = float2(jMaterial[JSON_TAG_OFFSET][0], jMaterial[JSON_TAG_OFFSET][1]);

	// Dissolve values
	dissolveScale = jMaterial[JSON_TAG_DISSOLVE_SCALE];
	dissolveOffset = float2(jMaterial[JSON_TAG_DISSOLVE_OFFSET][0], jMaterial[JSON_TAG_DISSOLVE_OFFSET][1]);
	dissolveDuration = jMaterial[JSON_TAG_DISSOLVE_DURATION];
	dissolveBlendThreshold = jMaterial[JSON_TAG_DISSOLVE_BLEND_THRESHOLD];
	dissolveEdgeSize = jMaterial[JSON_TAG_DISSOLVE_EDGE_SIZE];
	ResetDissolveValues();

	unsigned timeMs = timer.Stop();
	LOG("Material loaded in %ums", timeMs);
}

void ResourceMaterial::Unload() {
	App->resources->DecreaseReferenceCount(diffuseMapId);
	App->resources->DecreaseReferenceCount(specularMapId);
	App->resources->DecreaseReferenceCount(metallicMapId);
	App->resources->DecreaseReferenceCount(normalMapId);
	App->resources->DecreaseReferenceCount(emissiveMapId);
	App->resources->DecreaseReferenceCount(ambientOcclusionMapId);
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

	jMaterial[JSON_TAG_RENDERING_MODE] = (int) renderingMode;

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
	jMaterial[JSON_TAG_NORMAL_STRENGTH] = normalStrength;

	JsonValue jEmissiveColor = jMaterial[JSON_TAG_EMISSIVE_COLOR];
	jEmissiveColor[0] = emissiveColor.x;
	jEmissiveColor[1] = emissiveColor.y;
	jEmissiveColor[2] = emissiveColor.z;
	jEmissiveColor[3] = emissiveColor.w;
	jMaterial[JSON_TAG_EMISSIVE_MAP] = emissiveMapId;
	jMaterial[JSON_TAG_EMISSIVE_INTENSITY] = emissiveIntensity;
	jMaterial[JSON_TAG_AMBIENT_OCCLUSION_MAP] = ambientOcclusionMapId;

	jMaterial[JSON_TAG_SMOOTHNESS] = smoothness;
	jMaterial[JSON_TAG_HAS_SMOOTHNESS_IN_ALPHA_CHANNEL] = hasSmoothnessInAlphaChannel;

	JsonValue jTiling = jMaterial[JSON_TAG_TILING];
	jTiling[0] = tiling.x;
	jTiling[1] = tiling.y;
	JsonValue jOffset = jMaterial[JSON_TAG_OFFSET];
	jOffset[0] = offset.x;
	jOffset[1] = offset.y;

	// Dissolve values
	jMaterial[JSON_TAG_DISSOLVE_SCALE] = dissolveScale;
	JsonValue jDissolveOffset = jMaterial[JSON_TAG_DISSOLVE_OFFSET];
	jDissolveOffset[0] = dissolveOffset.x;
	jDissolveOffset[1] = dissolveOffset.y;
	jMaterial[JSON_TAG_DISSOLVE_DURATION] = dissolveDuration;
	jMaterial[JSON_TAG_DISSOLVE_BLEND_THRESHOLD] = dissolveBlendThreshold;
	jMaterial[JSON_TAG_DISSOLVE_EDGE_SIZE] = dissolveEdgeSize;

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

void ResourceMaterial::UpdateMask() {
	for (GameObject& gameObject : App->scene->scene->gameObjects) {
		ComponentMeshRenderer* meshRenderer = gameObject.GetComponent<ComponentMeshRenderer>();
		if (meshRenderer && meshRenderer->materialId == GetId()) {
			if (renderingMode == RenderingMode::TRANSPARENT) {
				gameObject.AddMask(MaskType::TRANSPARENT);
			} else {
				gameObject.DeleteMask(MaskType::TRANSPARENT);
			}
		}
	}
}

void ResourceMaterial::PlayDissolveAnimation() {
	dissolveAnimationFinished = false;
	currentTime = 0.0f;
	dissolveThreshold = 0.0f;
}

void ResourceMaterial::ResetDissolveValues() {
	dissolveThreshold = 0.0f;
	currentTime = 0.0f;
	dissolveAnimationFinished = true;
}

void ResourceMaterial::OnEditorUpdate() {
	// Save Material
	if (FileDialog::GetFileExtension(GetAssetFilePath().c_str()) == MATERIAL_EXTENSION) {
		if (ImGui::Button("Save Material")) {
			SaveToFile(GetAssetFilePath().c_str());
		}
	}

	// Shader types
	ImGui::TextColored(App->editor->titleColor, "Shader");
	const char* shaderTypes[] = {"[Legacy] Phong", "Standard (specular settings)", "Standard", "Unlit", "Standard Dissolve", "Unlit Dissolve"};
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
	}

	ImGui::NewLine();

	// Rendering Mode
	const char* renderingModes[] = {"Opaque", "Transparent"};
	const char* renderingModeCurrent = renderingModes[(int) renderingMode];
	if (ImGui::BeginCombo("Rendering Mode", renderingModeCurrent)) {
		for (int n = 0; n < IM_ARRAYSIZE(renderingModes); ++n) {
			bool isSelected = (renderingModeCurrent == renderingModes[n]);
			if (ImGui::Selectable(renderingModes[n], isSelected)) {
				renderingMode = (RenderingMode) n;
				UpdateMask();
			}
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	ImGui::NewLine();

	//Diffuse
	ImGui::BeginColumns("##diffuse_material", 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
	{
		ImGui::ResourceSlot<ResourceTexture>("Diffuse Map", &diffuseMapId);
	}
	ImGui::NextColumn();
	{
		ImGui::NewLine();
		ImGui::ColorEdit4("Color##color_d", diffuseColor.ptr(), ImGuiColorEditFlags_NoInputs);
	}
	ImGui::EndColumns();
	ImGui::NewLine();

	if (shaderType == MaterialShader::PHONG) {
		// Specular Options
		ImGui::BeginColumns("##specular_material", 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
		{
			ImGui::ResourceSlot<ResourceTexture>("Specular Map", &specularMapId);
		}
		ImGui::NextColumn();
		{
			ImGui::NewLine();
			if (specularMapId == 0) {
				ImGui::ColorEdit4("Color##color_s", specularColor.ptr(), ImGuiColorEditFlags_NoInputs);
			}
		}
		ImGui::EndColumns();

		// Shininess Options
		ImGui::Text("Shininess");
		ImGui::BeginColumns("##shininess_material", 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
		{
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
		}
		ImGui::NextColumn();
		{
			if (!hasSmoothnessInAlphaChannel) {
				std::string id_s("##shininess_");
				ImGui::PushID(id_s.c_str());
				ImGui::DragFloat(id_s.c_str(), &smoothness, App->editor->dragSpeed1f, 0, 1000);
				ImGui::PopID();
			}
		}
		ImGui::EndColumns();
		ImGui::NewLine();

	} 
	else if (shaderType != MaterialShader::UNLIT) {
		const char* smoothnessItems[] = {"Diffuse Alpha", "Specular Alpha"};

		if (shaderType == MaterialShader::STANDARD_SPECULAR) {
			// Specular Options
			ImGui::BeginColumns("##specular_material", 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
			{
				ImGui::ResourceSlot<ResourceTexture>("Specular Map", &specularMapId);
			}
			ImGui::NextColumn();
			{
				ImGui::NewLine();
				if (specularMapId == 0) {
					ImGui::ColorEdit4("Color##color_s", specularColor.ptr(), ImGuiColorEditFlags_NoInputs);
				}
			}
			ImGui::EndColumns();

		} else if (shaderType == MaterialShader::STANDARD || shaderType == MaterialShader::STANDARD_DISSOLVE) {
			// Metallic Options
			ImGui::BeginColumns("##metallic_material", 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
			{
				ImGui::ResourceSlot<ResourceTexture>("Metallic Map", &metallicMapId);
			}
			ImGui::NextColumn();
			{
				ImGui::NewLine();
				if (metallicMapId == 0) {
					ImGui::SliderFloat("##metalness", &metallic, 0.0, 1.0);
				}
			}
			ImGui::EndColumns();

			smoothnessItems[1] = "Metallic Alpha";
		}

		// Smoothness Options
		ImGui::Text("Smoothness");
		ImGui::BeginColumns("##smoothness_material", 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
		{
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
		}
		ImGui::NextColumn();
		{
			ImGui::SliderFloat("##smooth_", &smoothness, 0.0, 1.0);
		}
		ImGui::EndColumns();
		ImGui::NewLine();
	}

	if (shaderType != MaterialShader::UNLIT) {
		// Normal Options
		ImGui::BeginColumns("##normal_material", 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
		{
			ImGui::ResourceSlot<ResourceTexture>("Normal Map", &normalMapId);
		}
		ImGui::NextColumn();
		{
			ImGui::NewLine();
			if (normalMapId != 0) {
				ImGui::SliderFloat("##strength", &normalStrength, 0.0, 10.0);
			}
		}
		ImGui::EndColumns();
		ImGui::NewLine();

		// Ambient Occlusion Options
		ImGui::ResourceSlot<ResourceTexture>("Occlusion Map", &ambientOcclusionMapId);

		ImGui::NewLine();
	}

	// Emissive Options
	ImGui::BeginColumns("##emissive_map", 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
	{
		ImGui::ResourceSlot<ResourceTexture>("Emissive Map", &emissiveMapId);
	}

	ImGui::NextColumn();
	{
		ImGui::ColorEdit4("Color##color_e", emissiveColor.ptr(), ImGuiColorEditFlags_NoInputs);
		ImGui::SliderFloat("##emissiveStrength", &emissiveIntensity, 0.0, 100.0);
	}

	ImGui::EndColumns();

	ImGui::NewLine();
	ImGui::NewLine();

	// Tiling Options
	ImGui::DragFloat2("Tiling", tiling.ptr(), App->editor->dragSpeed1f, 1, inf);
	ImGui::DragFloat2("Offset", offset.ptr(), App->editor->dragSpeed3f, -inf, inf);

	if (shaderType == MaterialShader::STANDARD_DISSOLVE || shaderType == MaterialShader::UNLIT_DISSOLVE) {
		ImGui::NewLine();
		ImGui::Text("Dissolve");
		ImGui::DragFloat("Scale##dissolveScale", &dissolveScale, App->editor->dragSpeed2f, 0, inf);
		ImGui::DragFloat2("Offset##dissolveOffset", dissolveOffset.ptr(), App->editor->dragSpeed2f, -inf, inf);
		ImGui::DragFloat("Duration##dissolveScale", &dissolveDuration, App->editor->dragSpeed2f, 0, inf);
		ImGui::DragFloat("Blend Threshold##blendThreshold", &dissolveBlendThreshold, App->editor->dragSpeed2f, 0, 1);
		ImGui::DragFloat("Edge Size", &dissolveEdgeSize, App->editor->dragSpeed2f, 0, inf);

		if (ImGui::Button("Play Dissolve Animation")) {
			PlayDissolveAnimation();
		}
		if (ImGui::Button("Reset Dissolve Animation")) {
			ResetDissolveValues();
		}

		ImGui::Text("For debug only");
		ImGui::Checkbox("Animation finished", &dissolveAnimationFinished);
		ImGui::DragFloat("Threshold", &dissolveThreshold, App->editor->dragSpeed2f, 0, inf);
	}
}

void ResourceMaterial::Update() {
	if (!dissolveAnimationFinished && dissolveDuration > 0) {
		currentTime += App->time->GetDeltaTimeOrRealDeltaTime();
		if (currentTime < dissolveDuration) {
			dissolveThreshold = currentTime / dissolveDuration;
		} else {
			dissolveAnimationFinished = true;
			dissolveThreshold = 1.0f;
		}
	}
}
