#include "ModuleRender.h"

#include "Globals.h"
#include "Application.h"
#include "GameObject.h"
#include "Utils/Logging.h"
#include "Utils/Random.h"
#include "Components/ComponentMeshRenderer.h"
#include "Components/ComponentBoundingBox.h"
#include "Components/ComponentTransform.h"
#include "Components/ComponentAnimation.h"
#include "Components/ComponentCamera.h"
#include "Components/ComponentParticleSystem.h"
#include "Components/ComponentTrail.h"
#include "Components/ComponentBillboard.h"
#include "Components/ComponentSkyBox.h"
#include "Components/ComponentLight.h"
#include "Modules/ModuleInput.h"
#include "Modules/ModuleWindow.h"
#include "Modules/ModuleCamera.h"
#include "Modules/ModuleDebugDraw.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModuleScene.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModulePrograms.h"
#include "Modules/ModuleEvents.h"
#include "Modules/ModuleUserInterface.h"
#include "Modules/ModuleTime.h"
#include "Modules/ModuleNavigation.h"
#include "Resources/ResourceMesh.h"
#include "TesseractEvent.h"

#include "Geometry/AABB.h"
#include "Geometry/AABB2D.h"
#include "Geometry/OBB.h"
#include "debugdraw.h"
#include "GL/glew.h"
#include "SDL.h"
#include "Brofiler.h"

#include "Utils/Leaks.h"
#include <string>

#define GAUSS_KERNEL_SIZE 3

static float gaussKernel[GAUSS_KERNEL_SIZE] = {0.38774f, 0.24477f, 0.06136f};

// clang-format off
static const float cubeVertices[108] = {
	// Front (x, y, z)
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	// Left (x, y, z)
	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,
	
	// Right (x, y, z)
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	
	// Back (x, y, z)
	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,
	
	// Top (x, y, z)
	-1.0f,  1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,
	
	// Bottom (x, y, z)
	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f,  1.0f
}; // clang-format on

#if _DEBUG
static void __stdcall OurOpenGLErrorFunction(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	const char *tmpSource = "", *tmpType = "", *tmpSeverity = "";
	switch (source) {
	case GL_DEBUG_SOURCE_API:
		tmpSource = "API";
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		tmpSource = "Window System";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		tmpSource = "Shader Compiler";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		tmpSource = "Third Party";
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		tmpSource = "Application";
		break;
	case GL_DEBUG_SOURCE_OTHER:
		tmpSource = "Other";
		break;
	};
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		tmpType = "Error";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		tmpType = "Deprecated Behaviour";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		tmpType = "Undefined Behaviour";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		tmpType = "Portability";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		tmpType = "Performance";
		break;
	case GL_DEBUG_TYPE_MARKER:
		tmpType = "Marker";
		break;
	case GL_DEBUG_TYPE_PUSH_GROUP:
		tmpType = "Push Group";
		break;
	case GL_DEBUG_TYPE_POP_GROUP:
		tmpType = "Pop Group";
		break;
	case GL_DEBUG_TYPE_OTHER:
		tmpType = "Other";
		break;
	};
	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:
		tmpSeverity = "high";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		tmpSeverity = "medium";
		break;
	case GL_DEBUG_SEVERITY_LOW:
		tmpSeverity = "low";
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		tmpSeverity = "notification";
		break;
	};

	if (severity != GL_DEBUG_SEVERITY_HIGH) {
		return;
	}

	LOG("<Source:%s> <Type:%s> <Severity:%s> <ID:%d> <Message:%s>", tmpSource, tmpType, tmpSeverity, id, message);
}
#endif

