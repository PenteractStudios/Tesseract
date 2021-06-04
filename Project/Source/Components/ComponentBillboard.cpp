#include "ComponentBillboard.h"

#include "GameObject.h"
#include "Components/ComponentTransform.h"
#include "Components/UI/ComponentButton.h"
#include "Application.h"
#include "Modules/ModulePrograms.h"
#include "Modules/ModuleCamera.h"
#include "Modules/ModuleRender.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModuleTime.h"
#include "Modules/ModuleUserInterface.h"
#include "Panels/PanelScene.h"
#include "Resources/ResourceTexture.h"
#include "Resources/ResourceShader.h"
#include "FileSystem/TextureImporter.h"
#include "FileSystem/JsonValue.h"
#include "Math/float3x3.h"
#include "Utils/ImGuiUtils.h"

#include "Math/TransformOps.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "GL/glew.h"
#include "debugdraw.h"

#include "Utils/Leaks.h"

#define JSON_TAG_BILLBOARD_TYPE "BillboardType"
#define JSON_TAG_TEXTURE_TEXTUREID "TextureId"

#define JSON_TAG_YTILES "Ytiles"
#define JSON_TAG_XTILES "Xtiles"
#define JSON_TAG_ANIMATION_SPEED "AnimationSpeed"

#define JSON_TAG_INIT_COLOR "InitColor"
#define JSON_TAG_FINAL_COLOR "FinalColor"
#define JSON_TAG_START_TRANSITION "StartTransition"
#define JSON_TAG_END_TRANSITION "EndTransition"

#define JSON_TAG_FLIP_TEXTURE "FlipTexture"

