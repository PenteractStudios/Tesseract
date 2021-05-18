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
// clang-format off
static const float textureCords[12] = {
	// Front (x, y, z)
	0.0f,0.0f,
	1.0f,0.0f,
	0.0f,1.0f,
	//////////
	1.0f,0.0f,
	1.0f,1.0f,
	0.0f, 1.0f,
	};
// clang-format on
void ComponentTrail::Update() {
	ComponentTransform* transform = GetOwner().GetComponent<ComponentTransform>();

	if (isStarted) {
		previousPositionUp = currentPositionUp;
		previousPositionDown = currentPositionDown;
		previousPosition = currentPosition;

		currentPosition = transform->GetGlobalPosition();
		previousVectorUp = transform->GetGlobalRotation() * float3::unitY;
		previousVectorUp.Normalize();

		currentPositionUp = (previousVectorUp * width) + currentPosition;
		currentPositionDown = (-previousVectorUp * width) + currentPosition;
		if (trianglesCreated >= (maxVertices)) {
			UpdateVerticesPosition();
			trianglesCreated -= 30;
		}

		insertVertex(previousPositionDown);
		insertTextureCoords();
		insertVertex(currentPositionDown);
		insertTextureCoords();
		insertVertex(previousPositionUp);
		insertTextureCoords();

		insertVertex(currentPositionDown);
		insertTextureCoords();
		insertVertex(currentPositionUp);
		insertTextureCoords();
		insertVertex(previousPositionUp);
		insertTextureCoords();

		quadsCreated++;
		Draw();
	} else {
		isStarted = true;
		currentPosition = transform->GetGlobalPosition();
		currentPositionUp = transform->GetGlobalRotation() * float3::unitY;
		currentPositionUp.Normalize();
		currentPositionUp = currentPositionUp * width + currentPosition;
		currentPositionDown = -currentPositionUp * width + currentPosition;
	}
}

void ComponentTrail::Init() {
}

void ComponentTrail::DrawGizmos() {
}

void ComponentTrail::OnEditorUpdate() {
	ImGui::ResourceSlot<ResourceShader>("shader", &shaderID);
	ImGui::InputFloat("Witdh: ", &width);
	ImGui::ColorEdit4("InitColor##", initC.ptr());

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
				transform2D->SetSize(float2((float) width, (float) height));
			}
		}
		ImGui::Text("");
		ImGui::Separator();
		ImGui::TextColored(App->editor->titleColor, "Texture Preview");
		ImGui::TextWrapped("Size:");
		ImGui::SameLine();
		ImGui::TextWrapped("%d x %d", width, height);
		ImGui::Image((void*) textureResource->glTexture, ImVec2(200, 200));
		ImGui::Separator();
	}
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
	jComponent[JSON_TAG_TEXTURE_SHADERID] = shaderID;
	jComponent[JSON_TAG_TEXTURE_TEXTUREID] = textureID;
}

void ComponentTrail::Draw() {
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
	glDisable(GL_CULL_FACE);

	unsigned int quadVBO;
	glGenBuffers(1, &quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO); // set vbo active
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesPosition), verticesPosition, GL_STATIC_DRAW);

	/*glBindBuffer(GL_ARRAY_BUFFER, verticesPosition);*/
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*) 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*) (sizeof(float) * 3));
	glUseProgram(program);
	//TODO ADD DELTATIME

	ComponentTransform* transform = GetOwner().GetComponent<ComponentTransform>();

	Frustum* frustum = App->camera->GetActiveCamera()->GetFrustum();
	float4x4* proj = &App->camera->GetProjectionMatrix();
	float4x4* view = &App->camera->GetViewMatrix();

	float4x4 newModelMatrix = transform->GetGlobalMatrix().LookAt(transform->GetGlobalMatrix().RotatePart().Col(2), -frustum->Front(), transform->GetGlobalMatrix().RotatePart().Col(1), float3::unitY);
	float4x4 Final = float4x4::FromTRS(transform->GetGlobalPosition(), transform->GetGlobalMatrix().RotatePart(), transform->GetGlobalScale());

	//-> glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_TRUE, newModelMatrix.ptr());

	glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_TRUE, transform->GetGlobalMatrix().ptr());
	glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_TRUE, view->ptr());
	glUniformMatrix4fv(glGetUniformLocation(program, "proj"), 1, GL_TRUE, proj->ptr());

	glActiveTexture(GL_TEXTURE0);
	//glUniform1i(glGetUniformLocation(program, "diffuse"), 0);
	//glUniform1f(glGetUniformLocation(program, "currentFrame"), currentParticle.currentFrame);
	//glUniform1f(glGetUniformLocation(program, "colorFrame"), currentParticle.colorFrame);
	glUniform4fv(glGetUniformLocation(program, "inputColor"), 1, initC.ptr());
	//glUniform4fv(glGetUniformLocation(program, "finalColor"), 1, finalC.ptr());

	ResourceTexture* textureResource = App->resources->GetResource<ResourceTexture>(textureID);
	if (textureResource != nullptr) {
		glBindTexture(GL_TEXTURE_2D, textureResource->glTexture);
	}

	glDrawArrays(GL_TRIANGLES, 0, trianglesCreated);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}

void ComponentTrail::SpawnParticle() {
}

void ComponentTrail::UpdateVerticesPosition() {
	for (size_t i = 0; i < (maxVertices - 30); i++) {
		verticesPosition[i] = verticesPosition[i + 30];
	}
}
void ComponentTrail::insertVertex(float3 vertex) {
	verticesPosition[trianglesCreated++] = vertex.x;
	verticesPosition[trianglesCreated++] = vertex.y;
	verticesPosition[trianglesCreated++] = vertex.z;
}

void ComponentTrail::insertTextureCoords() {
	if (textureCreated == 12) textureCreated = 0;

	verticesPosition[trianglesCreated++] = textureCords[textureCreated++];
	verticesPosition[trianglesCreated++] = textureCords[textureCreated++];
}