bool ModuleRender::Init() {
	LOG("Creating Renderer context");

	context = SDL_GL_CreateContext(App->window->window);

	GLenum err = glewInit();
	LOG("Using Glew %s", glewGetString(GLEW_VERSION));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glFrontFace(GL_CCW);

#if _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(&OurOpenGLErrorFunction, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
#endif

	glGenTextures(1, &renderTexture);
	glGenTextures(1, &outputTexture);
	glGenTextures(1, &positionsTexture);
	glGenTextures(1, &normalsTexture);
	for (unsigned i = 0; i < NUM_CASCADE_FRUSTUM; i++) {
		glGenTextures(1, &depthMapTextures[i]);
	}
	glGenTextures(1, &ssaoTexture);
	glGenTextures(1, &auxBlurTexture);

	glGenRenderbuffers(1, &depthBuffer);

	glGenFramebuffers(1, &renderPassBuffer);
	glGenFramebuffers(1, &depthPrepassTextureBuffer);
	glGenFramebuffers(1, &depthMapTextureBuffer);
	glGenFramebuffers(1, &ssaoTextureBuffer);
	glGenFramebuffers(1, &ssaoBlurTextureBufferH);
	glGenFramebuffers(1, &ssaoBlurTextureBufferV);
	glGenFramebuffers(1, &colorCorrectionBuffer);

	ViewportResized(App->window->GetWidth(), App->window->GetHeight());
	UpdateFramebuffers();
	
	// Create Unit Cube VAO
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);

	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
	glBindVertexArray(0);

	// Calculate SSAO kernel
	for (unsigned i = 0; i < SSAO_KERNEL_SIZE; ++i) {
		float3 position;

		// Random direction
		position.x = Random() * 2.0f - 1.0f;
		position.y = Random() * 2.0f - 1.0f;
		position.z = Random();
		position.Normalize();

		// Random distance
		position *= Random();

		// Distribute according to (y = 0.1f + 0.9x^2)
		float scale = float(i) / float(SSAO_KERNEL_SIZE);
		scale = 0.1f + (scale * scale) * (1.0f - 0.1f);
		position *= scale;

		ssaoKernel[i] = position;
	}

	// Calculate random tangents
	for (unsigned i = 0; i < RANDOM_TANGENTS_ROWS * RANDOM_TANGENTS_COLS; ++i) {
		float3 tangent;
		tangent.x = Random() * 2.0f - 1.0f;
		tangent.y = Random() * 2.0f - 1.0f;
		tangent.z = 0.0f;
		tangent.Normalize();
		randomTangents[i] = tangent;
	}

	return true;
}

void ModuleRender::ClassifyGameObjects() {
	shadowGameObjects.clear();
	opaqueGameObjects.clear();
	transparentGameObjects.clear();

	App->camera->CalculateFrustumPlanes();
	float3 cameraPos = App->camera->GetActiveCamera()->GetFrustum()->Pos();
	Scene* scene = App->scene->scene;
	for (ComponentBoundingBox& boundingBox : scene->boundingBoxComponents) {
		GameObject& gameObject = boundingBox.GetOwner();
		gameObject.flag = false;
		if (gameObject.isInQuadtree) continue;

		if ((gameObject.GetMask().bitMask & static_cast<int>(MaskType::CAST_SHADOWS)) != 0) {
			shadowGameObjects.push_back(&gameObject);
		}

		const AABB& gameObjectAABB = boundingBox.GetWorldAABB();
		const OBB& gameObjectOBB = boundingBox.GetWorldOBB();
		if (App->camera->GetFrustumPlanes().CheckIfInsideFrustum(gameObjectAABB, gameObjectOBB)) {
			if ((gameObject.GetMask().bitMask & static_cast<int>(MaskType::TRANSPARENT)) == 0) {
				opaqueGameObjects.push_back(&gameObject);
			} else {
				ComponentTransform* transform = gameObject.GetComponent<ComponentTransform>();
				float dist = Length(cameraPos - transform->GetGlobalPosition());
				transparentGameObjects[dist] = &gameObject;
			}
		}
	}
	if (scene->quadtree.IsOperative()) {
		ClassifyGameObjectsFromQuadtree(scene->quadtree.root, scene->quadtree.bounds);
	}
}

