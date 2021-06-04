#include "SceneImporter.h"

#include "Application.h"
#include "GameObject.h"
#include "Utils/Logging.h"
#include "Utils/MSTimer.h"
#include "Utils/FileDialog.h"
#include "FileSystem/TextureImporter.h"
#include "Resources/ResourceScene.h"
#include "Components/ComponentTransform.h"
#include "Components/ComponentBoundingBox.h"
#include "Components/ComponentMeshRenderer.h"
#include "Components/ComponentScript.h"
#include "Modules/ModuleFiles.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleCamera.h"
#include "Modules/ModuleScene.h"
#include "Modules/ModuleTime.h"
#include "Modules/ModuleRender.h"
#include "Scripting/Script.h"
#include "ImporterCommon.h"

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/error/en.h"

#include "Utils/Leaks.h"

#define JSON_TAG_ROOT "Root"
#define JSON_TAG_QUADTREE_BOUNDS "QuadtreeBounds"
#define JSON_TAG_QUADTREE_MAX_DEPTH "QuadtreeMaxDepth"
#define JSON_TAG_QUADTREE_ELEMENTS_PER_NODE "QuadtreeElementsPerNode"
#define JSON_TAG_GAME_CAMERA "GameCamera"
#define JSON_TAG_AMBIENTLIGHT "AmbientLight"
#define JSON_TAG_NAVMESH "NavMesh"

bool SceneImporter::ImportScene(const char* filePath, JsonValue jMeta) {
	// Timer to measure importing a scene
	MSTimer timer;
	timer.Start();
	LOG("Importing scene from path: \"%s\".", filePath);

	// Read from file
	Buffer<char> buffer = App->files->Load(filePath);
	if (buffer.Size() == 0) {
		LOG("Error loading scene %s", filePath);
		return false;
	}

	// Parse document from file
	rapidjson::Document document;
	document.ParseInsitu<rapidjson::kParseNanAndInfFlag>(buffer.Data());
	if (document.HasParseError()) {
		LOG("Error parsing JSON: %s (offset: %u)", rapidjson::GetParseError_En(document.GetParseError()), document.GetErrorOffset());
		return false;
	}

	// Write document to buffer
	rapidjson::StringBuffer stringBuffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>, rapidjson::UTF8<>, rapidjson::CrtAllocator, rapidjson::kWriteNanAndInfFlag> writer(stringBuffer);
	document.Accept(writer);

	// Create scene resource
	unsigned resourceIndex = 0;
	std::unique_ptr<ResourceScene> scene = ImporterCommon::CreateResource<ResourceScene>(FileDialog::GetFileName(filePath).c_str(), filePath, jMeta, resourceIndex);

	// Save resource meta file
	bool saved = ImporterCommon::SaveResourceMetaFile(scene.get());
	if (!saved) {
		LOG("Failed to save scene resource meta file.");
		return false;
	}

	// Save to file
	saved = App->files->Save(scene->GetResourceFilePath().c_str(), stringBuffer.GetString(), stringBuffer.GetSize());
	if (!saved) {
		LOG("Failed to save state machine resource file.");
		return false;
	}

	// Send resource creation event
	App->resources->SendCreateResourceEvent(scene);

	unsigned timeMs = timer.Stop();
	LOG("Scene imported in %ums", timeMs);
	return true;
}

