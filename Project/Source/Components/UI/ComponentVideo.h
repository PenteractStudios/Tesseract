#pragma once
#include "Components/Component.h"

extern "C" {
	#include "libavutil/rational.h"
}
class ModuleTime;
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
	void VideoReaderClose();

private:
	UID videoID = 0;						  // Video file resource ID
	ProgramImageUI* imageUIProgram = nullptr; // Shader program
	unsigned int frameTexture = 0;			  // GL texture frame ID
	float elapsedVideoTime = 0;

	// Video Options
	bool verticalFlip = false;

	// Video frame data
	int frameWidth = 0;
	int frameHeight = 0;
	uint8_t* frameData = nullptr;

	// LibAV internal state
	int videoStreamIndex = -1;
	AVFormatContext* formatCtx = nullptr;
	AVCodecContext* codecCtx = nullptr;
	SwsContext* scalerCtx = nullptr;
	AVPacket* avPacket = nullptr;
	AVFrame* avFrame = nullptr;
	AVRational timeBase = {0, 0};
	float frameTime = 0;
};
