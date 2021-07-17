#include "Scene.h"

#include "GameObject.h"
#include "Application.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModulePhysics.h"
#include "Modules/ModuleTime.h"
#include "Resources/ResourceMesh.h"
#include "Utils/Logging.h"

#include "Utils/Leaks.h"

Scene::Scene(unsigned numGameObjects) {
	gameObjects.Allocate(numGameObjects);

	transformComponents.Allocate(numGameObjects);
	meshRendererComponents.Allocate(numGameObjects);
	boundingBoxComponents.Allocate(numGameObjects);
	cameraComponents.Allocate(numGameObjects);
	lightComponents.Allocate(numGameObjects);
	canvasComponents.Allocate(numGameObjects);
	canvasRendererComponents.Allocate(numGameObjects);
	imageComponents.Allocate(numGameObjects);
	transform2DComponents.Allocate(numGameObjects);
	boundingBox2DComponents.Allocate(numGameObjects);
	eventSystemComponents.Allocate(numGameObjects);
	toggleComponents.Allocate(numGameObjects);
	textComponents.Allocate(numGameObjects);
	buttonComponents.Allocate(numGameObjects);
	selectableComponents.Allocate(numGameObjects);
	sliderComponents.Allocate(numGameObjects);
	skyboxComponents.Allocate(numGameObjects);
	scriptComponents.Allocate(numGameObjects);
	animationComponents.Allocate(numGameObjects);
	particleComponents.Allocate(numGameObjects);
	trailComponents.Allocate(numGameObjects);
	audioSourceComponents.Allocate(numGameObjects);
	audioListenerComponents.Allocate(numGameObjects);
	progressbarsComponents.Allocate(numGameObjects);
	billboardComponents.Allocate(numGameObjects);
	sphereColliderComponents.Allocate(numGameObjects);
	boxColliderComponents.Allocate(numGameObjects);
	capsuleColliderComponents.Allocate(numGameObjects);
	agentComponents.Allocate(numGameObjects);
	obstacleComponents.Allocate(numGameObjects);
}

void Scene::ClearScene() {
	DestroyGameObject(root);
	root = nullptr;
	quadtree.Clear();
	SetNavMesh(0);

	assert(gameObjects.Count() == 0); // There should be no GameObjects outside the scene hierarchy
	gameObjects.Clear();			  // This looks redundant, but it resets the free list so that GameObject order is mantained when saving/loading
}

void Scene::RebuildQuadtree() {
	quadtree.Initialize(quadtreeBounds, quadtreeMaxDepth, quadtreeElementsPerNode);
	for (ComponentBoundingBox& boundingBox : boundingBoxComponents) {
		GameObject& gameObject = boundingBox.GetOwner();
		if (gameObject.IsStatic()) {
			boundingBox.CalculateWorldBoundingBox();
			const AABB& worldAABB = boundingBox.GetWorldAABB();
			quadtree.Add(&gameObject, AABB2D(worldAABB.minPoint.xz(), worldAABB.maxPoint.xz()));
			gameObject.isInQuadtree = true;
		}
	}
	quadtree.Optimize();
}

void Scene::ClearQuadtree() {
	quadtree.Clear();
	for (GameObject& gameObject : gameObjects) {
		gameObject.isInQuadtree = false;
	}
}

GameObject* Scene::CreateGameObject(GameObject* parent, UID id, const char* name) {
	GameObject* gameObject = gameObjects.Obtain(id);
	gameObject->scene = this;
	gameObject->id = id;
	gameObject->name = name;
	gameObject->SetParent(parent);

	return gameObject;
}

void Scene::DestroyGameObject(GameObject* gameObject) {
	if (gameObject == nullptr) return;

	// If the removed GameObject is the directionalLight of the scene, set it to nullptr
	if (gameObject == directionalLight) directionalLight = nullptr;

	// We need a copy because we are invalidating the iterator by removing GameObjects
	std::vector<GameObject*> children = gameObject->GetChildren();
	for (GameObject* child : children) {
		DestroyGameObject(child);
	}

	if (gameObject->isInQuadtree) {
		quadtree.Remove(gameObject);
	}
	
	bool selected = App->editor->selectedGameObject == gameObject;
	if (selected) App->editor->selectedGameObject = nullptr;

	gameObject->RemoveAllComponents();
	gameObject->SetParent(nullptr);
	gameObjects.Release(gameObject->GetID());
}