void ModuleRender::ComputeSSAOTexture() {
	ProgramSSAO* ssaoProgram = App->programs->ssao;
	if (ssaoProgram == nullptr) return;

	float4x4 viewMatrix = App->camera->GetViewMatrix();
	float4x4 projMatrix = App->camera->GetProjectionMatrix();

	glUseProgram(ssaoProgram->program);

	glUniformMatrix4fv(ssaoProgram->projLocation, 1, GL_TRUE, projMatrix.ptr());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, positionsTexture);
	glUniform1i(ssaoProgram->positionsLocation, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalsTexture);
	glUniform1i(ssaoProgram->normalsLocation, 1);

	glUniform3fv(ssaoProgram->kernelSamplesLocation, SSAO_KERNEL_SIZE, ssaoKernel[0].ptr());
	glUniform3fv(ssaoProgram->randomTangentsLocation, RANDOM_TANGENTS_ROWS * RANDOM_TANGENTS_COLS, randomTangents[0].ptr());
	glUniform2f(ssaoProgram->screenSizeLocation, viewportSize.x, viewportSize.y);
	glUniform1f(ssaoProgram->biasLocation, ssaoBias);
	glUniform1f(ssaoProgram->rangeLocation, ssaoRange);
	glUniform1f(ssaoProgram->powerLocation, ssaoPower);

	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void ModuleRender::BlurSSAOTexture(bool horizontal) {
	ProgramSSAOBlur* ssaoBlurProgram = App->programs->ssaoBlur;
	if (ssaoBlurProgram == nullptr) return;

	glUseProgram(ssaoBlurProgram->program);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, horizontal ? ssaoTexture : auxBlurTexture);
	glUniform1i(ssaoBlurProgram->inputTextureLocation, 0);

	glUniform1fv(ssaoBlurProgram->kernelLocation, GAUSS_KERNEL_SIZE, gaussKernel);
	glUniform1i(ssaoBlurProgram->horizontalLocation, horizontal ? 1 : 0);

	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void ModuleRender::ExecuteColorCorrection() {
	ProgramColorCorrection* colorCorrectionProgram = App->programs->colorCorrection;
	if (colorCorrectionProgram == nullptr) return;

	glUseProgram(colorCorrectionProgram->program);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderTexture);
	glUniform1i(colorCorrectionProgram->inputTextureLocation, 0);

	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void ModuleRender::DrawTexture(unsigned texture) {
	ProgramDrawTexture* drawTextureProgram = App->programs->drawTexture;
	if (drawTextureProgram == nullptr) return;

	glUseProgram(drawTextureProgram->program);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(drawTextureProgram->textureToDrawLocation, 0);

	glDrawArrays(GL_TRIANGLES, 0, 3);
}

bool ModuleRender::Start() {
	App->events->AddObserverToEvent(TesseractEventType::SCREEN_RESIZED, this);
	return true;
}

UpdateStatus ModuleRender::PreUpdate() {
	BROFILER_CATEGORY("ModuleRender - PreUpdate", Profiler::Color::Green)

	lightFrustum.ReconstructFrustum();

	glBindFramebuffer(GL_FRAMEBUFFER, renderPassBuffer);
#if !GAME
	glViewport(0, 0, static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y));
#else
	App->camera->ViewportResized(App->window->GetWidth(), App->window->GetHeight());
	glViewport(0, 0, App->window->GetWidth(), App->window->GetHeight());
#endif

	return UpdateStatus::CONTINUE;
}

