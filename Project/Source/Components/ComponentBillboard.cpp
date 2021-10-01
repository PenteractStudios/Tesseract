#include "ComponentBillboard.h"

#include "Application.h"
#include "GameObject.h"
#include "Modules/ModulePrograms.h"
#include "Modules/ModuleCamera.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModuleTime.h"
#include "Modules/ModuleUserInterface.h"
#include "Components/ComponentTransform.h"
#include "Components/UI/ComponentTransform2D.h"
#include "Resources/ResourceTexture.h"
#include "Resources/ResourceShader.h"
#include "FileSystem/JsonValue.h"
#include "Math/float3x3.h"
#include "Utils/ImGuiUtils.h"

#include "Math/TransformOps.h"
#include "imgui_color_gradient.h"
#include "GL/glew.h"

#include "Utils/Leaks.h"
#include <cmath>

#define JSON_TAG_PLAY_ON_AWAKE "PlayOnAwake"
#define JSON_TAG_BILLBOARD_LIFETIME "BillboardLifetime"

#define JSON_TAG_BILLBOARD_TYPE "BillboardType"
#define JSON_TAG_IS_HORIZONTAL_ORIENTATION "IsHorizontalOrientation"
#define JSON_TAG_RENDER_MODE "RenderMode"
#define JSON_TAG_TEXTURE_TEXTUREID "TextureId"
#define JSON_TAG_TEXTURE_INTENSITY "TextureIntensity"
#define JSON_TAG_FLIP_TEXTURE "FlipTexture"

#define JSON_TAG_YTILES "Ytiles"
#define JSON_TAG_XTILES "Xtiles"
#define JSON_TAG_ANIMATION_CYCLES "AnimationCycles"
#define JSON_TAG_ANIMATION_LOOP "AnimationLoop"

#define JSON_TAG_COLOR_OVER_LIFETIME "ColorOverLifetime"
#define JSON_TAG_COLOR_CYCLES "ColorCycles"
#define JSON_TAG_COLOR_LOOP "ColorLoop"
#define JSON_TAG_NUMBER_COLORS "NumberColors"
#define JSON_TAG_GRADIENT_COLORS "GradientColors"

#define ITEM_SIZE 150

