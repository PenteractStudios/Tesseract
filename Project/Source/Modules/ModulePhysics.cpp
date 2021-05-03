#include "Globals.h"
#include "Application.h"
#include "ModulePhysics.h"
#include "ModuleTime.h"
#include "ModuleInput.h"
#include "ModuleScene.h"
#include "Components/Component.h"
#include "Scene.h"
#include "Utils/MotionState.h"

#include "Utils/MotionState.h"
#include "Utils/Logging.h"

bool ModulePhysics::Init() {
	LOG("Creating Physics environment using Bullet Physics.");

	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	broadPhase = new btDbvtBroadphase();
	constraintSolver = new btSequentialImpulseConstraintSolver();
	debugDrawer = new DebugDrawer();

	world = new btDiscreteDynamicsWorld(dispatcher, broadPhase, constraintSolver, collisionConfiguration);
	world->setDebugDrawer(debugDrawer);
	world->setGravity(btVector3(0.f, gravity, 0.f));

	return true;
}

UpdateStatus ModulePhysics::PreUpdate() {
	if (App->time->IsGameRunning()) {
		world->stepSimulation(App->time->GetDeltaTime(), 15);

		int numManifolds = world->getDispatcher()->getNumManifolds();
		for (int i = 0; i < numManifolds; i++) {
			btPersistentManifold* contactManifold = world->getDispatcher()->getManifoldByIndexInternal(i);
			btCollisionObject* obA = (btCollisionObject*) (contactManifold->getBody0());
			btCollisionObject* obB = (btCollisionObject*) (contactManifold->getBody1());

			int numContacts = contactManifold->getNumContacts();
			if (numContacts > 0) {
				Component* pbodyA = (Component*) obA->getUserPointer();
				Component* pbodyB = (Component*) obB->getUserPointer();
				
				if (pbodyA && pbodyB) {
					switch (pbodyA->GetType()) {
					case ComponentType::SPHERE_COLLIDER:
						((ComponentSphereCollider*) pbodyA)->OnCollision();
						break;
						//case
						//case

					default:
						break;
					}

					switch (pbodyB->GetType()) {
					case ComponentType::SPHERE_COLLIDER:
						((ComponentSphereCollider*) pbodyB)->OnCollision();
						break;
						//case
						//case

					default:
						break;
					}
				}
			}
		}
	}
	return UpdateStatus::CONTINUE;
}

UpdateStatus ModulePhysics::Update() {
	if (App->time->IsGameRunning()) {
		/*if (App->input->GetKey(SDL_SCANCODE_F1) == KS_DOWN) //TODO: DOnt do it by keyboard!!
		debug = !debug;*/

		if (debug == true) {
			world->debugDrawWorld();
		}

		if (App->input->GetKey(SDL_SCANCODE_SPACE) == KS_DOWN) {
			/*AddSphereBody(new MotionState(), 10, 2)
			
			
			
			
			Sphere bullet;
			bullet.color = Green;
			bullet.radius = 1.0f;
			bullet.SetPos(App->camera->Position.x, App->camera->Position.y, App->camera->Position.z);

			App->physics->AddBody(bullet)->Push(-App->camera->Z.x * 20.0f, -App->camera->Z.y * 20.0f, -App->camera->Z.z * 20.0f);*/
		}


	}
	return UpdateStatus::CONTINUE;
}

bool ModulePhysics::CleanUp() {
	// TODO: clean necessary module lists/vectors
	ClearPhysicBodies();

	RELEASE(world);

	RELEASE(debugDrawer);
	RELEASE(constraintSolver);
	RELEASE(broadPhase);
	RELEASE(dispatcher);
	RELEASE(collisionConfiguration);
	// world

	return true;
}

void ModulePhysics::CreateSphereRigidbody(ComponentSphereCollider* sphereCollider) {
	sphereCollider->motionState = MotionState(sphereCollider, sphereCollider->centerOffset);
	sphereCollider->rigidBody = App->physics->AddSphereBody(&sphereCollider->motionState, sphereCollider->radius, sphereCollider->mass);
	sphereCollider->rigidBody->setUserPointer(sphereCollider);
	world->addRigidBody(sphereCollider->rigidBody);
}

void ModulePhysics::RemoveSphereRigidbody(ComponentSphereCollider* sphereCollider) {
	world->removeCollisionObject(sphereCollider->rigidBody);
}

void ModulePhysics::UpdateSphereRigidbody(ComponentSphereCollider* sphereCollider) {
	RemoveSphereRigidbody(sphereCollider);
	CreateSphereRigidbody(sphereCollider);
}

void ModulePhysics::InitializeRigidBodies() {

	// TODO: Remove this hardcode
	// Big plane as ground
	{
		btCollisionShape* colShape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);

		btDefaultMotionState* myMotionState = new btDefaultMotionState();
		btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, myMotionState, colShape);

		btRigidBody* body = new btRigidBody(rbInfo);
		world->addRigidBody(body);
	}

	for (ComponentSphereCollider& sphereCollider : App->scene->scene->sphereColliderComponents) {
		if (!sphereCollider.rigidBody) CreateSphereRigidbody(&sphereCollider);
	}
}

void ModulePhysics::ClearPhysicBodies() {
	for (int i = world->getNumCollisionObjects() - 1; i >= 0; i--) {
		btCollisionObject* obj = world->getCollisionObjectArray()[i];
		world->removeCollisionObject(obj);
	}
}

btRigidBody* ModulePhysics::AddSphereBody(MotionState* myMotionState, float radius, float mass) {
	btCollisionShape* colShape = new btSphereShape(radius);

	btVector3 localInertia(0, 0, 0);
	if (mass != 0.f)
		colShape->calculateLocalInertia(mass, localInertia);

	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
	btRigidBody* body = new btRigidBody(rbInfo);

	return body;
}

void ModulePhysics::SetGravity(float newGravity) {
	world->setGravity(btVector3(0.f, newGravity, 0.f));
}

// TODO: Remove Bullet Debug
// =================== DEBUG CALLBACKS ==========================
void DebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color) {
	dd::line((ddVec3) from, (ddVec3) to, (ddVec3) color); // TODO: Test if this actually works
}

void DebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) {
	dd::point((ddVec3) PointOnB, (ddVec3) color);
}

void DebugDrawer::reportErrorWarning(const char* warningString) {
	LOG("Bullet warning: %s", warningString);
}

void DebugDrawer::draw3dText(const btVector3& location, const char* textString) {
	LOG("Bullet draw text: %s", textString);
}

void DebugDrawer::setDebugMode(int debugMode) {
	mode = (DebugDrawModes) debugMode;
}

int DebugDrawer::getDebugMode() const {
	return mode;
}
