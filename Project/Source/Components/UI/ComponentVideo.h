#pragma once
#include "Components/Component.h"

extern "C" {
	#include "libavutil/rational.h"
}
class ModuleTime;
class ComponentAudioSource;
class ComponentTransform2D;
struct ProgramImageUI;
struct AVFormatContext;
struct AVCodecContext;
struct SwsContext;
struct AVFrame;
struct AVPacket;

class ComponentVideo : public Component {
public:
	REGISTER_COMPONENT(ComponentVideo, ComponentType::VIDEO, false); // Refer to ComponentType for the Constructor

	~ComponentVideo(); // Destructor

	void Init() override;							// Inits the component
	void Update() override;							// Update
	void OnEditorUpdate() override;					//
	void Save(JsonValue jComponent) const override; // Serializes object
	void Load(JsonValue jComponent) override;		// Deserializes object

	void Draw(ComponentTransform2D* transform); // Draws the current frame of the loaded video as a texture in a framebuffer

	void VideoReaderOpen(const char* filename);
	void ReadVideoFrame();
	void ReadAudioFrame();
	void VideoReaderClose();

private:
	UID videoID = 0;						  // Video file resource ID
	ProgramImageUI* imageUIProgram = nullptr; // Shader program
	unsigned int frameTexture = 0;			  // GL texture frame ID
	float elapsedVideoTime = 0;
	ComponentAudioSource* audioPlayer = nullptr;

	// Video Options
	bool verticalFlip = false;

	// Video frame data
	int frameWidth = 0;
	int frameHeight = 0;
	uint8_t* frameData = nullptr;

	// LibAV internal state
	int videoStreamIndex = -1;
	int audioStreamIndex = -1;
	AVFormatContext* formatCtx = nullptr;
	AVCodecContext* videoCodecCtx = nullptr;
	AVCodecContext* audioCodecCtx = nullptr;
	AVPacket* avPacket = nullptr;
	AVFrame* avFrame = nullptr;
	SwsContext* scalerCtx = nullptr;
	AVRational timeBase = {0, 0};
	float videoFrameTime = 0;
	float audioFrameTime = 0;

	// Audio Frame data
};