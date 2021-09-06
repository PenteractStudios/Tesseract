#include "ModuleScene.h"

#include "Globals.h"
#include "Application.h"
#include "GameObject.h"
#include "Utils/Logging.h"
#include "Utils/FileDialog.h"
#include "Utils/JsonValue.h"
#include "Importers/SceneImporter.h"
#include "Importers/TextureImporter.h"
#include "Components/Component.h"
#include "Components/ComponentTransform.h"
#include "Components/ComponentLight.h"
#include "Components/ComponentMeshRenderer.h"
#include "Components/ComponentBoundingBox.h"
#include "Components/ComponentCamera.h"
#include "Components/ComponentScript.h"
#include "Components/UI/ComponentCanvas.h"
#include "Components/UI/ComponentCanvasRenderer.h"
#include "Components/UI/ComponentTransform2D.h"
#include "Components/UI/ComponentImage.h"
#include "Components/UI/ComponentProgressBar.h"
#include "Modules/ModuleInput.h"
#include "Modules/ModulePrograms.h"
#include "Modules/ModuleCamera.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModuleFiles.h"
#include "Modules/ModuleRender.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleUserInterface.h"
#include "Modules/ModuleEvents.h"
#include "Modules/ModuleTime.h"
#include "Resources/ResourceTexture.h"
#include "Resources/ResourceSkybox.h"
#include "Resources/ResourceScene.h"
#include "Panels/PanelHierarchy.h"
#include "Scripting/Script.h"

#include "GL/glew.h"
#include "Math/myassert.h"
#include "assimp/cimport.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "Math/float4x4.h"
#include "Geometry/Sphere.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/reader.h"
#include "rapidjson/error/en.h"
#include <string>
#include <Windows.h>
#include <array>
#include <future>

#include "Brofiler.h"

#include "Utils/Leaks.h"

static aiLogStream logStream = {nullptr, nullptr};

static void AssimpLogCallback(const char* message, char* user) {
	std::string messageStr = message;
	std::string finalMessageStr = messageStr.substr(0, messageStr.find_last_of('\n'));
	LOG(finalMessageStr.c_str());
}

bool ModuleScene::Init() {
	scene = new Scene(10000);

#ifdef _DEBUG
	logStream.callback = AssimpLogCallback;
	aiAttachLogStream(&logStream);
#endif

	return true;
}

bool ModuleScene::Start() {
	App->events->AddObserverToEvent(TesseractEventType::GAMEOBJECT_DESTROYED, this);
	App->events->AddObserverToEvent(TesseractEventType::CHANGE_SCENE, this);
	App->events->AddObserverToEvent(TesseractEventType::LOAD_SCENE, this);
	App->events->AddObserverToEvent(TesseractEventType::SAVE_SCENE, this);
	App->events->AddObserverToEvent(TesseractEventType::COMPILATION_FINISHED, this);
	App->events->AddObserverToEvent(TesseractEventType::PRESSED_PLAY, this);

#if !GAME
	App->files->CreateFolder(ASSETS_PATH);
	App->files->CreateFolder(LIBRARY_PATH);
	App->files->CreateFolder(SKYBOX_PATH);
	App->files->CreateFolder(TEXTURES_PATH);
	App->files->CreateFolder(SHADERS_PATH);
	App->files->CreateFolder(SCENES_PATH);
	App->files->CreateFolder(MATERIALS_PATH);
	App->files->CreateFolder(PREFABS_PATH);
	App->files->CreateFolder(NAVMESH_PATH);
#endif

#if GAME
	App->events->AddEvent(TesseractEventType::PRESSED_PLAY);
	ResourceScene* startScene = App->resources->GetResource<ResourceScene>(startSceneId);
	if (startScene != nullptr) {
		SceneImporter::LoadScene(startScene->GetResourceFilePath().c_str());
	}
	if (App->scene->scene->root == nullptr) {
		App->scene->CreateEmptyScene();
	}

	App->time->SetVSync(true);
	App->time->limitFramerate = false;
#else
	CreateEmptyScene();
#endif

	return true;
}

UpdateStatus ModuleScene::Update() {
	BROFILER_CATEGORY("ModuleScene - Update", Profiler::Color::Green)

	if (loadingSceneId) {
		ChangeScene(loadingSceneId);
	}

	// Update GameObjects
	scene->root->Update();

	return UpdateStatus::CONTINUE;
}

bool ModuleScene::CleanUp() {
	RELEASE(scene);

#ifdef _DEBUG
	aiDetachAllLogStreams();
#endif

	return true;
}