GameObject* Scene::GetGameObject(UID id) const {
	return gameObjects.Find(id);
}

Component* Scene::GetComponentByTypeAndId(ComponentType type, UID componentId) {
	switch (type) {
	case ComponentType::TRANSFORM:
		return transformComponents.Find(componentId);
	case ComponentType::MESH_RENDERER:
		return meshRendererComponents.Find(componentId);
	case ComponentType::BOUNDING_BOX:
		return boundingBoxComponents.Find(componentId);
	case ComponentType::CAMERA:
		return cameraComponents.Find(componentId);
	case ComponentType::LIGHT:
		return lightComponents.Find(componentId);
	case ComponentType::CANVAS:
		return canvasComponents.Find(componentId);
	case ComponentType::CANVASRENDERER:
		return canvasRendererComponents.Find(componentId);
	case ComponentType::IMAGE:
		return imageComponents.Find(componentId);
	case ComponentType::TRANSFORM2D:
		return transform2DComponents.Find(componentId);
	case ComponentType::BUTTON:
		return buttonComponents.Find(componentId);
	case ComponentType::EVENT_SYSTEM:
		return eventSystemComponents.Find(componentId);
	case ComponentType::BOUNDING_BOX_2D:
		return boundingBox2DComponents.Find(componentId);
	case ComponentType::TOGGLE:
		return toggleComponents.Find(componentId);
	case ComponentType::TEXT:
		return textComponents.Find(componentId);
	case ComponentType::SELECTABLE:
		return selectableComponents.Find(componentId);
	case ComponentType::SLIDER:
		return sliderComponents.Find(componentId);
	case ComponentType::SKYBOX:
		return skyboxComponents.Find(componentId);
	case ComponentType::ANIMATION:
		return animationComponents.Find(componentId);
	case ComponentType::SCRIPT:
		return scriptComponents.Find(componentId);
	case ComponentType::PARTICLE:
		return particleComponents.Find(componentId);
	case ComponentType::TRAIL:
		return trailComponents.Find(componentId);
	case ComponentType::BILLBOARD:
		return billboardComponents.Find(componentId);
	case ComponentType::AUDIO_SOURCE:
		return audioSourceComponents.Find(componentId);
	case ComponentType::AUDIO_LISTENER:
		return audioListenerComponents.Find(componentId);
	case ComponentType::PROGRESS_BAR:
		return progressbarsComponents.Find(componentId);
	case ComponentType::SPHERE_COLLIDER:
		return sphereColliderComponents.Find(componentId);
	case ComponentType::BOX_COLLIDER:
		return boxColliderComponents.Find(componentId);
	case ComponentType::CAPSULE_COLLIDER:
		return capsuleColliderComponents.Find(componentId);
	case ComponentType::AGENT:
		return agentComponents.Find(componentId);
	case ComponentType::OBSTACLE:
		return obstacleComponents.Find(componentId);
	default:
		LOG("Component of type %i hasn't been registered in Scene::GetComponentByTypeAndId.", (unsigned) type);
		assert(false);
		return nullptr;
	}
}

Component* Scene::CreateComponentByTypeAndId(GameObject* owner, ComponentType type, UID componentId) {
	switch (type) {
	case ComponentType::TRANSFORM:
		return transformComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::MESH_RENDERER:
		return meshRendererComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::BOUNDING_BOX:
		return boundingBoxComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::CAMERA:
		return cameraComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::LIGHT:
		return lightComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::CANVAS:
		return canvasComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::CANVASRENDERER:
		return canvasRendererComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::IMAGE:
		return imageComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::TRANSFORM2D:
		return transform2DComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::BUTTON:
		return buttonComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::EVENT_SYSTEM:
		return eventSystemComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::BOUNDING_BOX_2D:
		return boundingBox2DComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::TOGGLE:
		return toggleComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::TEXT:
		return textComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::SELECTABLE:
		return selectableComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::SLIDER:
		return sliderComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::SKYBOX:
		return skyboxComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::ANIMATION:
		return animationComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::SCRIPT:
		return scriptComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::PARTICLE:
		return particleComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::TRAIL:
		return trailComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::BILLBOARD:
		return billboardComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::AUDIO_SOURCE:
		return audioSourceComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::AUDIO_LISTENER:
		return audioListenerComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::PROGRESS_BAR:
		return progressbarsComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::SPHERE_COLLIDER:
		return sphereColliderComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::BOX_COLLIDER:
		return boxColliderComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::CAPSULE_COLLIDER:
		return capsuleColliderComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::AGENT:
		return agentComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	case ComponentType::OBSTACLE:
		return obstacleComponents.Obtain(componentId, owner, componentId, owner->IsActive());
	default:
		LOG("Component of type %i hasn't been registered in Scene::CreateComponentByTypeAndId.", (unsigned) type);
		assert(false);
		return nullptr;
	}
}

