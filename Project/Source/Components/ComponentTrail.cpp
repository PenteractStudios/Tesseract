#include "ComponentTrail.h"

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
#include <Utils/Logging.h>

#define JSON_TAG_TEXTURE_SHADERID "ShaderId"
#define JSON_TAG_TEXTURE_TEXTUREID "TextureId"
#define JSON_TAG_COLOR "Color"

#define JSON_TAG_ALPHATRANSPARENCY "AlphaTransparency"

void ComponentTrail::Update() {
	ComponentTransform* transform = GetOwner().GetComponent<ComponentTransform>();

	if (isStarted) {
		previousPositionUp = currentPositionUp;
		previousPositionDown = currentPositionDown;

		previousPosition = currentPosition;

		currentPosition = transform->GetGlobalPosition();
		currentPositionUp = transform->GetGlobalRotation() * float3::unitX;
		currentPositionUp.Normalize();
		currentPositionUp = currentPositionUp * width + currentPosition;
		currentPositionDown = -currentPositionUp * width + currentPosition;
		if (trianglesCreated >= (maxVertices)) {
			UpdateVerticesPosition();
			trianglesCreated -= 18;
		}
		insertVertex(previousPositionDown);
		insertVertex(currentPositionDown);
		insertVertex(previousPositionUp);

		insertVertex(currentPositionDown);
		insertVertex(currentPositionUp);
		insertVertex(previousPositionUp);

		Draw();
	} else {
		isStarted = true;
		currentPosition = transform->GetGlobalPosition();
		currentPositionUp = transform->GetGlobalRotation() * float3::unitX;
		currentPositionUp.Normalize();
		currentPositionUp = currentPositionUp * width + currentPosition;
		currentPositionDown = -currentPositionUp * width + currentPosition;
	}
}

void ComponentTrail::Init() {}

void ComponentTrail::DrawGizmos() {
}

void ComponentTrail::OnEditorUpdate() {
	ImGui::ResourceSlot<ResourceShader>("shader", &shaderID);
	ImGui::InputFloat("Witdh: ", &width);
	ImGui::ColorEdit4("InitColor##", initC.ptr());

	UID oldID = textureID;
	ImGui::ResourceSlot<ResourceTexture>("texture", &textureID);
	ResourceTexture* textureResource = App->resources->GetResource<ResourceTexture>(textureID);
}

void ComponentTrail::Load(JsonValue jComponent) {
	shaderID = jComponent[JSON_TAG_TEXTURE_SHADERID];

	if (shaderID != 0) {
		App->resources->IncreaseReferenceCount(shaderID);
	}

	textureID = jComponent[JSON_TAG_TEXTURE_TEXTUREID];

	if (textureID != 0) {
		App->resources->IncreaseReferenceCount(textureID);
	}
}

void ComponentTrail::Save(JsonValue jComponent) const {
}

void ComponentTrail::Draw() {
	unsigned int program = 0;
	ResourceShader* shaderResouce = App->resources->GetResource<ResourceShader>(shaderID);
	if (shaderResouce) {
		program = shaderResouce->GetShaderProgram();
	} else {
		return;
	}

	glDisable(GL_CULL_FACE);
	//glDisable(GL_DEPTH_TEST);
	//glEnable(GL_BLEND);
	//glBlendEquation(GL_MAX);
	//glBlendFunc(GL_ONE, GL_ONE);

	unsigned int quadVBO;
	glGenBuffers(1, &quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO); // set vbo active
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesPosition), verticesPosition, GL_STATIC_DRAW);

	/*glBindBuffer(GL_ARRAY_BUFFER, verticesPosition);*/
	glEnableVertexAttribArray(0);
	//glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
	//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*) (sizeof(float) * 6 * 3));
	glUseProgram(program);
	//TODO ADD DELTATIME

	ComponentTransform* transform = GetOwner().GetComponent<ComponentTransform>();

	Frustum* frustum = App->camera->GetActiveCamera()->GetFrustum();
	float4x4* proj = &App->camera->GetProjectionMatrix();
	float4x4* view = &App->camera->GetViewMatrix();

	//float4x4 newModelMatrix = transform->GetGlobalMatrix().LookAt(transform->GetGlobalMatrix().RotatePart().Col(2), -frustum->Front(), transform->GetGlobalMatrix().RotatePart().Col(1), float3::unitY);
	//float4x4 Final = float4x4::FromTRS(transform->GetGlobalPosition(), transform->GetGlobalMatrix().RotatePart(), transform->GetScale());

	//-> glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_TRUE, newModelMatrix.ptr());

	//glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_TRUE, Final.ptr());
	glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_TRUE, view->ptr());
	glUniformMatrix4fv(glGetUniformLocation(program, "proj"), 1, GL_TRUE, proj->ptr());

	glActiveTexture(GL_TEXTURE0);
	//glUniform1i(glGetUniformLocation(program, "diffuse"), 0);
	//glUniform1f(glGetUniformLocation(program, "currentFrame"), currentParticle.currentFrame);
	//glUniform1f(glGetUniformLocation(program, "colorFrame"), currentParticle.colorFrame);
	glUniform4fv(glGetUniformLocation(program, "inputColor"), 1, initC.ptr());
	//glUniform4fv(glGetUniformLocation(program, "finalColor"), 1, finalC.ptr());

	/*ResourceTexture* textureResource = App->resources->GetResource<ResourceTexture>(textureID);
	if (textureResource != nullptr) {
		glBindTexture(GL_TEXTURE_2D, textureResource->glTexture);
	}*/

	glDrawArrays(GL_TRIANGLES, 0, trianglesCreated);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	//glEnable(GL_DEPTH_TEST);
}

void ComponentTrail::DuplicateComponent(GameObject& owner) {
}

void ComponentTrail::SpawnParticle() {
}

void ComponentTrail::UpdateVerticesPosition() {
	for (size_t i = 0; i < (maxVertices - 18); i++) {
		verticesPosition[i] = verticesPosition[i + 18];
	}
}
void ComponentTrail::insertVertex(float3 vertex) {
	verticesPosition[trianglesCreated++] = vertex.x;
	verticesPosition[trianglesCreated++] = vertex.y;
	verticesPosition[trianglesCreated++] = vertex.z;
}