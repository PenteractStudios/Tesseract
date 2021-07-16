#include "ComponentVideo.h"

#include "Application.h"
#include "GameObject.h"
#include "Modules/ModulePrograms.h"
#include "Modules/ModuleCamera.h"
#include "Modules/ModuleRender.h"
#include "Modules/ModuleUserInterface.h"
#include "Components/UI/ComponentTransform2D.h"
#include "Utils/Logging.h"

#include "imgui.h"
#include "Utils/ImGuiUtils.h"
#include "Math/TransformOps.h"

#define JSON_TAG_VIDEOID "VideoId"

#include "Resources/ResourceTexture.h"

extern "C" {
	#include "libavformat/avformat.h"
}
char av_error[AV_ERROR_MAX_STRING_SIZE] = {0};
#define av_err2str(errnum) av_make_error_string(av_error, AV_ERROR_MAX_STRING_SIZE, errnum)


ComponentVideo::~ComponentVideo() {
	glDeleteTextures(1, &frameTexture);
	avformat_close_input(&formatCtx);
	avformat_free_context(formatCtx);
	avcodec_free_context(&codecCtx);
}

void ComponentVideo::Init() {
	// Load shader
	imageUIProgram = App->programs->imageUI;

	// Allocate format context
	formatCtx = avformat_alloc_context();
	if (!formatCtx) {
		LOG("Couldn't allocate AVFormatContext");
		return;
	}
	LoadVideoFile("./2021-04-21 16-56-15.mp4");

	glGenTextures(1, &frameTexture);
	glBindTexture(GL_TEXTURE_2D, frameTexture);

	// set necessary texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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

void ComponentVideo::Draw(ComponentTransform2D* transform) {
	if (imageUIProgram == nullptr) return;

	
	glBindBuffer(GL_ARRAY_BUFFER, App->userInterface->GetQuadVBO());
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
	glUniform4fv(imageUIProgram->inputColorLocation, 1, float4(1.f,1.f,1.f,1.f).ptr());

	// allocate memory and set texture data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, frameWidth, frameHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, frameData);

	glUniform1i(imageUIProgram->hasDiffuseLocation, 1);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisable(GL_BLEND);
}

void ComponentVideo::LoadVideoFile(const char* filename) {
	AVCodecParameters* codecParams;
	AVCodec* avCodec;

	// Open video file
	if (avformat_open_input(&formatCtx, filename, NULL, NULL) != 0) {
		LOG("Couldn't open video file");
		return;
	}
	
	// Find the first valid video stream in the file
	for (int i = 0; i < formatCtx->nb_streams; ++i) {
		auto stream = formatCtx->streams[i];
		codecParams = formatCtx->streams[i]->codecpar;
		avCodec = avcodec_find_decoder(codecParams->codec_id);
		if (!avCodec) continue;

		if (codecParams->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoStreamIndex = i;
			break;
		}
		// TODO: decode audio the same way AVMEDIA_TYPE_AUDIO
	}

	if (videoStreamIndex == -1) {
		LOG("Couldn't find valid video stream inside file");
		return;
	}

	// Set up a codec context for the decoder
	codecCtx = avcodec_alloc_context3(avCodec);
	if (!codecCtx) {
		LOG("Couldn't allocate AVCodecContext");
		return;
	}

	if (avcodec_parameters_to_context(codecCtx, codecParams) < 0) {
		LOG("Couldn't initialise AVCodecContext");
		return;
	}

	if (avcodec_open2(codecCtx, avCodec, NULL) < 0) {
		LOG("Couldn't open video codec");
		return;
	}

	ReadVideoFrame();
}

void ComponentVideo::ReadVideoFrame() {
	// Read Frame
	AVPacket* avPacket = av_packet_alloc();
	AVFrame* avFrame = av_frame_alloc();
	if (!avPacket) {
		LOG("Couldn't allocate AVPacket");
		return;
	}
	if (!avFrame) {
		LOG("Couldn't allocate AVFrame");
		return;
	}

	int response;
	while (av_read_frame(formatCtx, avPacket) >= 0) {
		if (avPacket->stream_index != videoStreamIndex) continue;

		response = avcodec_send_packet(codecCtx, avPacket);
		if (response < 0) {
			LOG("Failed to decode packet: %s", av_err2str(response));
			return;
		}

		response = avcodec_receive_frame(codecCtx, avFrame);
		if (response == AVERROR(EAGAIN) || response == AVERROR(EOF)) {
			continue;
		}
		if (response < 0) {
			LOG("Failed to decode frame: %s", av_err2str(response));
			return;
		}

		av_packet_unref(avPacket);
	}

	unsigned char* data = new unsigned char[avFrame->width * avFrame->height * 3];
	for (int x = 0; x < avFrame->width; ++x) {
		for (int y = 0; y < avFrame->height; ++y) {
			data[y * avFrame->width * 3 + x * 3] = avFrame->data[0][y * avFrame->linesize[0] + x];	   // R
			data[y * avFrame->width * 3 + x * 3 + 1] = avFrame->data[0][y * avFrame->linesize[0] + x]; // G
			data[y * avFrame->width * 3 + x * 3 + 2] = avFrame->data[0][y * avFrame->linesize[0] + x]; // B
		}
	}

	frameWidth = avFrame->width;
	frameHeight = avFrame->height;
	frameData = data;

	av_frame_free(&avFrame);
	av_packet_free(&avPacket);
}
