#include "ComponentTrail.h"

#include "Application.h"
#include "GameObject.h"
#include "Modules/ModulePrograms.h"
#include "Modules/ModuleCamera.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModuleTime.h"
#include "Components/ComponentTransform.h"
#include "Resources/ResourceTexture.h"
#include "Resources/ResourceShader.h"
#include "FileSystem/JsonValue.h"
#include "Utils/ImGuiUtils.h"

#include "GL/glew.h"
#include "imgui_color_gradient.h"
#include <string>

#include "Utils/Leaks.h"

#define JSON_TAG_TEXTURE_TEXTUREID "TextureId"
#define JSON_TAG_MAXVERTICES "MaxVertices"

#define JSON_TAG_WIDTH "Width"
#define JSON_TAG_TRAIL_QUADS "TrailQuads"
#define JSON_TAG_TEXTURE_REPEATS "TextureRepeats"
#define JSON_TAG_QUAD_LIFE "QuadLife"
#define JSON_TAG_IS_RENDERING "IsRendering"

#define JSON_TAG_HAS_COLOR_OVER_TRAIL "HasColorOverTrail"
#define JSON_TAG_GRADIENT_COLOR "GradientColor"
#define JSON_TAG_NUMBER_COLORS "NumColors"

ComponentTrail::~ComponentTrail() {
	RELEASE(trail);
}

void ComponentTrail::Init() {
	if (!trail) trail = new Trail();
	trail->Init();
	trail->mainPosition = &GetOwner().GetComponent<ComponentTransform>()->GetPosition();
}

void ComponentTrail::Update() {
	trail->Update(GetOwner().GetComponent<ComponentTransform>()->GetPosition());
	trail->mainPosition = &GetOwner().GetComponent<ComponentTransform>()->GetPosition();
}

void ComponentTrail::OnEditorUpdate() {
	if (ImGui::Button("Play")) Play();
	if (ImGui::Button("Stop")) Stop();
	if (ImGui::Checkbox("Render", &trail->isRendering)) {
		trail->isStarted = false;
		trail->DeleteQuads();
	}
	ImGui::DragFloat("Witdh", &trail->width, App->editor->dragSpeed2f, 0, inf);
	if (ImGui::DragInt("Trail Quads", &trail->trailQuads, 1.0f, 1, trail->maxQuads, "%d", ImGuiSliderFlags_AlwaysClamp)) {
		if (trail->nTextures > trail->trailQuads) trail->nTextures = trail->trailQuads;
		trail->DeleteQuads();
	}
	if (ImGui::DragInt("Texture Repeats", &trail->nTextures, 1.0f, 1, trail->trailQuads, "%d", ImGuiSliderFlags_AlwaysClamp)) {
		trail->DeleteQuads();
		trail->EditTextureCoords();
	}

	ImGui::DragFloat("Quad Life", &trail->quadLife, App->editor->dragSpeed2f, 1, inf);

	ImGui::Checkbox("Color Over Trail", &trail->colorOverTrail);
	if (trail->colorOverTrail) {
		ImGui::GradientEditor(trail->gradient, trail->draggingGradient, trail->selectedGradient);
	}

	UID oldID = trail->textureID;
	ImGui::ResourceSlot<ResourceTexture>("texture", &trail->textureID);
	ResourceTexture* textureResource = App->resources->GetResource<ResourceTexture>(trail->textureID);
	if (textureResource != nullptr) {
		int width;
		int height;
		glGetTextureLevelParameteriv(textureResource->glTexture, 0, GL_TEXTURE_WIDTH, &width);
		glGetTextureLevelParameteriv(textureResource->glTexture, 0, GL_TEXTURE_HEIGHT, &height);

		if (oldID != trail->textureID) {
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
	if (!trail) trail = new Trail();
	trail->textureID = jComponent[JSON_TAG_TEXTURE_TEXTUREID];
	if (trail->textureID != 0) {
		App->resources->IncreaseReferenceCount(trail->textureID);
	}
	trail->maxVertices = jComponent[JSON_TAG_MAXVERTICES];
	trail->trailQuads = jComponent[JSON_TAG_TRAIL_QUADS];
	trail->quadLife = jComponent[JSON_TAG_QUAD_LIFE];
	trail->isRendering = jComponent[JSON_TAG_IS_RENDERING];
	trail->width = jComponent[JSON_TAG_WIDTH];
	trail->nTextures = jComponent[JSON_TAG_TEXTURE_REPEATS];

	trail->colorOverTrail = jComponent[JSON_TAG_HAS_COLOR_OVER_TRAIL];
	int numberColors = jComponent[JSON_TAG_NUMBER_COLORS];
	if (!trail->gradient) trail->gradient = new ImGradient();
	trail->gradient->clearList();
	JsonValue jColor = jComponent[JSON_TAG_GRADIENT_COLOR];
	for (int i = 0; i < numberColors; ++i) {
		JsonValue jMark = jColor[i];
		trail->gradient->addMark(jMark[4], ImColor((float) jMark[0], (float) jMark[1], (float) jMark[2], (float) jMark[3]));
	}
}

void ComponentTrail::Save(JsonValue jComponent) const {
	jComponent[JSON_TAG_TEXTURE_TEXTUREID] = trail->textureID;
	jComponent[JSON_TAG_MAXVERTICES] = trail->maxVertices;
	jComponent[JSON_TAG_TRAIL_QUADS] = trail->trailQuads;

	jComponent[JSON_TAG_WIDTH] = trail->width;
	jComponent[JSON_TAG_TEXTURE_REPEATS] = trail->nTextures;
	jComponent[JSON_TAG_QUAD_LIFE] = trail->quadLife;
	jComponent[JSON_TAG_IS_RENDERING] = trail->isRendering;
	// Color
	jComponent[JSON_TAG_HAS_COLOR_OVER_TRAIL] = trail->colorOverTrail;
	int color = 0;
	JsonValue jColor = jComponent[JSON_TAG_GRADIENT_COLOR];
	for (ImGradientMark* mark : trail->gradient->getMarks()) {
		JsonValue jMask = jColor[color];
		jMask[0] = mark->color[0];
		jMask[1] = mark->color[1];
		jMask[2] = mark->color[2];
		jMask[3] = mark->color[3];
		jMask[4] = mark->position;

		color++;
	}
	jComponent[JSON_TAG_NUMBER_COLORS] = trail->gradient->getMarks().size();
}
void ComponentTrail::Draw() {
	trail->Draw();
}

TESSERACT_ENGINE_API void ComponentTrail::Play() {
	trail->isRendering = true;
}

TESSERACT_ENGINE_API void ComponentTrail::Stop() {
	trail->isRendering = false;
	trail->isStarted = false;
	trail->DeleteQuads();
}

TESSERACT_ENGINE_API void ComponentTrail::SetWidth(float w) {
	trail->width = w;
}