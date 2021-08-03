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
	trail->OnEditorUpdate();
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