void ComponentBillboard::OnEditorUpdate() {
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

	const char* billboardTypeCombo[] = {"LookAt", "Stretch", "Horitzontal"};
	const char* billboardTypeComboCurrent = billboardTypeCombo[(int) billboardType];
	ImGui::TextColored(App->editor->textColor, "Type:");
	if (ImGui::BeginCombo("##Type", billboardTypeComboCurrent)) {
		for (int n = 0; n < IM_ARRAYSIZE(billboardTypeCombo); ++n) {
			bool isSelected = (billboardTypeComboCurrent == billboardTypeCombo[n]);
			if (ImGui::Selectable(billboardTypeCombo[n], isSelected)) {
				billboardType = (BillboardType) n;
			}
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	ImGui::Separator();

	ImGui::TextColored(App->editor->textColor, "Texture Settings");
	ImGui::Separator();

	ImGui::Checkbox("Random Frame", &isRandomFrame);

	UID oldID = textureID;
	ImGui::ResourceSlot<ResourceTexture>("texture", &textureID);

	ResourceTexture* textureResource = App->resources->GetResource<ResourceTexture>(textureID);

	if (textureResource != nullptr) {
		int width;
		int height;
		glGetTextureLevelParameteriv(textureResource->glTexture, 0, GL_TEXTURE_WIDTH, &width);
		glGetTextureLevelParameteriv(textureResource->glTexture, 0, GL_TEXTURE_HEIGHT, &height);

		if (oldID != textureID) {
			ComponentTransform2D* transform2D = GetOwner().GetComponent<ComponentTransform2D>();
			if (transform2D != nullptr) {
				transform2D->SetSize(float2(static_cast<float>(width), static_cast<float>(height)));
			}
		}

		ImGui::NewLine();
		ImGui::Separator();
		ImGui::TextColored(App->editor->titleColor, "Texture Preview");
		ImGui::TextWrapped("Size:");
		ImGui::SameLine();
		ImGui::TextWrapped("%i x %i", width, height);
		ImGui::Image((void*) textureResource->glTexture, ImVec2(200, 200));

		ImGui::NewLine();
		ImGui::TextColored(App->editor->titleColor, "Texture Sheet Animation");
		ImGui::DragScalar("Xtiles", ImGuiDataType_U32, &Xtiles);
		ImGui::DragScalar("Ytiles", ImGuiDataType_U32, &Ytiles);
		ImGui::DragFloat("Animation Speed", &animationSpeed, App->editor->dragSpeed2f, -inf, inf);

		ImGui::NewLine();
		ImGui::BeginColumns("##color_gradient", 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder | ImGuiColumnsFlags_NoPreserveWidths | ImGuiOldColumnFlags_NoForceWithinWindow);
		{
			ImGui::ColorEdit4("Init Color", initC.ptr(), ImGuiColorEditFlags_NoInputs);
			ImGui::ColorEdit4("Final Color", finalC.ptr(), ImGuiColorEditFlags_NoInputs);
		}
		ImGui::NextColumn();
		{
			ImGui::SliderFloat("Start##start_transition", &startTransition, 0.0f, endTransition, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::DragFloat("End##end_transition", &endTransition, startTransition, inf);
		}
		ImGui::EndColumns();

		ImGui::NewLine();
		ImGui::Text("Flip: ");
		ImGui::SameLine();
		ImGui::Checkbox("X", &flipTexture[0]);
		ImGui::SameLine();
		ImGui::Checkbox("Y", &flipTexture[1]);
	}
}

void ComponentBillboard::Load(JsonValue jComponent) {
	billboardType = (BillboardType)(int) jComponent[JSON_TAG_BILLBOARD_TYPE];

	textureID = jComponent[JSON_TAG_TEXTURE_TEXTUREID];
	if (textureID != 0) {
		App->resources->IncreaseReferenceCount(textureID);
	}

	Ytiles = jComponent[JSON_TAG_YTILES];
	Xtiles = jComponent[JSON_TAG_XTILES];
	animationSpeed = jComponent[JSON_TAG_ANIMATION_SPEED];

	JsonValue jColor = jComponent[JSON_TAG_INIT_COLOR];
	initC.Set(jColor[0], jColor[1], jColor[2]);
	JsonValue jColor2 = jComponent[JSON_TAG_FINAL_COLOR];
	finalC.Set(jColor2[0], jColor2[1], jColor2[2]);
	startTransition = jComponent[JSON_TAG_START_TRANSITION];
	endTransition = jComponent[JSON_TAG_END_TRANSITION];

	JsonValue jFlip = jComponent[JSON_TAG_FLIP_TEXTURE];
	flipTexture[0] = jFlip[0];
	flipTexture[1] = jFlip[1];
}

void ComponentBillboard::Save(JsonValue jComponent) const {
	jComponent[JSON_TAG_BILLBOARD_TYPE] = (int) billboardType;

	jComponent[JSON_TAG_TEXTURE_TEXTUREID] = textureID;

	jComponent[JSON_TAG_YTILES] = Ytiles;
	jComponent[JSON_TAG_XTILES] = Xtiles;
	jComponent[JSON_TAG_ANIMATION_SPEED] = animationSpeed;

	JsonValue jColor = jComponent[JSON_TAG_INIT_COLOR];
	jColor[0] = initC.x;
	jColor[1] = initC.y;
	jColor[2] = initC.z;
	JsonValue jColor2 = jComponent[JSON_TAG_FINAL_COLOR];
	jColor2[0] = finalC.x;
	jColor2[1] = finalC.y;
	jColor2[2] = finalC.z;
	jComponent[JSON_TAG_START_TRANSITION] = startTransition;
	jComponent[JSON_TAG_END_TRANSITION] = endTransition;

	JsonValue jFlip = jComponent[JSON_TAG_FLIP_TEXTURE];
	jFlip[0] = flipTexture[0];
	jFlip[1] = flipTexture[1];
}

void ComponentBillboard::Init() {
	ComponentTransform* transform = GetOwner().GetComponent<ComponentTransform>();
	initPos = transform->GetGlobalPosition();
	previousPos = transform->GetGlobalRotation() * float3::unitY;
	float3x3 newRotation = float3x3::FromEulerXYZ(0.f, 0.f, -pi / 2);
	modelStretch = transform->GetGlobalMatrix() * newRotation;
}

void ComponentBillboard::Update() {
	ComponentTransform* transform = GetOwner().GetComponent<ComponentTransform>();

	float3 position = transform->GetGlobalPosition();
	if (!previousPos.Equals(position)) {
		direction = (position - previousPos).Normalized();
	}
	previousPos = position;

	if (App->time->IsGameRunning()) {
		colorFrame += App->time->GetDeltaTime();
	} else {
		colorFrame += App->time->GetRealTimeDeltaTime();
	}
}

void ComponentBillboard::Draw() {
	unsigned int program = App->programs->billboard;

	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	glBindBuffer(GL_ARRAY_BUFFER, App->userInterface->GetQuadVBO());
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*) (sizeof(float) * 6 * 3));
	glUseProgram(program);

	ComponentTransform* transform = GetOwner().GetComponent<ComponentTransform>();

	float3x3 rotatePart = transform->GetGlobalMatrix().RotatePart();
	float3 position = transform->GetGlobalPosition();
	float4x4 modelMatrix = transform->GetGlobalMatrix();
	float3 scale = transform->GetGlobalScale();

	Frustum* frustum = App->camera->GetActiveCamera()->GetFrustum();
	float4x4* proj = &App->camera->GetProjectionMatrix();
	float4x4* view = &App->camera->GetViewMatrix();

	float4x4 newModelMatrix;

	if (billboardType == BillboardType::LOOK_AT) {
		newModelMatrix = modelMatrix.LookAt(rotatePart.Col(2), -frustum->Front(), rotatePart.Col(1), float3::unitY);
		newModelMatrix = float4x4::FromTRS(position, newModelMatrix.RotatePart(), scale);

	} else if (billboardType == BillboardType::STRETCH) {
		float3 cameraPos = App->camera->GetActiveCamera()->GetFrustum()->Pos();
		float3 cameraDir = (cameraPos - initPos).Normalized();
		float3 upDir = Cross(direction, cameraDir);
		float3 newCameraDir = Cross(direction, upDir);

		float3x3 newRotation;
		newRotation.SetCol(0, upDir);
		newRotation.SetCol(1, direction);
		newRotation.SetCol(2, newCameraDir);

		newModelMatrix = float4x4::FromTRS(position, newRotation * modelStretch.RotatePart(), scale);

	} else if (billboardType == BillboardType::HORIZONTAL) {
		newModelMatrix = modelMatrix.LookAt(rotatePart.Col(2), float3::unitY, rotatePart.Col(1), float3::unitY);
		newModelMatrix = float4x4::FromTRS(position, newModelMatrix.RotatePart(), scale);

	} else if (billboardType == BillboardType::VERTICAL) {
		// TODO: Implement it
		modelMatrix = modelMatrix;
	}

	glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_TRUE, newModelMatrix.ptr());
	glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_TRUE, view->ptr());
	glUniformMatrix4fv(glGetUniformLocation(program, "proj"), 1, GL_TRUE, proj->ptr());

	if (!isRandomFrame) {
		if (App->time->IsGameRunning()) {
			currentFrame += animationSpeed * App->time->GetDeltaTime();
		} else {
			currentFrame += animationSpeed * App->time->GetRealTimeDeltaTime();
		}
	}

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(program, "diffuse"), 0);
	glUniform1f(glGetUniformLocation(program, "currentFrame"), currentFrame);
	glUniform1f(glGetUniformLocation(program, "colorFrame"), colorFrame);
	glUniform4fv(glGetUniformLocation(program, "initColor"), 1, initC.ptr());
	glUniform4fv(glGetUniformLocation(program, "finalColor"), 1, finalC.ptr());
	glUniform1f(glGetUniformLocation(program, "startTransition"), startTransition);
	glUniform1f(glGetUniformLocation(program, "endTransition"), endTransition);

	glUniform1i(glGetUniformLocation(program, "Xtiles"), Xtiles);
	glUniform1i(glGetUniformLocation(program, "Ytiles"), Ytiles);

	glUniform1i(glGetUniformLocation(program, "flipX"), flipTexture[0]);
	glUniform1i(glGetUniformLocation(program, "flipY"), flipTexture[1]);

	ResourceTexture* textureResource = App->resources->GetResource<ResourceTexture>(textureID);
	if (textureResource != nullptr) {
		glBindTexture(GL_TEXTURE_2D, textureResource->glTexture);
	}
	//TODO: implement drawarrays
	//glDrawArraysInstanced(GL_TRIANGLES, 0, 4, 100);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}