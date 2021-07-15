#pragma once
#include "Components/Component.h"

class ComponentTransform2D;
struct AVFormatContext;

class ComponentVideo : public Component {
public:
	REGISTER_COMPONENT(ComponentVideo, ComponentType::VIDEO, false); // Refer to ComponentType for the Constructor

	~ComponentVideo(); // Destructor

	void Init() override;							// Inits the component
	void Update() override;							// Update
	void OnEditorUpdate() override;					//
	void Save(JsonValue jComponent) const override; // Serializes object
	void Load(JsonValue jComponent) override;		// Deserializes object

	void Draw(ComponentTransform2D* transform) const; // Draws the current frame of the loaded video as a texture in a framebuffer

	void LoadVideoFile(const char* filename, int* widthOut, int* heightOut, unsigned char** dataOut);
	void ReadVideoFrame();

private:
	UID videoID = 0; // ID of the video

	AVFormatContext* formatCtx;
};
