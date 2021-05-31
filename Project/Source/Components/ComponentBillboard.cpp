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
#include "GL/glew.h"
#include "debugdraw.h"

#include "Utils/Leaks.h"

#define JSON_TAG_TEXTURE_SHADERID "ShaderId"
#define JSON_TAG_TEXTURE_TEXTUREID "TextureId"
#define JSON_TAG_INITCOLOR "InitColor"
#define JSON_TAG_FINALCOLOR "FinalColor"

#define JSON_TAG_COLOR "Color"
#define JSON_TAG_YTILES "Ytiles"
#define JSON_TAG_XTILES "Xtiles"
#define JSON_TAG_ANIMATIONSPEED "AnimationSpeed"

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

	ImGui::TextColored(App->editor->textColor, "Texture Settings");
	ImGui::Separator();

	ImGui::Checkbox("Random Frame", &isRandomFrame);
	ImGui::ResourceSlot<ResourceShader>("shader", &shaderID);

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
		ImGui::DragScalar("Xtiles", ImGuiDataType_U32, &Xtiles);
		ImGui::DragScalar("Ytiles", ImGuiDataType_U32, &Ytiles);
		ImGui::DragFloat("Animation Speed", &animationSpeed, App->editor->dragSpeed2f, -inf, inf);

		ImGui::ColorEdit3("InitColor##", initC.ptr());
		ImGui::ColorEdit3("FinalColor##", finalC.ptr());
	}
}

void ComponentBillboard::Load(JsonValue jComponent) {
	shaderID = jComponent[JSON_TAG_TEXTURE_SHADERID];

	if (shaderID != 0) {
		App->resources->IncreaseReferenceCount(shaderID);
	}

	textureID = jComponent[JSON_TAG_TEXTURE_TEXTUREID];

	if (textureID != 0) {
		App->resources->IncreaseReferenceCount(textureID);
	}
	Ytiles = jComponent[JSON_TAG_YTILES];
	Xtiles = jComponent[JSON_TAG_XTILES];
	animationSpeed = jComponent[JSON_TAG_ANIMATIONSPEED];

	JsonValue jColor = jComponent[JSON_TAG_INITCOLOR];
	initC.Set(jColor[0], jColor[1], jColor[2]);

	JsonValue jColor2 = jComponent[JSON_TAG_FINALCOLOR];
	finalC.Set(jColor2[0], jColor2[1], jColor2[2]);
}

void ComponentBillboard::Save(JsonValue jComponent) const {
	jComponent[JSON_TAG_TEXTURE_SHADERID] = shaderID;
	jComponent[JSON_TAG_TEXTURE_TEXTUREID] = textureID;
	jComponent[JSON_TAG_YTILES] = Ytiles;
	jComponent[JSON_TAG_XTILES] = Xtiles;
	jComponent[JSON_TAG_ANIMATIONSPEED] = animationSpeed;

	JsonValue jColor = jComponent[JSON_TAG_INITCOLOR];
	jColor[0] = initC.x;
	jColor[1] = initC.y;
	jColor[2] = initC.z;
	JsonValue jColor2 = jComponent[JSON_TAG_FINALCOLOR];
	jColor2[0] = finalC.x;
	jColor2[1] = finalC.y;
	jColor2[2] = finalC.z;
}

void ComponentBillboard::Draw() {
	unsigned int program = 0;
	ResourceShader* shaderResouce = App->resources->GetResource<ResourceShader>(shaderID);
	if (shaderResouce) {
		program = shaderResouce->GetShaderProgram();
	} else {
		return;
	}

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
	Frustum* frustum = App->camera->GetActiveCamera()->GetFrustum();
	float4x4* proj = &App->camera->GetProjectionMatrix();
	float4x4* view = &App->camera->GetViewMatrix();

	float4x4 modelMatrix = transform->GetGlobalMatrix();
	float4x4 newModelMatrix = modelMatrix.LookAt(rotatePart.Col(2), -frustum->Front(), rotatePart.Col(1), float3::unitY);
	newModelMatrix = float4x4::FromTRS(transform->GetGlobalPosition(), newModelMatrix.RotatePart(), transform->GetGlobalScale());

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

	colorFrame += 0.01f;
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(program, "diffuse"), 0);
	glUniform1f(glGetUniformLocation(program, "currentFrame"), currentFrame);
	glUniform1f(glGetUniformLocation(program, "colorFrame"), colorFrame);
	//TODO WAIT NIL MAY NEED TO IMPROVE THIS FEATURE
	glUniform4fv(glGetUniformLocation(program, "initColor"), 1, initC.ptr());
	glUniform4fv(glGetUniformLocation(program, "finalColor"), 1, finalC.ptr());

	glUniform1i(glGetUniformLocation(program, "Xtiles"), Xtiles);
	glUniform1i(glGetUniformLocation(program, "Ytiles"), Ytiles);

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