ComponentBillboard::~ComponentBillboard() {
	RELEASE(gradient);
}

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
	ImGui::Indent();
	ImGui::PushItemWidth(ITEM_SIZE);

	if (ImGui::Button("Play")) {
		Play();
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop")) {
		Stop();
	}
	ImGui::DragFloat("Duration", &billboardLifetime, App->editor->dragSpeed2f, 0, inf);
	ImGui::Checkbox("Play On Awake", &playOnAwake);

	// Render
	if (ImGui::CollapsingHeader("Render")) {
		const char* billboardTypeCombo[] = {"LookAt", "Stretch", "Horitzontal", "Vertical"};
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

		if (billboardType == BillboardType::HORIZONTAL) {
			ImGui::Indent();
			ImGui::Checkbox("Orientate to direction", &isHorizontalOrientation);
			ImGui::Unindent();
		}
		ImGui::NewLine();

		const char* renderModeCombo[] = {"Additive", "Transparent"};
		const char* renderModeComboCurrent = renderModeCombo[(int) renderMode];
		if (ImGui::BeginCombo("Render Mode##", renderModeComboCurrent)) {
			for (int n = 0; n < IM_ARRAYSIZE(renderModeCombo); ++n) {
				bool isSelected = (renderModeComboCurrent == renderModeCombo[n]);
				if (ImGui::Selectable(renderModeCombo[n], isSelected)) {
					renderMode = (ParticleRenderMode) n;
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		ImGui::NewLine();

		ImGui::ResourceSlot<ResourceTexture>("texture", &textureID);
		if (textureID) {
			ImGui::Indent();
			ImGui::DragFloat3("Intensity (RGB)", textureIntensity.ptr(), App->editor->dragSpeed2f, 0.0f, inf);
			ImGui::Unindent();
		}

		ImGui::NewLine();
		ImGui::Text("Flip: ");
		ImGui::SameLine();
		ImGui::Checkbox("X", &flipTexture[0]);
		ImGui::SameLine();
		ImGui::Checkbox("Y", &flipTexture[1]);
	}

	// Texture Sheet Animation
	if (ImGui::CollapsingHeader("Texture Sheet Animation")) {
		ImGui::DragScalar("Xtiles", ImGuiDataType_U32, &Xtiles);
		ImGui::DragScalar("Ytiles", ImGuiDataType_U32, &Ytiles);
		ImGui::DragFloat("Cycles##animation_cycles", &animationCycles, App->editor->dragSpeed2f, 1, inf);
		ImGui::Checkbox("Loop##animation_loop", &animationLoop);
	}

	// Color Over Lifetime
	if (ImGui::CollapsingHeader("Color over lifetime")) {
		ImGui::Checkbox("##color_over_lifetime", &colorOverLifetime);
		if (colorOverLifetime) {
			ImGui::SameLine();
			ImGui::PushItemWidth(200);
			ImGui::GradientEditor(gradient, draggingGradient, selectedGradient);
			ImGui::PushItemWidth(ITEM_SIZE);
			ImGui::NewLine();
			ImGui::DragFloat("Cycles##color_cycles", &colorCycles, App->editor->dragSpeed2f, 1, inf);
			ImGui::Checkbox("Loop##color_loop", &colorLoop);
		}
	}

	// Texture Preview
	if (ImGui::CollapsingHeader("Texture Preview")) {
		ResourceTexture* textureResource = App->resources->GetResource<ResourceTexture>(textureID);
		if (textureResource != nullptr) {
			int width;
			int height;
			glGetTextureLevelParameteriv(textureResource->glTexture, 0, GL_TEXTURE_WIDTH, &width);
			glGetTextureLevelParameteriv(textureResource->glTexture, 0, GL_TEXTURE_HEIGHT, &height);
			ImGui::TextColored(App->editor->titleColor, "Billboard Texture");
			ImGui::TextWrapped("Size:");
			ImGui::SameLine();
			ImGui::TextWrapped("%i x %i", width, height);
			ImGui::Image((void*) textureResource->glTexture, ImVec2(200, 200));
			ImGui::NewLine();
		}
	}

	ImGui::Unindent();
	ImGui::PopItemWidth();
	ImGui::NewLine();
}

void ComponentBillboard::Load(JsonValue jComponent) {
	playOnAwake = jComponent[JSON_TAG_PLAY_ON_AWAKE];
	billboardLifetime = jComponent[JSON_TAG_BILLBOARD_LIFETIME];

	billboardType = (BillboardType)(int) jComponent[JSON_TAG_BILLBOARD_TYPE];
	isHorizontalOrientation = jComponent[JSON_TAG_IS_HORIZONTAL_ORIENTATION];
	renderMode = (ParticleRenderMode)(int) jComponent[JSON_TAG_RENDER_MODE];
	textureID = jComponent[JSON_TAG_TEXTURE_TEXTUREID];
	if (textureID != 0) {
		App->resources->IncreaseReferenceCount(textureID);
	}
	JsonValue jTextureIntensity = jComponent[JSON_TAG_TEXTURE_INTENSITY];
	textureIntensity[0] = jTextureIntensity[0];
	textureIntensity[1] = jTextureIntensity[1];
	textureIntensity[2] = jTextureIntensity[2];
	JsonValue jFlip = jComponent[JSON_TAG_FLIP_TEXTURE];
	flipTexture[0] = jFlip[0];
	flipTexture[1] = jFlip[1];

	Ytiles = jComponent[JSON_TAG_YTILES];
	Xtiles = jComponent[JSON_TAG_XTILES];
	animationCycles = jComponent[JSON_TAG_ANIMATION_CYCLES];
	animationLoop = jComponent[JSON_TAG_ANIMATION_LOOP];

	colorOverLifetime = jComponent[JSON_TAG_COLOR_OVER_LIFETIME];
	colorCycles = jComponent[JSON_TAG_COLOR_CYCLES];
	colorLoop = jComponent[JSON_TAG_COLOR_LOOP];
	int numberColors = jComponent[JSON_TAG_NUMBER_COLORS];
	if (!gradient) gradient = new ImGradient();
	gradient->clearList();
	JsonValue jColor = jComponent[JSON_TAG_GRADIENT_COLORS];
	for (int i = 0; i < numberColors; ++i) {
		JsonValue jMark = jColor[i];
		gradient->addMark(jMark[4], ImColor((float) jMark[0], (float) jMark[1], (float) jMark[2], (float) jMark[3]));
	}
}

void ComponentBillboard::Save(JsonValue jComponent) const {
	jComponent[JSON_TAG_PLAY_ON_AWAKE] = playOnAwake;
	jComponent[JSON_TAG_BILLBOARD_LIFETIME] = billboardLifetime;

	jComponent[JSON_TAG_BILLBOARD_TYPE] = (int) billboardType;
	jComponent[JSON_TAG_IS_HORIZONTAL_ORIENTATION] = isHorizontalOrientation;
	jComponent[JSON_TAG_RENDER_MODE] = (int) renderMode;
	jComponent[JSON_TAG_TEXTURE_TEXTUREID] = textureID;
	JsonValue jTextureIntensity = jComponent[JSON_TAG_TEXTURE_INTENSITY];
	jTextureIntensity[0] = textureIntensity[0];
	jTextureIntensity[1] = textureIntensity[1];
	jTextureIntensity[2] = textureIntensity[2];
	JsonValue jFlip = jComponent[JSON_TAG_FLIP_TEXTURE];
	jFlip[0] = flipTexture[0];
	jFlip[1] = flipTexture[1];

	jComponent[JSON_TAG_YTILES] = Ytiles;
	jComponent[JSON_TAG_XTILES] = Xtiles;
	jComponent[JSON_TAG_ANIMATION_CYCLES] = animationCycles;
	jComponent[JSON_TAG_ANIMATION_LOOP] = animationLoop;

	jComponent[JSON_TAG_COLOR_OVER_LIFETIME] = colorOverLifetime;
	jComponent[JSON_TAG_COLOR_CYCLES] = colorCycles;
	jComponent[JSON_TAG_COLOR_LOOP] = colorLoop;
	int color = 0;
	JsonValue jColor = jComponent[JSON_TAG_GRADIENT_COLORS];
	for (ImGradientMark* mark : gradient->getMarks()) {
		JsonValue jMask = jColor[color];
		jMask[0] = mark->color[0];
		jMask[1] = mark->color[1];
		jMask[2] = mark->color[2];
		jMask[3] = mark->color[3];
		jMask[4] = mark->position;

		color++;
	}
	jComponent[JSON_TAG_NUMBER_COLORS] = gradient->getMarks().size();
}

void ComponentBillboard::Init() {
	if (!gradient) gradient = new ImGradient();
	ComponentTransform* transform = GetOwner().GetComponent<ComponentTransform>();
	initPos = transform->GetGlobalPosition();
	previousPos = transform->GetGlobalRotation() * float3::unitY;
	modelStretch = float3x3::FromEulerXYZ(0.f, 0.f, -pi / 2);
}

void ComponentBillboard::Update() {
	// Play On Awake activation in Game
	if (!isStarted && App->time->HasGameStarted() && playOnAwake) {
		Play();
		isStarted = true;
	}

	if (time > billboardLifetime && !animationLoop && !colorLoop) {
		isPlaying = false;
	}

	if (isPlaying) {
		time += App->time->GetDeltaTimeOrRealDeltaTime();

		if (colorOverLifetime) {
			float timeMod = fmod(time, billboardLifetime / colorCycles);
			colorFrame = timeMod / billboardLifetime * colorCycles;
			if (!colorLoop && time > billboardLifetime) {
				colorFrame = 1.0;
			}
		}

		int totalTiles = Xtiles * Ytiles;
		currentFrame = time / billboardLifetime * animationCycles * totalTiles;
		if (!animationLoop && time > billboardLifetime) {
			currentFrame = (float) totalTiles;
		}
	}

	// Direction for Stretch
	ComponentTransform* transform = GetOwner().GetComponent<ComponentTransform>();
	float3 position = transform->GetGlobalPosition();
	if (!previousPos.Equals(position)) {
		direction = (position - previousPos).Normalized();
	}
	previousPos = position;
}

void ComponentBillboard::Draw() {
	ProgramBillboard* program = App->programs->billboard;
	glUseProgram(program->program);

	unsigned glTexture = 0;
	ResourceTexture* texture = App->resources->GetResource<ResourceTexture>(textureID);
	glTexture = texture ? texture->glTexture : 0;
	int hasDiffuseMap = texture ? 1 : 0;

	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	if (renderMode == ParticleRenderMode::ADDITIVE) {
		glBlendFunc(GL_ONE, GL_ONE);
	} else {
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	glBindBuffer(GL_ARRAY_BUFFER, App->userInterface->GetQuadVBO());
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*) (sizeof(float) * 6 * 3));

	ComponentTransform* transform = GetOwner().GetComponent<ComponentTransform>();

	float3x3 rotatePart = transform->GetGlobalMatrix().RotatePart();
	float3 position = transform->GetGlobalPosition();
	float4x4 modelMatrix = transform->GetGlobalMatrix();
	float3 scale = transform->GetGlobalScale();

	Frustum* frustum = App->camera->GetActiveCamera()->GetFrustum();
	float4x4* proj = &App->camera->GetProjectionMatrix();
	float4x4* view = &App->camera->GetViewMatrix();

	float4x4 newModelMatrix;

	if (billboardType == BillboardType::NORMAL) {
		newModelMatrix = modelMatrix.LookAt(rotatePart.Col(2), -frustum->Front(), rotatePart.Col(1), float3::unitY);
		newModelMatrix = float4x4::FromTRS(position, newModelMatrix.RotatePart() * rotatePart, scale);

	} else if (billboardType == BillboardType::STRETCH) {
		float3 cameraPos = App->camera->GetActiveCamera()->GetFrustum()->Pos();
		float3 cameraDir = (cameraPos - position).Normalized();
		float3 upDir = Cross(direction, cameraDir);
		float3 newCameraDir = Cross(direction, upDir);

		float3x3 newRotation;
		newRotation.SetCol(0, upDir);
		newRotation.SetCol(1, direction);
		newRotation.SetCol(2, newCameraDir);

		newModelMatrix = float4x4::FromTRS(position, newRotation * modelStretch, scale);

	} else if (billboardType == BillboardType::HORIZONTAL) {
		if (isHorizontalOrientation) {
			float3 direction = transform->GetGlobalRotation().WorldZ();
			float3 projection = position + direction - direction.y * float3::unitY;
			direction = (projection - position).Normalized();
			float3 right = Cross(float3::unitY, direction);

			float3x3 newRotation;
			newRotation.SetCol(1, right);
			newRotation.SetCol(2, float3::unitY);
			newRotation.SetCol(0, direction);

			newModelMatrix = float4x4::FromTRS(position, newRotation, scale);
		} else {
			newModelMatrix = float4x4::LookAt(float3::unitZ, float3::unitY, float3::unitY, float3::unitY);
			newModelMatrix = float4x4::FromTRS(position, newModelMatrix.RotatePart(), scale);
		}

	} else if (billboardType == BillboardType::VERTICAL) {
		float3 cameraPos = App->camera->GetActiveCamera()->GetFrustum()->Pos();
		float3 cameraDir = (float3(cameraPos.x, position.y, cameraPos.z) - position).Normalized();
		newModelMatrix = float4x4::LookAt(float3::unitZ, cameraDir, float3::unitY, float3::unitY);
		newModelMatrix = float4x4::FromTRS(position, newModelMatrix.RotatePart(), scale);
	}

	glUniformMatrix4fv(program->modelLocation, 1, GL_TRUE, newModelMatrix.ptr());
	glUniformMatrix4fv(program->viewLocation, 1, GL_TRUE, view->ptr());
	glUniformMatrix4fv(program->projLocation, 1, GL_TRUE, proj->ptr());

	float4 color = float4::one;
	if (colorOverLifetime) {
		gradient->getColorAt(colorFrame, color.ptr());
	}

	glUniform1i(program->diffuseMapLocation, 0);
	glUniform1i(program->hasDiffuseLocation, hasDiffuseMap);
	glUniform4fv(program->inputColorLocation, 1, color.ptr());
	glUniform3fv(program->intensityLocation, 1, textureIntensity.ptr());

	glUniform1f(program->currentFrameLocation, currentFrame);

	glUniform1i(program->xTilesLocation, Xtiles);
	glUniform1i(program->yTilesLocation, Ytiles);

	glUniform1i(program->xFlipLocation, flipTexture[0]);
	glUniform1i(program->yFlipLocation, flipTexture[1]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, glTexture);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}

void ComponentBillboard::Play() {
	isPlaying = true;
	time = 0.0f;
}

void ComponentBillboard::Stop() {
	isPlaying = false;
}

// Getters
float ComponentBillboard::GetBillboardLifetime() const {
	return billboardLifetime;
}

bool ComponentBillboard::GetPlayOnAwake() const {
	return playOnAwake;
}

float ComponentBillboard::GetCurrentFrame() const {
	return currentFrame;
}

float3 ComponentBillboard::GetTextureIntensity() const {
	return textureIntensity;
}

float ComponentBillboard::GetAnimationCycles() const {
	return animationCycles;
}

bool ComponentBillboard::GetAnimationLoop() const {
	return animationLoop;
}

float ComponentBillboard::GetColorCycles() const {
	return colorCycles;
}

bool ComponentBillboard::GetColorLoop() const {
	return colorLoop;
}

// Setters
void ComponentBillboard::SetBillboardLifetime(float _billboardLifetime) {
	billboardLifetime = _billboardLifetime;
}

void ComponentBillboard::SetPlayOnAwake(bool _playOnAwake) {
	playOnAwake = _playOnAwake;
}

void ComponentBillboard::SetCurrentFrame(float _currentFrame) {
	currentFrame = _currentFrame;
}

void ComponentBillboard::SetTextureIntensity(float3 _textureIntensity) {
	textureIntensity = _textureIntensity;
}

void ComponentBillboard::SetAnimationCycles(float _animationCycles) {
	animationCycles = _animationCycles;
}

void ComponentBillboard::SetAnimationLoop(bool _animationLoop) {
	animationLoop = _animationLoop;
}

void ComponentBillboard::SetColorCycles(float _colorCycles) {
	colorCycles = _colorCycles;
}

void ComponentBillboard::SetColorLoop(bool _colorLoop) {
	colorLoop = _colorLoop;
}
