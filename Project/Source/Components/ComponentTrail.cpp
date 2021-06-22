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
#include "Utils/ImGuiUtils.h"
#include "Utils/Logging.h"

#include "Math/float3x3.h"
#include "Math/TransformOps.h"
#include "imgui.h"
#include "GL/glew.h"
#include "debugdraw.h"
#include "rapidjson/rapidjson.h"

#include "Utils/Leaks.h"
#include <string>

#define JSON_TAG_TEXTURE_TEXTUREID "TextureId"
#define JSON_TAG_MAXVERTICES "MaxVertices"

#define JSON_TAG_WIDTH "Width"
#define JSON_TAG_TRAILQUADS "TrailQuads"
#define JSON_TAG_NREPEATS "TextureRepeats"

#define JSON_TAG_COLOR_OVER_TRAIL "ColorOverTrail"
#define JSON_TAG_GRADIENT_COLOR "GradientColor"
#define JSON_TAG_NUMBER_COLORS "NumColors"
#define JSON_TAG_COLOR_LIFE "ColorLife"

void ComponentTrail::Init() {
	glGenBuffers(1, &quadVBO);
	EditTextureCoords();
}

void ComponentTrail::Update() {
	ComponentTransform* transform = GetOwner().GetComponent<ComponentTransform>();
	float3 vectorUp = (transform->GetGlobalRotation() * float3::unitY).Normalized();

	if (isStarted) {
		previousPositionUp = currentPositionUp;
		previousPositionDown = currentPositionDown;
		previousPosition = currentPosition;

		currentPosition = transform->GetGlobalPosition();
		currentPositionUp = (vectorUp * width) + currentPosition;
		currentPositionDown = (-vectorUp * width) + currentPosition;

		float3x3 modelMatrix = float3x3::identity;

		if (trianglesCreated >= maxVertices) {
			UpdateVerticesPosition();
		}

		InsertVertex(modelMatrix * previousPositionDown);
		InsertTextureCoords();
		InsertVertex(modelMatrix * currentPositionDown);
		InsertTextureCoords();
		InsertVertex(modelMatrix * previousPositionUp);
		InsertTextureCoords();

		InsertVertex(modelMatrix * currentPositionDown);
		InsertTextureCoords();
		InsertVertex(modelMatrix * currentPositionUp);
		InsertTextureCoords();
		InsertVertex(modelMatrix * previousPositionUp);
		InsertTextureCoords();

		quadsCreated++;

		// Upate Trail Time
		if (App->time->IsGameRunning()) {
			trailTime += App->time->GetDeltaTime();
		} else {
			trailTime += App->time->GetRealTimeDeltaTime();
		}

	} else {
		isStarted = true;
		currentPosition = transform->GetGlobalPosition();
		currentPositionUp = vectorUp * width + currentPosition;
		currentPositionDown = -vectorUp * width + currentPosition;
	}
}

void ComponentTrail::OnEditorUpdate() {
	ImGui::DragFloat("Witdh", &width, App->editor->dragSpeed2f, 0, inf);
	if (ImGui::DragInt("Trail Quads", &trailQuads, 1.0f, 1, 50, "%d", ImGuiSliderFlags_AlwaysClamp)) {
		if (nTextures > trailQuads) nTextures = trailQuads;
		DeleteQuads();
	}
	if (ImGui::DragInt("Texture Repeats", &nTextures, 1.0f, 1, trailQuads, "%d", ImGuiSliderFlags_AlwaysClamp)) {
		DeleteQuads();
		EditTextureCoords();
	}
	ImGui::Checkbox("Color Over Trail", &colorOverTrail);
	if (colorOverTrail) {
		ImGui::DragFloat("Color Life", &colorLife, App->editor->dragSpeed2f, 0, inf);
		ImGui::GradientEditor(&gradient, draggingGradient, selectedGradient);
		if (ImGui::Button("Reset Color")) {
			ResetColor();
		}
	}

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
		ImGui::NewLine();
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
	textureID = jComponent[JSON_TAG_TEXTURE_TEXTUREID];
	if (textureID != 0) {
		App->resources->IncreaseReferenceCount(textureID);
	}
	maxVertices = jComponent[JSON_TAG_MAXVERTICES];
	trailQuads = jComponent[JSON_TAG_TRAILQUADS];

	width = jComponent[JSON_TAG_WIDTH];
	nRepeats = jComponent[JSON_TAG_NREPEATS];

	colorOverTrail = jComponent[JSON_TAG_COLOR_OVER_TRAIL];
	int numberColors = jComponent[JSON_TAG_NUMBER_COLORS];
	gradient.clearList();
	JsonValue jColor = jComponent[JSON_TAG_GRADIENT_COLOR];
	for (int i = 0; i < numberColors; ++i) {
		JsonValue jMark = jColor[i];
		gradient.addMark(jMark[4], ImColor((float) jMark[0], (float) jMark[1], (float) jMark[2], (float) jMark[3]));
	}
	colorLife = jComponent[JSON_TAG_COLOR_LIFE];
}

