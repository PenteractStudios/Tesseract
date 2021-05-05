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

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/error/en.h"

#include "Utils/Leaks.h"

#define JSON_TAG_RESOURCES "Resources"
#define JSON_TAG_TYPE "Type"
#define JSON_TAG_ID "Id"
#define JSON_TAG_ROOT "Root"
#define JSON_TAG_QUADTREE_BOUNDS "QuadtreeBounds"
#define JSON_TAG_QUADTREE_MAX_DEPTH "QuadtreeMaxDepth"
#define JSON_TAG_QUADTREE_ELEMENTS_PER_NODE "QuadtreeElementsPerNode"
#define JSON_TAG_GAME_CAMERA "GameCamera"
#define JSON_TAG_AMBIENTLIGHT "AmbientLight"

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
	JsonValue jResources = jMeta[JSON_TAG_RESOURCES];
	JsonValue jResource = jResources[0];
	UID id = jResource[JSON_TAG_ID];
	ResourceScene* scene = App->resources->CreateResource<ResourceScene>(filePath, id ? id : GenerateUID());

	// Add resource to meta file
	jResource[JSON_TAG_TYPE] = GetResourceTypeName(scene->GetType());
	jResource[JSON_TAG_ID] = scene->GetId();

	// Save to file
	App->files->Save(scene->GetResourceFilePath().c_str(), stringBuffer.GetString(), stringBuffer.GetSize());

	unsigned timeMs = timer.Stop();
	LOG("Scene imported in %ums", timeMs);
	return true;
}

void SceneImporter::LoadScene(const char* filePath) {
	// Clear scene
	Scene* scene = App->scene->scene;
	scene->ClearScene();
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

	UID gameCameraID = jScene[JSON_TAG_GAME_CAMERA];
	App->camera->ChangeGameCamera(App->scene->scene->GetComponent<ComponentCamera>(gameCameraID), true);

	JsonValue ambientLight = jScene[JSON_TAG_AMBIENTLIGHT];
	App->renderer->ambientColor = {ambientLight[0], ambientLight[1] ,ambientLight[2]};

	if (App->time->IsGameRunning()) {
		for (ComponentScript& script : scene->scriptComponents) {
			script.OnStart();
		}
	}

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

	if (App->camera->GetGameCamera()) {
		jScene[JSON_TAG_GAME_CAMERA] = App->camera->GetGameCamera()->GetID();
	}

	JsonValue ambientLight = jScene[JSON_TAG_AMBIENTLIGHT];
	ambientLight[0] = App->renderer->ambientColor.x;
	ambientLight[1] = App->renderer->ambientColor.y;
	ambientLight[2] = App->renderer->ambientColor.z;

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