#include "ComponentVideo.h"

#include "Application.h"
#include "GameObject.h"
#include "Modules/ModulePrograms.h"
#include "Modules/ModuleCamera.h"
#include "Modules/ModuleRender.h"
#include "Modules/ModuleUserInterface.h"
#include "Components/UI/ComponentTransform2D.h"

#include "imgui.h"
#include "Utils/ImGuiUtils.h"
#include "Math/TransformOps.h"

#define JSON_TAG_VIDEOID "VideoId"

#include "Resources/ResourceTexture.h"
#include "Utils/Logging.h"

extern "C" {
	#include "libavformat/avformat.h"
	#include "libavutil/dict.h"
}

ComponentVideo::~ComponentVideo() {
}

void ComponentVideo::Init() {
	AVFormatContext* fmt_ctx = NULL;
	AVDictionaryEntry* tag = NULL;
	int ret;
	const char* filename = "./2021-04-21 16-56-15.mp4";
	if ((ret = avformat_open_input(&fmt_ctx, filename, NULL, NULL)))
		return;

	while ((tag = av_dict_get(fmt_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
		LOG("%s=%s\n", tag->key, tag->value);

	avformat_close_input(&fmt_ctx);
	// Initialise libav with the video
}

void ComponentVideo::Update() {
	// Change Frame
}

void ComponentVideo::OnEditorUpdate() {
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

	UID oldID = /*textureID*/ 0;
	/* ImGui::ResourceSlot<ResourceTexture>("texture", /*&textureID 0);

	ResourceTexture* textureResource = App->resources->GetResource<ResourceTexture>(/*textureID 0);

	if (textureResource != nullptr) {
		int width;
		int height;
		glGetTextureLevelParameteriv(textureResource->glTexture, 0, GL_TEXTURE_WIDTH, &width);
		glGetTextureLevelParameteriv(textureResource->glTexture, 0, GL_TEXTURE_HEIGHT, &height);

		if (oldID != /*textureID 0) {
			ComponentTransform2D* transform2D = GetOwner().GetComponent<ComponentTransform2D>();
			if (transform2D != nullptr) {
				transform2D->SetSize(float2((float) width, (float) height));
			}
		}

		ImGui::TextWrapped("Size: %d x %d", width, height);
	}*/
}

void ComponentVideo::Save(JsonValue jComponent) const {
	jComponent[JSON_TAG_VIDEOID] = videoID;
}

void ComponentVideo::Load(JsonValue jComponent) {
	videoID = jComponent[JSON_TAG_VIDEOID];
}

void ComponentVideo::Draw(ComponentTransform2D* transform) const {
	ProgramImageUI* imageUIProgram = App->programs->imageUI;
	if (imageUIProgram == nullptr) return;

	/* if (alphaTransparency) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	if (isFill) {
		glBindBuffer(GL_ARRAY_BUFFER, fillQuadVBO);
	} else {
		glBindBuffer(GL_ARRAY_BUFFER, App->userInterface->GetQuadVBO());
	}*/
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*) ((sizeof(float) * 6 * 3)));
	glUseProgram(imageUIProgram->program);

	float4x4 modelMatrix = transform->GetGlobalScaledMatrix();
	float4x4& proj = App->camera->GetProjectionMatrix();
	float4x4& view = App->camera->GetViewMatrix();

	if (App->userInterface->IsUsing2D()) {
		proj = float4x4::D3DOrthoProjLH(-1, 1, App->renderer->GetViewportSize().x, App->renderer->GetViewportSize().y); //near plane. far plane, screen width, screen height
		view = float4x4::identity;
	}

	ComponentCanvasRenderer* canvasRenderer = GetOwner().GetComponent<ComponentCanvasRenderer>();
	if (canvasRenderer != nullptr) {
		float factor = canvasRenderer->GetCanvasScreenFactor();
		view = view * float4x4::Scale(factor, factor, factor);
	}

	glUniformMatrix4fv(imageUIProgram->viewLocation, 1, GL_TRUE, view.ptr());
	glUniformMatrix4fv(imageUIProgram->projLocation, 1, GL_TRUE, proj.ptr());
	glUniformMatrix4fv(imageUIProgram->modelLocation, 1, GL_TRUE, modelMatrix.ptr());

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(imageUIProgram->diffuseLocation, 0);
	glUniform4fv(imageUIProgram->inputColorLocation, 1, (0,0,0)/*GetMainColor().ptr()*/);

	ResourceTexture* textureResource = App->resources->GetResource<ResourceTexture>(/*textureID*/0);
	if (textureResource != nullptr) {
		glBindTexture(GL_TEXTURE_2D, textureResource->glTexture);
		glUniform1i(imageUIProgram->hasDiffuseLocation, 1);
	} else {
		glUniform1i(imageUIProgram->hasDiffuseLocation, 0);
	}

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisable(GL_BLEND);
}
