#pragma once
#include "Module.h"
#include "Components/Physics/ComponentSphereCollider.h"
#include "Components/Physics/ComponentCapsuleCollider.h"

#include "Math/float4x4.h"
#include "btBulletDynamicsCommon.h"
#include "debugdraw.h"

class DebugDrawer;
class MotionState;

class ModulePhysics : public Module {

public:
	// ------- Core Functions ------ //
	bool Init() override;
	//bool Start();
	UpdateStatus PreUpdate();
	UpdateStatus Update();
	//UpdateStatus PostUpdate();
	bool CleanUp();
	//void ReceiveEvent(TesseractEvent& e);

	// ------ Add/Remove Body ------ //
	void CreateSphereRigidbody(ComponentSphereCollider* sphereCollider);
	void RemoveSphereRigidbody(ComponentSphereCollider* sphereCollider);
	void UpdateSphereRigidbody(ComponentSphereCollider* sphereCollider);

	void CreateCapsuleRigidbody(ComponentCapsuleCollider* capsuleCollider);
	void RemoveCapsuleRigidbody(ComponentCapsuleCollider* capsuleCollider);
	void UpdateCapsuleRigidbody(ComponentCapsuleCollider* capsuleCollider);

	void InitializeRigidBodies();
	void ClearPhysicBodies();
	btRigidBody* AddSphereBody(MotionState* myMotionState, float radius, float mass);
	btRigidBody* AddCapsuleBody(MotionState* myMotionState, float radius, float height, float mass);

	// ----------- Setters --------- //
	void SetGravity(float newGravity);

public:
	float gravity = -9.81f;

private:
	btDiscreteDynamicsWorld* world = nullptr;

	btDefaultCollisionConfiguration* collisionConfiguration = nullptr;
	btCollisionDispatcher* dispatcher = nullptr;
	btBroadphaseInterface* broadPhase = nullptr;
	btSequentialImpulseConstraintSolver* constraintSolver = nullptr;
	
	DebugDrawer* debugDrawer;

	bool debug = true;

	/*p2List<btCollisionShape*> shapes;
	p2List<PhysBody3D*> bodies;
	p2List<btDefaultMotionState*> motions;
	p2List<btTypedConstraint*> constraints;*/
};


class DebugDrawer : public btIDebugDraw {
public:
	DebugDrawer() {}
	void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
	void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);
	void reportErrorWarning(const char* warningString);
	void draw3dText(const btVector3& location, const char* textString);
	void setDebugMode(int debugMode);
	int getDebugMode() const;
	
	DebugDrawModes mode; // How to initialise this enum?
};