#pragma once
#include "Components/Component.h"

class ComponentTransform2D;
struct ProgramImageUI;
struct AVFormatContext;
struct AVCodecContext;

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

	void LoadVideoFile(const char* filename);
	void ReadVideoFrame();

private:
	UID videoID = 0; // ID of the video
	ProgramImageUI* imageUIProgram = nullptr;

	unsigned char* frameData = nullptr;
	int frameWidth = 0;
	int frameHeight = 0;

	int videoStreamIndex = -1;

	unsigned int frameTexture = 0; // DELETE ON DESTRUCTOR

	// LibAV members
	AVFormatContext* formatCtx = nullptr;
	AVCodecContext* codecCtx = nullptr;
};