UpdateStatus ModuleRender::Update() {
	BROFILER_CATEGORY("ModuleRender - Update", Profiler::Color::Green)

	culledTriangles = 0;
	Scene* scene = App->scene->scene;
	float3 gammaClearColor = float3(pow(clearColor.x, 2.2f), pow(clearColor.y, 2.2f), pow(clearColor.y, 2.2f));

	lightFrustum.UpdateFrustums();

	ClassifyGameObjects();

	// Shadow Pass
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapTextureBuffer);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glClear(GL_DEPTH_BUFFER_BIT);

	for (GameObject* gameObject : shadowGameObjects) {
		DrawGameObjectShadowPass(gameObject);
	}

	// Depth Prepass
	glBindFramebuffer(GL_FRAMEBUFFER, depthPrepassTextureBuffer);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (GameObject* gameObject : opaqueGameObjects) {
		DrawGameObjectDepthPrepass(gameObject);
	}

	// SSAO pass
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoTextureBuffer);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LESS);
	glClear(GL_COLOR_BUFFER_BIT);

	if (ssaoActive) {
		ComputeSSAOTexture();
	}

	// SSAO horitontal blur
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurTextureBufferH);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT);

	if (ssaoActive) {
		BlurSSAOTexture(true);
	}

	// SSAO vertical blur
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurTextureBufferV);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT);

	if (ssaoActive) {
		BlurSSAOTexture(false);
	}

	// Render pass
	glBindFramebuffer(GL_FRAMEBUFFER, renderPassBuffer);
	glClearColor(gammaClearColor.x, gammaClearColor.y, gammaClearColor.z, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL);
	glClear(GL_COLOR_BUFFER_BIT);

	// Debug textures
	if (drawSSAOTexture) {
		DrawTexture(ssaoTexture);
		return UpdateStatus::CONTINUE;
	}
	if (drawDepthMapTexture) {
		DrawTexture(depthMapTextures[0]);
		return UpdateStatus::CONTINUE;
	}

	// Draw SkyBox (Always first element)
	for (ComponentSkyBox& skybox : scene->skyboxComponents) {
		if (skybox.IsActive()) skybox.Draw();
	}

	// Draw Opaque
	glDepthFunc(GL_EQUAL);
	for (GameObject* gameObject : opaqueGameObjects) {
		DrawGameObject(gameObject);
	}
	glDepthFunc(GL_LEQUAL);

	// Draw Transparent
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for (std::map<float, GameObject*>::reverse_iterator it = transparentGameObjects.rbegin(); it != transparentGameObjects.rend(); ++it) {
		DrawGameObject((*it).second);
	}
	glDisable(GL_BLEND);

	// Draw particles (TODO: improve with culling)
	for (ComponentParticleSystem& particleSystem : scene->particleComponents) {
		if (particleSystem.IsActive()) particleSystem.Draw();
	}
	for (ComponentBillboard& billboard : scene->billboardComponents) {
		if (billboard.IsActive()) billboard.Draw();
	}
	for (ComponentTrail& trail : scene->trailComponents) {
		if (trail.IsActive()) trail.Draw();
	}

	// Draw Gizmos
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	if (App->camera->IsEngineCameraActive() || debugMode) {
		// Draw NavMesh
		if (drawNavMesh) {
			App->navigation->DrawGizmos();
		}

		GameObject* selectedGameObject = App->editor->selectedGameObject;
		if (selectedGameObject) selectedGameObject->DrawGizmos();

		// --- All Gizmos options
		if (drawCameraFrustums) {
			for (ComponentCamera& camera : scene->cameraComponents) {
				camera.DrawGizmos();
			}
		}
		if (drawLightGizmos) {
			for (ComponentLight& light : scene->lightComponents) {
				light.DrawGizmos();
			}
		}
		if (drawParticleGizmos) {
			for (ComponentParticleSystem& particle : scene->particleComponents) {
				particle.DrawGizmos();
			}
		}

		if (drawColliders) {
			for (ComponentBoxCollider& collider : scene->boxColliderComponents) {
				collider.DrawGizmos();
			}
			for (ComponentSphereCollider& collider : scene->sphereColliderComponents) {
				collider.DrawGizmos();
			}
			for (ComponentCapsuleCollider& collider : scene->capsuleColliderComponents) {
				collider.DrawGizmos();
			}
		}

		// Draw quadtree
		if (drawQuadtree) DrawQuadtreeRecursive(App->scene->scene->quadtree.root, App->scene->scene->quadtree.bounds);

		// Draw debug draw
		if (drawDebugDraw) App->debugDraw->Draw(App->camera->GetViewMatrix(), App->camera->GetProjectionMatrix(), static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y));

		// Draw Animations
		if (drawAllBones) {
			for (ComponentAnimation& animationComponent : App->scene->scene->animationComponents) {
				GameObject* rootBone = animationComponent.GetOwner().GetRootBone();
				if (rootBone) DrawAnimation(rootBone);
			}
		}

		// Draw debug draw Light Frustum
		if (drawLightFrustumGizmo) {
			lightFrustum.DrawGizmos();
		}
	}

	//Render UI
	RenderUI();

	// Color correction
	glBindFramebuffer(GL_FRAMEBUFFER, colorCorrectionBuffer);
	glClearColor(gammaClearColor.x, gammaClearColor.y, gammaClearColor.z, 1.0f);
	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT);

	ExecuteColorCorrection();

	// Render to screen
#if GAME
	glBindFramebuffer(GL_READ_FRAMEBUFFER, colorCorrectionBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y), 0, 0, static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y), GL_COLOR_BUFFER_BIT, GL_NEAREST);
#else
	glBindFramebuffer(GL_READ_FRAMEBUFFER, colorCorrectionBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderPassBuffer);
	glBlitFramebuffer(0, 0, static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y), 0, 0, static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y), GL_COLOR_BUFFER_BIT, GL_NEAREST);
#endif

	return UpdateStatus::CONTINUE;
}