void Scene::RemoveComponentByTypeAndId(ComponentType type, UID componentId) {
	switch (type) {
	case ComponentType::TRANSFORM:
		transformComponents.Release(componentId);
		break;
	case ComponentType::MESH_RENDERER:
		meshRendererComponents.Release(componentId);
		break;
	case ComponentType::BOUNDING_BOX:
		boundingBoxComponents.Release(componentId);
		break;
	case ComponentType::CAMERA:
		cameraComponents.Release(componentId);
		break;
	case ComponentType::LIGHT:
		lightComponents.Release(componentId);
		break;
	case ComponentType::CANVAS:
		canvasComponents.Release(componentId);
		break;
	case ComponentType::CANVASRENDERER:
		canvasRendererComponents.Release(componentId);
		break;
	case ComponentType::IMAGE:
		imageComponents.Release(componentId);
		break;
	case ComponentType::TRANSFORM2D:
		transform2DComponents.Release(componentId);
		break;
	case ComponentType::BUTTON:
		buttonComponents.Release(componentId);
		break;
	case ComponentType::EVENT_SYSTEM:
		eventSystemComponents.Release(componentId);
		break;
	case ComponentType::BOUNDING_BOX_2D:
		boundingBox2DComponents.Release(componentId);
		break;
	case ComponentType::TOGGLE:
		toggleComponents.Release(componentId);
		break;
	case ComponentType::TEXT:
		textComponents.Release(componentId);
		break;
	case ComponentType::SELECTABLE:
		selectableComponents.Release(componentId);
		break;
	case ComponentType::SLIDER:
		sliderComponents.Release(componentId);
		break;
	case ComponentType::SKYBOX:
		skyboxComponents.Release(componentId);
		break;
	case ComponentType::ANIMATION:
		animationComponents.Release(componentId);
		break;
	case ComponentType::SCRIPT:
		scriptComponents.Release(componentId);
		break;
	case ComponentType::PARTICLE:
		for (ComponentParticleSystem& ps : particleComponents) {
			ps.DestroyParticlesColliders();
		}
		particleComponents.Release(componentId);
		break;
	case ComponentType::TRAIL:
		trailComponents.Release(componentId);
		break;
	case ComponentType::BILLBOARD:
		billboardComponents.Release(componentId);
		break;
	case ComponentType::AUDIO_SOURCE:
		audioSourceComponents.Release(componentId);
		break;
	case ComponentType::AUDIO_LISTENER:
		audioListenerComponents.Release(componentId);
		break;
	case ComponentType::PROGRESS_BAR:
		progressbarsComponents.Release(componentId);
		break;
	case ComponentType::SPHERE_COLLIDER:
		if (App->time->IsGameRunning()) App->physics->RemoveSphereRigidbody(sphereColliderComponents.Find(componentId));
		sphereColliderComponents.Release(componentId);
		break;
	case ComponentType::BOX_COLLIDER:
		if (App->time->IsGameRunning()) App->physics->RemoveBoxRigidbody(boxColliderComponents.Find(componentId));
		boxColliderComponents.Release(componentId);
		break;
	case ComponentType::CAPSULE_COLLIDER:
		if (App->time->IsGameRunning()) App->physics->RemoveCapsuleRigidbody(capsuleColliderComponents.Find(componentId));
		capsuleColliderComponents.Release(componentId);
		break;
	case ComponentType::AGENT:
		agentComponents.Release(componentId);
		break;
	case ComponentType::OBSTACLE:
		obstacleComponents.Release(componentId);
		break;
	default:
		LOG("Component of type %i hasn't been registered in Scene::RemoveComponentByTypeAndId.", (unsigned) type);
		assert(false);
		break;
	}
}

int Scene::GetTotalTriangles() const {
	int triangles = 0;
	for (const ComponentMeshRenderer& meshComponent : meshRendererComponents) {
		ResourceMesh* mesh = App->resources->GetResource<ResourceMesh>(meshComponent.meshId);
		if (mesh != nullptr) {
			triangles += mesh->numIndices / 3;
		}
	}
	return triangles;
}

