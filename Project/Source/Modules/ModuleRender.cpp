#include "ModuleRender.h"

#include "Globals.h"
#include "Application.h"
#include "Utils/Logging.h"
#include "Components/ComponentMeshRenderer.h"
#include "Components/ComponentBoundingBox.h"
#include "Components/ComponentTransform.h"
#include "Components/ComponentAnimation.h"
#include "Modules/ModuleInput.h"
#include "Modules/ModuleWindow.h"
#include "Modules/ModuleCamera.h"
#include "Modules/ModuleDebugDraw.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModuleScene.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModulePrograms.h"
#include "Modules/ModuleUserInterface.h"
#include "Modules/ModuleTime.h"
#include "Event.h"

#include "Geometry/AABB.h"
#include "Geometry/AABB2D.h"
#include "Geometry/OBB.h"
#include "debugdraw.h"
#include "GL/glew.h"
#include "SDL.h"
#include "Brofiler.h"

#include "Utils/Leaks.h"

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

bool ModuleRender::Init() {
	LOG("Creating Renderer context");

	context = SDL_GL_CreateContext(App->window->window);

	GLenum err = glewInit();
	LOG("Using Glew %s", glewGetString(GLEW_VERSION));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

//#ifdef _DEBUG
//	glEnable(GL_DEBUG_OUTPUT);
//	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
//	glDebugMessageCallback(&OurOpenGLErrorFunction, nullptr);
//	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
//#endif

	glGenFramebuffers(1, &framebuffer);
	glGenRenderbuffers(1, &depthRenderbuffer);
	glGenTextures(1, &renderTexture);

	ViewportResized(200, 200);

	return true;
}

UpdateStatus ModuleRender::PreUpdate() {
	BROFILER_CATEGORY("ModuleRender - PreUpdate", Profiler::Color::Green)

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glViewport(0, 0, viewportWidth, viewportHeight);

	glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	return UpdateStatus::CONTINUE;
}

UpdateStatus ModuleRender::Update() {
	BROFILER_CATEGORY("ModuleRender - Update", Profiler::Color::Green)

	// Draw Skybox as a first element
	DrawSkyBox();

	// Draw the scene
	//PerformanceTimer timer;
	//timer.Start();
	App->camera->CalculateFrustumPlanes();
	Scene* scene = App->scene->scene;
	for (ComponentBoundingBox& boundingBox : scene->boundingBoxComponents) {
		GameObject& gameObject = boundingBox.GetOwner();
		gameObject.flag = false;
		if (gameObject.isInQuadtree) continue;

		const AABB& gameObjectAABB = boundingBox.GetWorldAABB();
		const OBB& gameObjectOBB = boundingBox.GetWorldOBB();
		if (CheckIfInsideFrustum(gameObjectAABB, gameObjectOBB)) {
			DrawGameObject(&gameObject);
		}
	}
	if (scene->quadtree.IsOperative()) {
		DrawSceneRecursive(scene->quadtree.root, scene->quadtree.bounds);
	}
	//LOG("Scene draw: %llu mis", timer.Stop());

	if (App->camera->IsEngineCameraActive()) {
		// Draw Guizmos
		GameObject* selectedGameObject = App->editor->selectedGameObject;
		if (selectedGameObject) selectedGameObject->DrawGizmos();

		// Draw quadtree
		if (drawQuadtree) {
			DrawQuadtreeRecursive(App->scene->scene->quadtree.root, App->scene->scene->quadtree.bounds);
		}
	}

	//Render UI
	RenderUI();

	// Draw debug draw
	App->debugDraw->Draw(App->camera->GetViewMatrix(), App->camera->GetProjectionMatrix(), viewportWidth, viewportHeight);

	// Draw Animations
	for (ComponentAnimation& animationComponent : App->scene->scene->animationComponents) {
		DrawAnimation(animationComponent.GetOwner().GetRootBone());
	}

	return UpdateStatus::CONTINUE;
}

UpdateStatus ModuleRender::PostUpdate() {
	BROFILER_CATEGORY("ModuleRender - PostUpdate", Profiler::Color::Green)

	SDL_GL_SwapWindow(App->window->window);

	return UpdateStatus::CONTINUE;
}

bool ModuleRender::CleanUp() {
	glDeleteTextures(1, &renderTexture);
	glDeleteRenderbuffers(1, &depthRenderbuffer);
	glDeleteFramebuffers(1, &framebuffer);

	return true;
}

void ModuleRender::ViewportResized(int width, int height) {
	viewportWidth = width;
	viewportHeight = height;

	// Framebuffer calculations
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glBindTexture(GL_TEXTURE_2D, renderTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		LOG("ERROR: Framebuffer is not complete!");
	}
}