UpdateStatus ModuleRender::PostUpdate() {
	BROFILER_CATEGORY("ModuleRender - PostUpdate", Profiler::Color::Green)

#if !GAME
	if (viewportUpdated) {
		UpdateFramebuffers();
		viewportUpdated = false;
	}
#endif

	SDL_GL_SwapWindow(App->window->window);

	return UpdateStatus::CONTINUE;
}

bool ModuleRender::CleanUp() {
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVBO);

	glDeleteTextures(1, &renderTexture);
	glDeleteTextures(1, &outputTexture);
	glDeleteTextures(1, &positionsTexture);
	glDeleteTextures(1, &normalsTexture);
	for (unsigned i = 0; i < NUM_CASCADE_FRUSTUM; i++) {
		glDeleteTextures(1, &depthMapTextures[i]);
	}
	glDeleteTextures(1, &ssaoTexture);
	glDeleteTextures(1, &auxBlurTexture);

	glDeleteRenderbuffers(1, &depthBuffer);

	glDeleteFramebuffers(1, &renderPassBuffer);
	glDeleteFramebuffers(1, &depthPrepassTextureBuffer);
	glDeleteFramebuffers(1, &depthMapTextureBuffer);
	glDeleteFramebuffers(1, &ssaoTextureBuffer);
	glDeleteFramebuffers(1, &ssaoBlurTextureBufferH);
	glDeleteFramebuffers(1, &ssaoBlurTextureBufferV);
	glDeleteFramebuffers(1, &colorCorrectionBuffer);

	return true;
}

void ModuleRender::ViewportResized(int width, int height) {
	viewportSize.x = static_cast<float>(width);
	viewportSize.y = static_cast<float>(height);

	viewportUpdated = true;
}

void ModuleRender::ReceiveEvent(TesseractEvent& ev) {
	switch (ev.type) {
	case TesseractEventType::SCREEN_RESIZED:
		ViewportResized(ev.Get<ViewportResizedStruct>().newWidth, ev.Get<ViewportResizedStruct>().newHeight);
		break;
	default:
		break;
	}
}

void ModuleRender::UpdateFramebuffers() {
	// Depth buffer
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y));
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// Depth prepass buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthPrepassTextureBuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

	glBindTexture(GL_TEXTURE_2D, positionsTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y), 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, positionsTexture, 0);

	glBindTexture(GL_TEXTURE_2D, normalsTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y), 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalsTexture, 0);

	GLuint drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glDrawBuffers(2, drawBuffers);

	// Shadow buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapTextureBuffer);

	glBindTexture(GL_TEXTURE_2D, depthMapTextures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y), 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTextures[0], 0);

	glDrawBuffer(GL_NONE);

	// SSAO buffer
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoTextureBuffer);

	glBindTexture(GL_TEXTURE_2D, ssaoTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y), 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoTexture, 0);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	// SSAO horizontal blur buffer
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurTextureBufferH);

	glBindTexture(GL_TEXTURE_2D, auxBlurTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y), 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, auxBlurTexture, 0);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	// SSAO vertical blur buffer
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurTextureBufferV);

	glBindTexture(GL_TEXTURE_2D, ssaoTexture);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoTexture, 0);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	// Render buffer
	glBindFramebuffer(GL_FRAMEBUFFER, renderPassBuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

	glBindTexture(GL_TEXTURE_2D, renderTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y), 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	// Color correction buffer
	glBindFramebuffer(GL_FRAMEBUFFER, colorCorrectionBuffer);

	glBindTexture(GL_TEXTURE_2D, outputTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTexture, 0);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		LOG("ERROR: Framebuffer is not complete!");
	}
}

void ModuleRender::SetVSync(bool vsync) {
	SDL_GL_SetSwapInterval(vsync);
}

void ModuleRender::ToggleDebugMode() {
	debugMode = !debugMode;
}

void ModuleRender::ToggleDebugDraw() {
	drawDebugDraw = !drawDebugDraw;
}

void ModuleRender::ToggleDrawQuadtree() {
	drawQuadtree = !drawQuadtree;
}

void ModuleRender::ToggleDrawBBoxes() {
	drawAllBoundingBoxes = !drawAllBoundingBoxes;
}

void ModuleRender::ToggleDrawSkybox() { // TODO: review Godmodecamera
	skyboxActive = !skyboxActive;
}

void ModuleRender::ToggleDrawAnimationBones() {
	drawAllBones = !drawAllBones;
}

