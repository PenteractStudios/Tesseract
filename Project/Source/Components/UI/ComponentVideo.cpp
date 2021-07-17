#include "ComponentVideo.h"

#include "Application.h"
#include "GameObject.h"
#include "Modules/ModuleUserInterface.h"
#include "Modules/ModulePrograms.h"
#include "Modules/ModuleCamera.h"
#include "Modules/ModuleRender.h"
#include "Modules/ModuleTime.h"
#include "Components/UI/ComponentTransform2D.h"
#include "Utils/Logging.h"

#include "imgui.h"
#include "Utils/ImGuiUtils.h"
#include "Math/TransformOps.h"

extern "C" {
	#include "libavformat/avformat.h"
	#include "libswscale/swscale.h"
}

#include "Utils/Leaks.h"


#define JSON_TAG_VIDEOID "VideoId"

char av_error[AV_ERROR_MAX_STRING_SIZE] = {0};
#define av_err2str(errnum) av_make_error_string(av_error, AV_ERROR_MAX_STRING_SIZE, errnum)


ComponentVideo::~ComponentVideo() {
	VideoReaderClose();

	// Release GL texture
	glDeleteTextures(1, &frameTexture);
}

void ComponentVideo::Init() {
	// Load shader
	imageUIProgram = App->programs->imageUI;

	// Set GL texture buffer
	glGenTextures(1, &frameTexture);
	glBindTexture(GL_TEXTURE_2D, frameTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	VideoReaderOpen("./2021-03-18 17-39-25.mp4");
}

void ComponentVideo::Update() {
	elapsedVideoTime += App->time->GetDeltaTime();
	if (elapsedVideoTime > frameTime || frameTime == 0) {
		ReadVideoFrame();
	}
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


	ImGui::Checkbox("Flip Vertically", &verticalFlip);
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, frameWidth, frameHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, frameData);

	glUniform1i(imageUIProgram->hasDiffuseLocation, 1);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisable(GL_BLEND);
}

void ComponentVideo::VideoReaderOpen(const char* filename) {
	// Open video file
	formatCtx = avformat_alloc_context();
	if (!formatCtx) {
		LOG("Couldn't allocate AVFormatContext");
		return;
	}
	if (avformat_open_input(&formatCtx, filename, NULL, NULL) != 0) {
		LOG("Couldn't open video file");
		return;
	}
	
	// Find a valid video stream in the file
	AVCodecParameters* codecParams;
	AVCodec* avCodec;
	videoStreamIndex = -1;
	for (unsigned int i = 0u; i < formatCtx->nb_streams; ++i) {
		codecParams = formatCtx->streams[i]->codecpar;
		avCodec = avcodec_find_decoder(codecParams->codec_id);
		if (!avCodec) continue;

		if (codecParams->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoStreamIndex = i;
			frameWidth = codecParams->width;
			frameHeight = codecParams->height;
			timeBase = formatCtx->streams[i]->time_base;
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
	
	// Allocate memory for packets and frames
	avPacket = av_packet_alloc();
	if (!avPacket) {
		LOG("Couldn't allocate AVPacket");
		return;
	}
	avFrame = av_frame_alloc();
	if (!avFrame) {
		LOG("Couldn't allocate AVFrame");
		return;
	}

	// Allocate frame buffer
	frameData = new uint8_t[frameWidth * frameHeight * 4];
}

void ComponentVideo::ReadVideoFrame() {
	
	int response;
	while (av_read_frame(formatCtx, avPacket) >= 0) {
		if (avPacket->stream_index != videoStreamIndex) {
			av_packet_unref(avPacket);
			continue;
		}

		response = avcodec_send_packet(codecCtx, avPacket);
		if (response < 0) {
			LOG("Failed to decode packet: %s", av_err2str(response));
			return;
		}

		response = avcodec_receive_frame(codecCtx, avFrame);
		if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
			av_packet_unref(avPacket);
			continue;
		}
		if (response < 0) {
			LOG("Failed to decode frame: %s", av_err2str(response));
			return;
		}

		av_packet_unref(avPacket);
		break;
	}

	frameTime = avFrame->pts * timeBase.num / (float)timeBase.den;
	// ------------------------------- TODO: can we read frames directly in RGB?

	if (!scalerCtx) {
		// Set SwScaler - Scale frame size + Pixel converter to RGB
		// TODO: Set destination size
		scalerCtx = sws_getContext(frameWidth, frameHeight, codecCtx->pix_fmt, /**/ avFrame->width, /**/ avFrame->height, AV_PIX_FMT_RGB0, SWS_FAST_BILINEAR, NULL, NULL, NULL);

		if (!scalerCtx) {
			LOG("Couldn't initialise SwScaler.");
			return;
		}
	}

	// -------------------------------------------

	// Transform pixel format to RGB
	if (!verticalFlip) {	// We flip the image by default. To have an inverted image, don't do the flipping 
		avFrame->data[0] += avFrame->linesize[0] * (codecCtx->height - 1);
		avFrame->linesize[0] *= -1;
		avFrame->data[1] += avFrame->linesize[1] * (codecCtx->height / 2 - 1);
		avFrame->linesize[1] *= -1;
		avFrame->data[2] += avFrame->linesize[2] * (codecCtx->height / 2 - 1);
		avFrame->linesize[2] *= -1;
	}
	uint8_t* dest[4] = {frameData, NULL, NULL, NULL};
	int linSize[4] = {frameWidth * 4, 0, 0, 0};
	sws_scale(scalerCtx, avFrame->data, avFrame->linesize, 0, frameHeight, dest, linSize);


}

void ComponentVideo::VideoReaderClose() {
	// Close libAV context -  free allocated memory
	sws_freeContext(scalerCtx);
	scalerCtx = nullptr;
	avformat_close_input(&formatCtx);
	avformat_free_context(formatCtx);
	avcodec_free_context(&codecCtx);
	av_frame_free(&avFrame);
	av_packet_free(&avPacket);

	// Release frame data buffer
	RELEASE(frameData);
}