void ModuleRender::SetVSync(bool vsync) {
	SDL_GL_SetSwapInterval(vsync);
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

void ModuleRender::ReceiveEvent(const Event& ev) {
}

void ModuleRender::DrawSceneRecursive(const Quadtree<GameObject>::Node& node, const AABB2D& aabb) {
	AABB aabb3d = AABB({aabb.minPoint.x, -1000000.0f, aabb.minPoint.y}, {aabb.maxPoint.x, 1000000.0f, aabb.maxPoint.y});
	if (CheckIfInsideFrustum(aabb3d, OBB(aabb3d))) {
		if (node.IsBranch()) {
			vec2d center = aabb.minPoint + (aabb.maxPoint - aabb.minPoint) * 0.5f;

			const Quadtree<GameObject>::Node& topLeft = node.childNodes->nodes[0];
			AABB2D topLeftAABB = {{aabb.minPoint.x, center.y}, {center.x, aabb.maxPoint.y}};
			DrawSceneRecursive(topLeft, topLeftAABB);

			const Quadtree<GameObject>::Node& topRight = node.childNodes->nodes[1];
			AABB2D topRightAABB = {{center.x, center.y}, {aabb.maxPoint.x, aabb.maxPoint.y}};
			DrawSceneRecursive(topRight, topRightAABB);

			const Quadtree<GameObject>::Node& bottomLeft = node.childNodes->nodes[2];
			AABB2D bottomLeftAABB = {{aabb.minPoint.x, aabb.minPoint.y}, {center.x, center.y}};
			DrawSceneRecursive(bottomLeft, bottomLeftAABB);

			const Quadtree<GameObject>::Node& bottomRight = node.childNodes->nodes[3];
			AABB2D bottomRightAABB = {{center.x, aabb.minPoint.y}, {aabb.maxPoint.x, center.y}};
			DrawSceneRecursive(bottomRight, bottomRightAABB);
		} else {
			const Quadtree<GameObject>::Element* element = node.firstElement;
			while (element != nullptr) {
				GameObject* gameObject = element->object;
				if (!gameObject->flag) {
					ComponentBoundingBox* boundingBox = gameObject->GetComponent<ComponentBoundingBox>();
					const AABB& gameObjectAABB = boundingBox->GetWorldAABB();
					const OBB& gameObjectOBB = boundingBox->GetWorldOBB();
					if (CheckIfInsideFrustum(gameObjectAABB, gameObjectOBB)) {
						DrawGameObject(gameObject);
					}

					gameObject->flag = true;
				}
				element = element->next;
			}
		}
	}
}

bool ModuleRender::CheckIfInsideFrustum(const AABB& aabb, const OBB& obb) {
	float3 points[8];
	obb.GetCornerPoints(points);

	const FrustumPlanes& frustumPlanes = App->camera->GetFrustumPlanes();
	for (const Plane& plane : frustumPlanes.planes) {
		// check box outside/inside of frustum
		int out = 0;
		for (int i = 0; i < 8; i++) {
			out += (plane.SignedDistance(points[i]) > 0 ? 1 : 0);
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

void ModuleRender::DrawGameObject(GameObject* gameObject) {
	ComponentTransform* transform = gameObject->GetComponent<ComponentTransform>();
	std::vector<ComponentMeshRenderer*> meshes = gameObject->GetComponents<ComponentMeshRenderer>();
	ComponentBoundingBox* boundingBox = gameObject->GetComponent<ComponentBoundingBox>();

	if (boundingBox && drawAllBoundingBoxes && App->camera->IsEngineCameraActive()) {
		boundingBox->DrawBoundingBox();
	}

	for (ComponentMeshRenderer* mesh : meshes) {
		mesh->Draw(transform->GetGlobalMatrix());
	}
}

void ModuleRender::DrawSkyBox() {
	// TODO: (Texture resource) Make skybox work
	/*
	if (skyboxActive) {
		glDepthFunc(GL_LEQUAL);

		unsigned program = App->programs->skyboxProgram;
		glUseProgram(program);
		glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_TRUE, App->camera->GetViewMatrix().ptr());
		glUniformMatrix4fv(glGetUniformLocation(program, "proj"), 1, GL_TRUE, App->camera->GetProjectionMatrix().ptr());
		glUniform1i(glGetUniformLocation(program, "cubemap"), 0);

		glBindVertexArray(App->scene->skyboxVao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, App->scene->skyboxCubeMap->glTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		glDepthFunc(GL_LESS);
	}
	*/
}

void ModuleRender::RenderUI() {
	if (App->time->IsGameRunning() || App->editor->panelScene.IsUsing2D()) {
		SetOrtographicRender();
		App->camera->EnableOrtographic();
	}

	glDisable(GL_DEPTH_TEST); // In order to not clip with Models
	App->userInterface->Render();
	glEnable(GL_DEPTH_TEST);

	if (App->time->IsGameRunning() || App->editor->panelScene.IsUsing2D()) {
		App->camera->EnablePerspective();
		SetPerspectiveRender();
	}
}

void ModuleRender::SetOrtographicRender() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, viewportWidth, viewportHeight, 0, 1, -1);
	glMatrixMode(GL_MODELVIEW);
}

void ModuleRender::SetPerspectiveRender() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
}
void ModuleRender::DrawAnimation(const GameObject* gameObject, bool hasAnimation) {
	for (const GameObject* childen : gameObject->GetChildren()) {
		ComponentTransform* transform = childen->GetComponent<ComponentTransform>();

		dd::point(transform->GetGlobalPosition(), dd::colors::Red, 5);
		dd::line(gameObject->GetComponent<ComponentTransform>()->GetGlobalPosition(), transform->GetGlobalPosition(), dd::colors::Cyan, 0, false);

		DrawAnimation(childen, true);
	}
}