void ModuleRender::ToggleDrawCameraFrustums() {
	drawCameraFrustums = !drawCameraFrustums;
}

void ModuleRender::ToggleDrawLightGizmos() {
	drawLightGizmos = !drawLightGizmos;
}

void ModuleRender::ToggleDrawParticleGizmos() {
	drawParticleGizmos = !drawParticleGizmos;
}

void ModuleRender::ToggleDrawLightFrustumGizmo() {
	drawLightFrustumGizmo = !drawLightFrustumGizmo;
}

void ModuleRender::UpdateShadingMode(const char* shadingMode) {
	drawDepthMapTexture = false;
	drawSSAOTexture = false;

	if (strcmp(shadingMode, "Shaded") == 0) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	} else if (strcmp(shadingMode, "Wireframe") == 0) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else if (strcmp(shadingMode, "Depth") == 0) {
		drawDepthMapTexture = true;
	} else if (strcmp(shadingMode, "Ambient Occlusion") == 0) {
		drawSSAOTexture = true;
	}
}

float4x4 ModuleRender::GetLightViewMatrix() const {
	return lightFrustum.GetFrustum().ViewMatrix();
}

float4x4 ModuleRender::GetLightProjectionMatrix() const {
	return lightFrustum.GetFrustum().ProjectionMatrix();
}

int ModuleRender::GetCulledTriangles() const {
	return culledTriangles;
}

void ModuleRender::DrawQuadtreeRecursive(const Quadtree<GameObject>::Node& node, const AABB2D& aabb) {
	if (node.IsBranch()) {
		vec2d center = aabb.minPoint + (aabb.maxPoint - aabb.minPoint) * 0.5f;

		const Quadtree<GameObject>::Node& topLeft = node.childNodes->nodes[0];
		AABB2D topLeftAABB = {{aabb.minPoint.x, center.y}, {center.x, aabb.maxPoint.y}};
		DrawQuadtreeRecursive(topLeft, topLeftAABB);

		const Quadtree<GameObject>::Node& topRight = node.childNodes->nodes[1];
		AABB2D topRightAABB = {{center.x, center.y}, {aabb.maxPoint.x, aabb.maxPoint.y}};
		DrawQuadtreeRecursive(topRight, topRightAABB);

		const Quadtree<GameObject>::Node& bottomLeft = node.childNodes->nodes[2];
		AABB2D bottomLeftAABB = {{aabb.minPoint.x, aabb.minPoint.y}, {center.x, center.y}};
		DrawQuadtreeRecursive(bottomLeft, bottomLeftAABB);

		const Quadtree<GameObject>::Node& bottomRight = node.childNodes->nodes[3];
		AABB2D bottomRightAABB = {{center.x, aabb.minPoint.y}, {aabb.maxPoint.x, center.y}};
		DrawQuadtreeRecursive(bottomRight, bottomRightAABB);
	} else {
		float3 points[8] = {
			{aabb.minPoint.x, 0, aabb.minPoint.y},
			{aabb.maxPoint.x, 0, aabb.minPoint.y},
			{aabb.maxPoint.x, 0, aabb.maxPoint.y},
			{aabb.minPoint.x, 0, aabb.maxPoint.y},
			{aabb.minPoint.x, 30, aabb.minPoint.y},
			{aabb.maxPoint.x, 30, aabb.minPoint.y},
			{aabb.maxPoint.x, 30, aabb.maxPoint.y},
			{aabb.minPoint.x, 30, aabb.maxPoint.y},
		};
		dd::box(points, dd::colors::White);
	}
}