void ComponentTrail::Save(JsonValue jComponent) const {
	jComponent[JSON_TAG_TEXTURE_TEXTUREID] = textureID;
	jComponent[JSON_TAG_MAXVERTICES] = maxVertices;
	jComponent[JSON_TAG_TRAILQUADS] = trailQuads;

	jComponent[JSON_TAG_WIDTH] = width;
	jComponent[JSON_TAG_NREPEATS] = nRepeats;

	// Color
	jComponent[JSON_TAG_COLOR_OVER_TRAIL] = colorOverTrail;
	int color = 0;
	JsonValue jColor = jComponent[JSON_TAG_GRADIENT_COLOR];
	for (ImGradientMark* mark : gradient.getMarks()) {
		JsonValue jMask = jColor[color];
		jMask[0] = mark->color[0];
		jMask[1] = mark->color[1];
		jMask[2] = mark->color[2];
		jMask[3] = mark->color[3];
		jMask[4] = mark->position;

		color++;
	}
	jComponent[JSON_TAG_NUMBER_COLORS] = gradient.getMarks().size();
	jComponent[JSON_TAG_COLOR_LIFE] = colorLife;
}

void ComponentTrail::Draw() {
	unsigned int program = App->programs->trail;
	glUseProgram(program);

	unsigned glTexture = 0;
	ResourceTexture* texture = App->resources->GetResource<ResourceTexture>(textureID);
	glTexture = texture ? texture->glTexture : 0;
	int hasDiffuseMap = texture ? 1 : 0;

	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_CULL_FACE);

	glBindBuffer(GL_ARRAY_BUFFER, quadVBO); // set vbo active
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesPosition), verticesPosition, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*) 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*) (sizeof(float) * 3));

	float4x4* proj = &App->camera->GetProjectionMatrix();
	float4x4* view = &App->camera->GetViewMatrix();

	glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_TRUE, view->ptr());
	glUniformMatrix4fv(glGetUniformLocation(program, "proj"), 1, GL_TRUE, proj->ptr());

	float4 color = float4::one;
	if (colorOverTrail) {
		float factor = trailTime / colorLife;
		gradient.getColorAt(factor, color.ptr());
	}

	glUniform1i(glGetUniformLocation(program, "diffuseMap"), 0);
	glUniform1i(glGetUniformLocation(program, "hasDiffuse"), hasDiffuseMap);
	glUniform4fv(glGetUniformLocation(program, "inputColor"), 1, color.ptr());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, glTexture);

	glDrawArrays(GL_TRIANGLES, 0, trianglesCreated);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}

void ComponentTrail::UpdateVerticesPosition() {
	for (int i = 0; i < maxVertices - 30; i++) {
		verticesPosition[i] = verticesPosition[i + 30];
	}
	trianglesCreated -= 30;
}

void ComponentTrail::InsertVertex(float3 vertex) {
	verticesPosition[trianglesCreated++] = vertex.x;
	verticesPosition[trianglesCreated++] = vertex.y;
	verticesPosition[trianglesCreated++] = vertex.z;
}

void ComponentTrail::InsertTextureCoords() {
	if (nTextures == 1) {
		textureCreated = 0;
		for (int i = 0; i < trianglesCreated + 2;) {
			verticesPosition[(i + 3)] = textureCords[textureCreated++];
			verticesPosition[(i + 4)] = textureCords[textureCreated++];
			i += 5;
		}
		trianglesCreated += 2;
	} else {
		if (textureCreated == nRepeats) textureCreated = 0;

		verticesPosition[trianglesCreated++] = textureCords[textureCreated++];
		verticesPosition[trianglesCreated++] = textureCords[textureCreated++];
	}
}

void ComponentTrail::DeleteQuads() {
	for (int i = 0; i < maxVertices; i++) {
		verticesPosition[i] = 0.0f;
	}
	isStarted = false;
	quadsCreated = 0;
	trianglesCreated = 0;
	maxVertices = 30 * trailQuads;
	textureCreated = 0;
}

void ComponentTrail::EditTextureCoords() {
	int nLine = 0;
	float factor = nTextures / trailQuads;
	nRepeats = (trailQuads / nTextures) * 12;
	for (int textureEdited = 0; textureEdited < nRepeats;) {
		///vertice1
		textureCords[textureEdited++] = (nLine) *factor;
		textureCords[textureEdited++] = 0.0f;

		textureCords[textureEdited++] = (nLine + 1) * factor;
		textureCords[textureEdited++] = 0.0f;

		textureCords[textureEdited++] = (nLine) *factor;
		textureCords[textureEdited++] = 1.0f;

		///vertice2
		textureCords[textureEdited++] = (nLine + 1) * factor;
		textureCords[textureEdited++] = 0.0f;

		textureCords[textureEdited++] = (nLine + 1) * factor;
		textureCords[textureEdited++] = 1.0f;

		textureCords[textureEdited++] = (nLine) *factor;
		textureCords[textureEdited++] = 1.0f;

		nLine++;
	}
}

void ComponentTrail::ResetColor() {
	trailTime = 0.0f;
}