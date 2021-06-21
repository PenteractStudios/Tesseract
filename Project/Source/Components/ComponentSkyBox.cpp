#include "ComponentSkyBox.h"

#include "Application.h"
#include "GameObject.h"
#include "Utils/ImGuiUtils.h"
#include "Resources/ResourceSkybox.h"
#include "Modules/ModuleCamera.h"
#include "Modules/ModuleEditor.h"
#include "ComponentSkyBox.h"
#include "Modules/ModuleRender.h"
#include "Modules/ModulePrograms.h"

#include "GL/glew.h"
#include "imgui.h"

#define JSON_TAG_SHADER "Shader"
#define JSON_TAG_SKYBOX "Skybox"

void ComponentSkyBox::Save(JsonValue jComponent) const {
	jComponent[JSON_TAG_SHADER] = shaderId;
	jComponent[JSON_TAG_SKYBOX] = skyboxId;
}

void ComponentSkyBox::Load(JsonValue jComponent) {
	shaderId = jComponent[JSON_TAG_SHADER];
	if (shaderId != 0) App->resources->IncreaseReferenceCount(shaderId);
	skyboxId = jComponent[JSON_TAG_SKYBOX];
	if (skyboxId != 0) App->resources->IncreaseReferenceCount(skyboxId);
}

void ComponentSkyBox::OnEditorUpdate() {
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
	ImGui::ResourceSlot<ResourceSkybox>("Skybox", &skyboxId);
}

void ComponentSkyBox::Draw() {
	if (!IsActive()) return;

	ResourceSkybox* skybox = App->resources->GetResource<ResourceSkybox>(skyboxId);
	if (skybox == nullptr) return;

	ProgramSkybox* skyboxProgram = App->programs->skybox;
	if (skyboxProgram == nullptr) return;

	glUseProgram(skyboxProgram->program);

	float4x4 proj = App->camera->GetProjectionMatrix();
	float4x4 view = App->camera->GetViewMatrix();
	glUniformMatrix4fv(skyboxProgram->viewLocation, 1, GL_TRUE, &view[0][0]);
	glUniformMatrix4fv(skyboxProgram->projLocation, 1, GL_TRUE, &proj[0][0]);

	glUniform1i(skyboxProgram->cubemapLocation, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->GetGlCubeMap());

	glBindVertexArray(App->renderer->cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

UID ComponentSkyBox::GetSkyboxResourceID() {
	return skyboxId;
}