void ModuleRender::ClassifyGameObjectsFromQuadtree(const Quadtree<GameObject>::Node& node, const AABB2D& aabb) {
	AABB aabb3d = AABB({aabb.minPoint.x, -1000000.0f, aabb.minPoint.y}, {aabb.maxPoint.x, 1000000.0f, aabb.maxPoint.y});
	if (App->camera->GetFrustumPlanes().CheckIfInsideFrustum(aabb3d, OBB(aabb3d))) {
		if (node.IsBranch()) {
			vec2d center = aabb.minPoint + (aabb.maxPoint - aabb.minPoint) * 0.5f;

			const Quadtree<GameObject>::Node& topLeft = node.childNodes->nodes[0];
			AABB2D topLeftAABB = {{aabb.minPoint.x, center.y}, {center.x, aabb.maxPoint.y}};
			ClassifyGameObjectsFromQuadtree(topLeft, topLeftAABB);

			const Quadtree<GameObject>::Node& topRight = node.childNodes->nodes[1];
			AABB2D topRightAABB = {{center.x, center.y}, {aabb.maxPoint.x, aabb.maxPoint.y}};
			ClassifyGameObjectsFromQuadtree(topRight, topRightAABB);

			const Quadtree<GameObject>::Node& bottomLeft = node.childNodes->nodes[2];
			AABB2D bottomLeftAABB = {{aabb.minPoint.x, aabb.minPoint.y}, {center.x, center.y}};
			ClassifyGameObjectsFromQuadtree(bottomLeft, bottomLeftAABB);

			const Quadtree<GameObject>::Node& bottomRight = node.childNodes->nodes[3];
			AABB2D bottomRightAABB = {{center.x, aabb.minPoint.y}, {aabb.maxPoint.x, center.y}};
			ClassifyGameObjectsFromQuadtree(bottomRight, bottomRightAABB);
		} else {
			float3 cameraPos = App->camera->GetActiveCamera()->GetFrustum()->Pos();

			const Quadtree<GameObject>::Element* element = node.firstElement;
			while (element != nullptr) {
				GameObject* gameObject = element->object;
				if (!gameObject->flag) {
					ComponentBoundingBox* boundingBox = gameObject->GetComponent<ComponentBoundingBox>();
					const AABB& gameObjectAABB = boundingBox->GetWorldAABB();
					const OBB& gameObjectOBB = boundingBox->GetWorldOBB();

					if ((gameObject->GetMask().bitMask & static_cast<int>(MaskType::CAST_SHADOWS)) != 0) {
						shadowGameObjects.push_back(gameObject);
					}

					if (App->camera->GetFrustumPlanes().CheckIfInsideFrustum(gameObjectAABB, gameObjectOBB)) {
						if ((gameObject->GetMask().bitMask & static_cast<int>(MaskType::TRANSPARENT)) == 0) {
							opaqueGameObjects.push_back(gameObject);
						} else {
							ComponentTransform* transform = gameObject->GetComponent<ComponentTransform>();
							float dist = Length(cameraPos - transform->GetGlobalPosition());
							transparentGameObjects[dist] = gameObject;
						}
					}

					gameObject->flag = true;
				}
				element = element->next;
			}
		}
	}
}

/*
bool ModuleRender::CheckIfInsideFrustum(const AABB& aabb, const OBB& obb) {
	float3 points[8] {
		obb.pos - obb.r.x * obb.axis[0] - obb.r.y * obb.axis[1] - obb.r.z * obb.axis[2],
		obb.pos - obb.r.x * obb.axis[0] - obb.r.y * obb.axis[1] + obb.r.z * obb.axis[2],
		obb.pos - obb.r.x * obb.axis[0] + obb.r.y * obb.axis[1] - obb.r.z * obb.axis[2],
		obb.pos - obb.r.x * obb.axis[0] + obb.r.y * obb.axis[1] + obb.r.z * obb.axis[2],
		obb.pos + obb.r.x * obb.axis[0] - obb.r.y * obb.axis[1] - obb.r.z * obb.axis[2],
		obb.pos + obb.r.x * obb.axis[0] - obb.r.y * obb.axis[1] + obb.r.z * obb.axis[2],
		obb.pos + obb.r.x * obb.axis[0] + obb.r.y * obb.axis[1] - obb.r.z * obb.axis[2],
		obb.pos + obb.r.x * obb.axis[0] + obb.r.y * obb.axis[1] + obb.r.z * obb.axis[2]};

	const FrustumPlanes& frustumPlanes = App->camera->GetFrustumPlanes();
	for (const Plane& plane : frustumPlanes.planes) {
		// check box outside/inside of frustum
		int out = 0;
		for (int i = 0; i < 8; i++) {
			out += (plane.normal.Dot(points[i]) - plane.d > 0 ? 1 : 0);
		}
		if (out == 8) return false;
	}

	// check frustum outside/inside box
	int out;
	out = 0;
	for (int i = 0; i < 8; i++) out += ((frustumPlanes.points[i].x > aabb.MaxX()) ? 1 : 0);
	if (out == 8) return false;
	out = 0;
	for (int i = 0; i < 8; i++) out += ((frustumPlanes.points[i].x < aabb.MinX()) ? 1 : 0);
	if (out == 8) return false;
	out = 0;
	for (int i = 0; i < 8; i++) out += ((frustumPlanes.points[i].y > aabb.MaxY()) ? 1 : 0);
	if (out == 8) return false;
	out = 0;
	for (int i = 0; i < 8; i++) out += ((frustumPlanes.points[i].y < aabb.MinY()) ? 1 : 0);
	if (out == 8) return false;
	out = 0;
	for (int i = 0; i < 8; i++) out += ((frustumPlanes.points[i].z > aabb.MaxZ()) ? 1 : 0);
	if (out == 8) return false;
	out = 0;
	for (int i = 0; i < 8; i++) out += ((frustumPlanes.points[i].z < aabb.MinZ()) ? 1 : 0);
	if (out == 8) return false;

	return true;
}
*/