void ModuleScene::ReceiveEvent(TesseractEvent& e) {
	switch (e.type) {
	case TesseractEventType::GAMEOBJECT_DESTROYED:
		scene->DestroyGameObject(e.Get<DestroyGameObjectStruct>().gameObject);
		break;
	case TesseractEventType::CHANGE_SCENE: {
		UID sceneId = e.Get<ChangeSceneStruct>().sceneId;
		ResourceScene* sceneResource = App->resources->GetResource<ResourceScene>(sceneId);
		if (sceneResource == nullptr) break;

		RELEASE(scene);
		scene = sceneResource->TransferScene();
		App->resources->DecreaseReferenceCount(sceneId);
		loadingSceneId = 0;

		ComponentCamera* gameCamera = scene->GetComponent<ComponentCamera>(scene->gameCameraId);
		App->camera->ChangeGameCamera(gameCamera, gameCamera != nullptr);

		App->renderer->ambientColor = scene->ambientColor;

		if (App->time->HasGameStarted()) {
			scene->StartScene();
		}
		break;
	}
	case TesseractEventType::LOAD_SCENE: {
		Scene* newScene = SceneImporter::LoadScene(e.Get<LoadSceneStruct>().filePath.c_str());
		if (newScene == nullptr) break;

		RELEASE(scene);
		scene = newScene;
		break;
	}
	case TesseractEventType::SAVE_SCENE:
		SceneImporter::SaveScene(scene, e.Get<SaveSceneStruct>().filePath.c_str());
		break;
	case TesseractEventType::COMPILATION_FINISHED:
		for (ComponentScript& script : scene->scriptComponents) {
			script.CreateScriptInstance();
		}
		break;
	}
}

void ModuleScene::CreateEmptyScene() {
	scene->ClearScene();

	// Create Scene root node
	GameObject* root = scene->CreateGameObject(nullptr, GenerateUID(), "Scene");
	scene->root = root;
	ComponentTransform* sceneTransform = root->CreateComponent<ComponentTransform>();

	// Create Directional Light
	GameObject* dirLight = scene->CreateGameObject(root, GenerateUID(), "Directional Light");
	scene->directionalLight = dirLight;
	ComponentTransform* dirLightTransform = dirLight->CreateComponent<ComponentTransform>();
	dirLightTransform->SetPosition(float3(0, 300, 0));
	dirLightTransform->SetRotation(Quat::FromEulerXYZ(pi / 2, 0.0f, 0.0));
	dirLightTransform->SetScale(float3(1, 1, 1));
	ComponentLight* dirLightLight = dirLight->CreateComponent<ComponentLight>();

	// Create Game Camera
	GameObject* gameCamera = scene->CreateGameObject(root, GenerateUID(), "Game Camera");
	ComponentTransform* gameCameraTransform = gameCamera->CreateComponent<ComponentTransform>();
	gameCameraTransform->SetPosition(float3(2, 3, -5));
	gameCameraTransform->SetRotation(Quat::identity);
	gameCameraTransform->SetScale(float3(1, 1, 1));
	ComponentCamera* gameCameraCamera = gameCamera->CreateComponent<ComponentCamera>();
	ComponentSkyBox* gameCameraSkybox = gameCamera->CreateComponent<ComponentSkyBox>();
	ComponentAudioListener* audioListener = gameCamera->CreateComponent<ComponentAudioListener>();

	root->Init();
}

void ModuleScene::PreloadScene(UID newSceneId) {
	if (loadingSceneId != newSceneId) {
		App->resources->DecreaseReferenceCount(loadingSceneId);
		loadingSceneId = newSceneId;
		App->resources->IncreaseReferenceCount(loadingSceneId);
	}
}

void ModuleScene::ChangeScene(UID newSceneId) {
	if (newSceneId == 0) return;

	shouldChangeScene = true;

	if (loadingSceneId != newSceneId) {
		App->resources->DecreaseReferenceCount(loadingSceneId);
		loadingSceneId = newSceneId;
		App->resources->IncreaseReferenceCount(loadingSceneId);
	}

	ResourceScene* sceneResource = App->resources->GetResource<ResourceScene>(newSceneId);
	if (sceneResource == nullptr) return;

	TesseractEvent e(TesseractEventType::CHANGE_SCENE);
	e.Set<ChangeSceneStruct>(newSceneId);
	App->events->AddEvent(e);

	shouldChangeScene = false;
}

Scene* ModuleScene::GetCurrentScene() {
	return scene;
}

void ModuleScene::LoadScene(const char* filePath) {
	TesseractEvent loadSceneEv(TesseractEventType::LOAD_SCENE);
	loadSceneEv.Set<LoadSceneStruct>(filePath);
	App->events->AddEvent(loadSceneEv);
}

void ModuleScene::SaveScene(const char* filePath) {
	TesseractEvent saveSceneEv(TesseractEventType::SAVE_SCENE);
	saveSceneEv.Set<SaveSceneStruct>(filePath);
	App->events->AddEvent(saveSceneEv);
}

void ModuleScene::DestroyGameObjectDeferred(GameObject* gameObject) {
	if (gameObject == nullptr) return;

	const std::vector<GameObject*>& children = gameObject->GetChildren();
	for (GameObject* child : children) {
		DestroyGameObjectDeferred(child);
	}
	TesseractEvent e(TesseractEventType::GAMEOBJECT_DESTROYED);
	e.Set<DestroyGameObjectStruct>(gameObject);

	App->events->AddEvent(e);
}