void SceneImporter::LoadScene(const char* filePath) {
	// Clear scene
	Scene* scene = App->scene->scene;
	scene->ClearScene();
	scene->sceneLoaded = false;
	App->editor->selectedGameObject = nullptr;

	// Timer to measure loading a scene
	MSTimer timer;
	timer.Start();
	LOG("Loading scene from path: \"%s\".", filePath);

	// Read from file
	Buffer<char> buffer = App->files->Load(filePath);

	if (buffer.Size() == 0) return;

	// Parse document from file
	rapidjson::Document document;
	document.ParseInsitu<rapidjson::kParseNanAndInfFlag>(buffer.Data());
	if (document.HasParseError()) {
		LOG("Error parsing JSON: %s (offset: %u)", rapidjson::GetParseError_En(document.GetParseError()), document.GetErrorOffset());
		return;
	}
	JsonValue jScene(document, document);

	// Load GameObjects
	JsonValue jRoot = jScene[JSON_TAG_ROOT];
	GameObject* root = scene->gameObjects.Obtain(0);
	scene->root = root;
	root->scene = scene;
	root->Load(jRoot);
	root->InitComponents();

	// Quadtree generation
	JsonValue jQuadtreeBounds = jScene[JSON_TAG_QUADTREE_BOUNDS];
	scene->quadtreeBounds = {{jQuadtreeBounds[0], jQuadtreeBounds[1]}, {jQuadtreeBounds[2], jQuadtreeBounds[3]}};
	scene->quadtreeMaxDepth = jScene[JSON_TAG_QUADTREE_MAX_DEPTH];
	scene->quadtreeElementsPerNode = jScene[JSON_TAG_QUADTREE_ELEMENTS_PER_NODE];
	scene->RebuildQuadtree();

	ComponentCamera* gameCamera = scene->GetComponent<ComponentCamera>(jScene[JSON_TAG_GAME_CAMERA]);
	App->camera->ChangeGameCamera(gameCamera, gameCamera != nullptr);
	if(App->time->IsGameRunning()) App->camera->ChangeActiveCamera(gameCamera, gameCamera != nullptr);

	JsonValue ambientLight = jScene[JSON_TAG_AMBIENTLIGHT];
	App->renderer->ambientColor = {ambientLight[0], ambientLight[1], ambientLight[2]};

	// NavMesh
	scene->SetNavMesh(jScene[JSON_TAG_NAVMESH]);

	unsigned timeMs = timer.Stop();
	LOG("Scene loaded in %ums.", timeMs);
}

bool SceneImporter::SaveScene(const char* filePath) {
	// Create document
	rapidjson::Document document;
	document.SetObject();
	JsonValue jScene(document, document);

	// Save scene information
	Scene* scene = App->scene->scene;
	JsonValue jQuadtreeBounds = jScene[JSON_TAG_QUADTREE_BOUNDS];
	jQuadtreeBounds[0] = scene->quadtreeBounds.minPoint.x;
	jQuadtreeBounds[1] = scene->quadtreeBounds.minPoint.y;
	jQuadtreeBounds[2] = scene->quadtreeBounds.maxPoint.x;
	jQuadtreeBounds[3] = scene->quadtreeBounds.maxPoint.y;
	jScene[JSON_TAG_QUADTREE_MAX_DEPTH] = scene->quadtreeMaxDepth;
	jScene[JSON_TAG_QUADTREE_ELEMENTS_PER_NODE] = scene->quadtreeElementsPerNode;

	ComponentCamera* gameCamera = App->camera->GetGameCamera();
	jScene[JSON_TAG_GAME_CAMERA] = gameCamera ? gameCamera->GetID() : 0;

	JsonValue ambientLight = jScene[JSON_TAG_AMBIENTLIGHT];
	ambientLight[0] = App->renderer->ambientColor.x;
	ambientLight[1] = App->renderer->ambientColor.y;
	ambientLight[2] = App->renderer->ambientColor.z;

	// NavMesh
	jScene[JSON_TAG_NAVMESH] = scene->navMeshId;

	// Save GameObjects
	JsonValue jRoot = jScene[JSON_TAG_ROOT];
	scene->root->Save(jRoot);

	// Write document to buffer
	rapidjson::StringBuffer stringBuffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>, rapidjson::UTF8<>, rapidjson::CrtAllocator, rapidjson::kWriteNanAndInfFlag> writer(stringBuffer);
	document.Accept(writer);

	// Save to file
	App->files->Save(filePath, stringBuffer.GetString(), stringBuffer.GetSize());

	return true;
}