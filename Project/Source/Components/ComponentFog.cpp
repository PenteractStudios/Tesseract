#include "ComponentFog.h"

#include "Application.h"
#include "GameObject.h"
#include "Modules/ModulePrograms.h"
#include "Modules/ModuleRender.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleCamera.h"

#include "GL/glew.h"
#include "imgui.h"

#define JSON_TAG_FALLOFF "Falloff"

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
	ImGui::DragFloat("Falloff", &falloff, App->editor->dragSpeed5f, 0.0f, inf, "%.4f");
}

void ComponentFog::Load(JsonValue jComponent) {
	falloff = jComponent[JSON_TAG_FALLOFF];
}

void ComponentFog::Save(JsonValue jComponent) const {
	jComponent[JSON_TAG_FALLOFF] = falloff;
}

void ComponentFog::Draw() {
	ProgramHeightFog* heightFogProgram = App->programs->heightFog;
	if (heightFogProgram == nullptr) return;

	glUseProgram(heightFogProgram->program);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, App->renderer->renderTexture);
	glUniform1i(heightFogProgram->originalRenderLocation, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, App->renderer->positionsMSTexture);
	glUniform1i(heightFogProgram->positionsLocation, 1);

	glUniform3fv(heightFogProgram->viewPosLocation, 1, App->camera->GetPosition().ptr());

	glUniform1f(heightFogProgram->falloffLocation, falloff);

	glDrawArrays(GL_TRIANGLES, 0, 3);
}