std::vector<float> Scene::GetVertices() {
	std::vector<float> result;

	for (ComponentMeshRenderer& meshRenderer : meshRendererComponents) {
		ResourceMesh* mesh = App->resources->GetResource<ResourceMesh>(meshRenderer.meshId);
		ComponentTransform* transform = meshRenderer.GetOwner().GetComponent<ComponentTransform>();
		if (mesh != nullptr && transform->GetOwner().IsStatic()) {
			for (size_t i = 0; i < mesh->meshVertices.size(); i += 3) {
				float4 transformedVertex = transform->GetGlobalMatrix() * float4(mesh->meshVertices[i], mesh->meshVertices[i + 1], mesh->meshVertices[i + 2], 1);
				result.push_back(transformedVertex.x);
				result.push_back(transformedVertex.y);
				result.push_back(transformedVertex.z);
			}
		}
	}

	return result;
}

std::vector<int> Scene::GetTriangles() {
	int triangles = 0;
	std::vector<int> maxVertMesh;
	maxVertMesh.push_back(0);
	for (ComponentMeshRenderer& meshRenderer : meshRendererComponents) {
		ResourceMesh* mesh = App->resources->GetResource<ResourceMesh>(meshRenderer.meshId);
		if (mesh != nullptr && meshRenderer.GetOwner().IsStatic()) {
			triangles += mesh->numIndices / 3;
			maxVertMesh.push_back(mesh->numVertices);
		}
	}
	std::vector<int> result(triangles * 3);

	int currentGlobalTri = 0;
	int vertOverload = 0;
	int i = 0;

	for (ComponentMeshRenderer& meshRenderer : meshRendererComponents) {
		ResourceMesh* mesh = App->resources->GetResource<ResourceMesh>(meshRenderer.meshId);
		if (mesh != nullptr && meshRenderer.GetOwner().IsStatic()) {
			vertOverload += maxVertMesh[i];
			for (unsigned j = 0; j < mesh->meshIndices.size(); j += 3) {
				result[currentGlobalTri] = mesh->meshIndices[j] + vertOverload;
				result[currentGlobalTri + 1] = mesh->meshIndices[j + 1] + vertOverload;
				result[currentGlobalTri + 2] = mesh->meshIndices[j + 2] + vertOverload;
				currentGlobalTri += 3;
			}
			i++;
		}
	}

	return result;
}

std::vector<float> Scene::GetNormals() {
	std::vector<float> result;

	for (ComponentMeshRenderer& meshRenderer : meshRendererComponents) {
		ResourceMesh* mesh = App->resources->GetResource<ResourceMesh>(meshRenderer.meshId);
		ComponentTransform* transform = meshRenderer.GetOwner().GetComponent<ComponentTransform>();
		if (mesh != nullptr && transform->GetOwner().IsStatic()) {
			for (size_t i = 0; i < mesh->meshNormals.size(); i += 3) {
				float4 transformedVertex = transform->GetGlobalMatrix() * float4(mesh->meshNormals[i], mesh->meshNormals[i + 1], mesh->meshNormals[i + 2], 1);
				result.push_back(transformedVertex.x);
				result.push_back(transformedVertex.y);
				result.push_back(transformedVertex.z);
			}
		}
	}

	return result;
}

std::vector<GameObject*> Scene::GetCulledMeshes(const FrustumPlanes& planes, const int mask) {
	std::vector<GameObject*> meshes;

	for (ComponentMeshRenderer componentMR : meshRendererComponents) {

		GameObject *go = &componentMR.GetOwner();

		Mask& maskGo = go->GetMask();

		if ((maskGo.bitMask & mask) != 0) {
			ComponentBoundingBox* boundingBox = go->GetComponent<ComponentBoundingBox>();
			if (boundingBox) {
				if (planes.CheckIfInsideFrustumPlanes(boundingBox->GetWorldAABB(), boundingBox->GetWorldOBB())) {
					meshes.push_back(go);
				}
			}
		}

	}

	return meshes;
}

void Scene::SetNavMesh(UID navMesh) {
	if (navMeshId != 0) {
		App->resources->DecreaseReferenceCount(navMeshId);
	}

	navMeshId = navMesh;

	if (navMesh != 0) {
		App->resources->IncreaseReferenceCount(navMesh);
	}
}

UID Scene::GetNavMesh() {
	return navMeshId;
}
