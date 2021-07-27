#include "ComponentFog.h"

#include "Application.h"
#include "GameObject.h"
#include "Modules/ModulePrograms.h"
#include "Modules/ModuleRender.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleCamera.h"

#include "GL/glew.h"
#include "imgui.h"

#define JSON_TAG_DENSITY "Density"
#define JSON_TAG_FALLOFF "Falloff"
#define JSON_TAG_INSCATTERING_COLOR "InscatteringColor"

void ComponentFog::OnEditorUpdate() {
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
	ImGui::DragFloat("Density", &density, App->editor->dragSpeed5f, 0.0f, inf, "%.4f");
	ImGui::DragFloat("Falloff", &falloff, App->editor->dragSpeed5f, 0.0f, inf, "%.4f");
	ImGui::ColorEdit4("Inscattering Color", inscatteringColor.ptr(), ImGuiColorEditFlags_NoInputs);
}

void ComponentFog::Load(JsonValue jComponent) {
	density = jComponent[JSON_TAG_DENSITY];
	falloff = jComponent[JSON_TAG_FALLOFF];
	JsonValue jInscatteringColor = jComponent[JSON_TAG_INSCATTERING_COLOR];
	inscatteringColor = float4(jInscatteringColor[0], jInscatteringColor[1], jInscatteringColor[2], jInscatteringColor[3]);
}

void ComponentFog::Save(JsonValue jComponent) const {
	jComponent[JSON_TAG_DENSITY] = density;
	jComponent[JSON_TAG_FALLOFF] = falloff;
	JsonValue jInscatteringColor = jComponent[JSON_TAG_INSCATTERING_COLOR];
	jInscatteringColor[0] = inscatteringColor.x;
	jInscatteringColor[1] = inscatteringColor.y;
	jInscatteringColor[2] = inscatteringColor.z;
	jInscatteringColor[3] = inscatteringColor.w;
}

void ComponentFog::Draw() {
	ProgramHeightFog* heightFogProgram = App->programs->heightFog;
	if (heightFogProgram == nullptr) return;

	ComponentTransform* transform = GetOwner().GetComponent<ComponentTransform>();
	if (transform == nullptr) return;

	glUseProgram(heightFogProgram->program);

	float4x4 viewMatrix = App->camera->GetViewMatrix();
	float4x4 projectionMatrix = App->camera->GetProjectionMatrix();

	glUniformMatrix4fv(heightFogProgram->viewLocation, 1, GL_TRUE, viewMatrix.ptr());
	glUniformMatrix4fv(heightFogProgram->projLocation, 1, GL_TRUE, projectionMatrix.ptr());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, App->renderer->positionsMSTexture);
	glUniform1i(heightFogProgram->positionsLocation, 0);

	glUniform3fv(heightFogProgram->viewPosLocation, 1, App->camera->GetPosition().ptr());

	glUniform1f(heightFogProgram->densityLocation, density);
	glUniform1f(heightFogProgram->falloffLocation, falloff);
	glUniform1f(heightFogProgram->heightLocation, transform->GetGlobalPosition().y);
	glUniform4fv(heightFogProgram->inscatteringColorLocation, 1, inscatteringColor.ptr());

	glDrawArrays(GL_TRIANGLES, 0, 3);
}
