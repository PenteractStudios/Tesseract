#pragma once

#include "Module.h"
#include "Utils/Quadtree.h"
#include "Rendering/LightFrustum.h"

#include "MathGeoLibFwd.h"
#include "Math/float3.h"

#include <map>

#define SSAO_KERNEL_SIZE 64
#define RANDOM_TANGENTS_ROWS 4
#define RANDOM_TANGENTS_COLS 4

class GameObject;

class ModuleRender : public Module {
public:
	// ------- Core Functions ------ //
	bool Init() override;

	bool Start() override;
	UpdateStatus PreUpdate() override;
	UpdateStatus Update() override;
	UpdateStatus PostUpdate() override;
	bool CleanUp() override;
	void ReceiveEvent(TesseractEvent& ev) override;

	void ViewportResized(int width, int height); // Updates the viewport aspect ratio with the new one given by parameters. It will set 'viewportUpdated' to true, to regenerate the framebuffer to its new size using UpdateFramebuffers().
	void UpdateFramebuffers();					 // Generates the rendering framebuffer on Init(). If 'viewportUpdated' was set to true, it will be also called at PostUpdate().

	void SetVSync(bool vsync);

	// -------- Game Debug --------- //
	void ToggleDebugMode();
	void ToggleDebugDraw();
	void ToggleDrawQuadtree();
	void ToggleDrawBBoxes();
	void ToggleDrawSkybox(); // TODO: review Godmodecamera
	void ToggleDrawAnimationBones();
	void ToggleDrawCameraFrustums();
	void ToggleDrawLightGizmos();
	void ToggleDrawLightFrustumGizmo();
	void ToggleDrawParticleGizmos();

	void UpdateShadingMode(const char* shadingMode);

	float4x4 GetLightViewMatrix() const;
	float4x4 GetLightProjectionMatrix() const;

	int GetCulledTriangles() const;
	const float2 GetViewportSize();

	bool ObjectInsideFrustum(GameObject* gameObject);

public:
	void* context = nullptr; // SDL context.

	// - Render Buffer GL pointers - //
	unsigned cubeVAO = 0;
	unsigned cubeVBO = 0;

	unsigned renderTexture = 0;
	unsigned positionsTexture = 0;
	unsigned normalsTexture = 0;
	unsigned depthMapTextures[NUM_CASCADE_FRUSTUM] = {0, 0, 0, 0};
	unsigned ssaoTexture = 0;
	unsigned auxBlurTexture = 0;

	unsigned depthBuffer = 0;

	unsigned framebuffer = 0;
	unsigned depthPrepassTextureBuffer = 0;
	unsigned depthMapTextureBuffer = 0;
	unsigned ssaoTextureBuffer = 0;
	unsigned ssaoBlurTextureBufferH = 0;
	unsigned ssaoBlurTextureBufferV = 0;

	// ------- Viewport Updated ------- //
	bool viewportUpdated = true;

	// -- Debugging Tools Toggles -- //
	bool debugMode = false; // Flag to activate DrawOptions only ingame (not use in the engine)
	bool drawDebugDraw = true;
	bool drawQuadtree = true;
	bool drawAllBoundingBoxes = false;
	bool skyboxActive = true; // TODO: review Godmodecamera
	bool drawAllBones = false;
	bool drawCameraFrustums = false;
	bool drawLightGizmos = false;
	bool drawLightFrustumGizmo = false;
	bool drawNavMesh = false;
	bool drawParticleGizmos = false;
	bool drawColliders = false;
	int culledTriangles = 0;

	float3 ambientColor = {0.25f, 0.25f, 0.25f}; // Color of ambient Light
	float3 clearColor = {0.1f, 0.1f, 0.1f};		 // Color of the viewport between frames

	bool ssaoActive = true;
	float ssaoRange = 1.0f;
	float ssaoBias = 0.0f;
	float ssaoPower = 1.0f;

	LightFrustum lightFrustum;

private:
	void DrawQuadtreeRecursive(const Quadtree<GameObject>::Node& node, const AABB2D& aabb);			  // Draws the quadrtee nodes if 'drawQuadtree' is set to true.
	void ClassifyGameObjects();																		  // Classify Game Objects from Scene taking into account Frustum Culling, Shadows and Rendering Mode
	void ClassifyGameObjectsFromQuadtree(const Quadtree<GameObject>::Node& node, const AABB2D& aabb); // Classify Game Objects from Scene taking into account Frustum Culling, Quadtree, Shadows and Rendering Mode
	//bool CheckIfInsideFrustum(const AABB& aabb, const OBB& obb);									  // ??
	void DrawGameObject(GameObject* gameObject);													  // ??
	void DrawGameObjectDepthPrepass(GameObject* gameObject);
	void DrawGameObjectShadowPass(GameObject* gameObject);
	void DrawAnimation(const GameObject* gameObject, bool hasAnimation = false);
	void RenderUI();
	void SetOrtographicRender();
	void SetPerspectiveRender();

	void ComputeSSAOTexture();
	void BlurSSAOTexture(bool horizontal);

	void DrawTexture(unsigned texture);

private:
	// ------- Viewport Size ------- //
	float2 viewportSize = float2::zero;
	bool drawDepthMapTexture = false;
	bool drawSSAOTexture = false;

	std::vector<GameObject*> shadowGameObjects;			 // Vector of Shadow Casted GameObjects
	std::vector<GameObject*> opaqueGameObjects;			 // Vector of Opaque GameObjects
	std::map<float, GameObject*> transparentGameObjects; // Map with Transparent GameObjects

	float3 ssaoKernel[SSAO_KERNEL_SIZE];
	float3 randomTangents[RANDOM_TANGENTS_ROWS * RANDOM_TANGENTS_COLS];
};