void ModuleRender::DrawGameObject(GameObject* gameObject) {
	ComponentTransform* transform = gameObject->GetComponent<ComponentTransform>();
	ComponentView<ComponentMeshRenderer> meshes = gameObject->GetComponents<ComponentMeshRenderer>();
	ComponentBoundingBox* boundingBox = gameObject->GetComponent<ComponentBoundingBox>();

	if (boundingBox && drawAllBoundingBoxes && (App->camera->IsEngineCameraActive() || debugMode)) {
		boundingBox->DrawBoundingBox();
	}

	for (ComponentMeshRenderer& mesh : meshes) {
		mesh.Draw(transform->GetGlobalMatrix());

		ResourceMesh* resourceMesh = App->resources->GetResource<ResourceMesh>(mesh.meshId);
		if (resourceMesh != nullptr) {
			culledTriangles += resourceMesh->numIndices / 3;
		}
	}
}

void ModuleRender::DrawGameObjectDepthPrepass(GameObject* gameObject) {
	ComponentView<ComponentMeshRenderer> meshes = gameObject->GetComponents<ComponentMeshRenderer>();
	ComponentTransform* transform = gameObject->GetComponent<ComponentTransform>();
	assert(transform);

	for (ComponentMeshRenderer& mesh : meshes) {
		mesh.DrawDepthPrepass(transform->GetGlobalMatrix());
	}
}

void ModuleRender::DrawGameObjectShadowPass(GameObject* gameObject) {
	ComponentView<ComponentMeshRenderer> meshes = gameObject->GetComponents<ComponentMeshRenderer>();
	ComponentTransform* transform = gameObject->GetComponent<ComponentTransform>();
	assert(transform);

	for (ComponentMeshRenderer& mesh : meshes) {
		mesh.DrawShadow(transform->GetGlobalMatrix());
	}
}

void ModuleRender::RenderUI() {
	if (App->userInterface->IsUsing2D()) {
		SetOrtographicRender();
		App->camera->EnableOrtographic();
	}

	glDisable(GL_DEPTH_TEST); // In order to not clip with Models
	App->userInterface->Render();
	glEnable(GL_DEPTH_TEST);

	if (App->userInterface->IsUsing2D()) {
		App->camera->EnablePerspective();
		SetPerspectiveRender();
	}
}

void ModuleRender::SetOrtographicRender() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, viewportSize.x, viewportSize.y, 0, 1, -1);
	glMatrixMode(GL_MODELVIEW);
}

void ModuleRender::SetPerspectiveRender() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
}

const float2 ModuleRender::GetViewportSize() {
	return viewportSize;
}

bool ModuleRender::ObjectInsideFrustum(GameObject* gameObject) {
	ComponentBoundingBox* boundingBox = gameObject->GetComponent<ComponentBoundingBox>();
	if (boundingBox) {
		return App->camera->GetFrustumPlanes().CheckIfInsideFrustum(boundingBox->GetWorldAABB(), boundingBox->GetWorldOBB());
	}
	return false;
}

void ModuleRender::DrawAnimation(const GameObject* gameObject, bool hasAnimation) {
	for (const GameObject* childen : gameObject->GetChildren()) {
		ComponentTransform* transform = childen->GetComponent<ComponentTransform>();

		dd::point(transform->GetGlobalPosition(), dd::colors::Red, 5);
		dd::line(gameObject->GetComponent<ComponentTransform>()->GetGlobalPosition(), transform->GetGlobalPosition(), dd::colors::Cyan, 0, false);

		DrawAnimation(childen, true);
	}
}