#pragma once
#include "Module.h"

#include "debugdraw.h"
#include "btBulletDynamicsCommon.h"

class DebugDrawer;

class ModulePhysics : public Module {

public:
	// ------- Core Functions ------ //
	bool Init() override;
	//bool Start();
	UpdateStatus PreUpdate();
	UpdateStatus Update();
	UpdateStatus PostUpdate();
	bool CleanUp();
	//void ReceiveEvent(TesseractEvent& e);

public:
	float gravity = 9.81f;

private:
	btDiscreteDynamicsWorld* world = nullptr;

	btDefaultCollisionConfiguration* collisionConfiguration = nullptr;
	btCollisionDispatcher* dispatcher = nullptr;
	btBroadphaseInterface* broadPhase = nullptr;
	btSequentialImpulseConstraintSolver* constraintSolver = nullptr;
	
	DebugDrawer* debugDrawer;

	bool debug = false;

	/*p2List<btCollisionShape*> shapes;
	p2List<PhysBody3D*> bodies;
	p2List<btDefaultMotionState*> motions;
	p2List<btTypedConstraint*> constraints;*/
};


class DebugDrawer : public btIDebugDraw {
public:
	void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
	void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);
	void reportErrorWarning(const char* warningString);
	void draw3dText(const btVector3& location, const char* textString);
	void setDebugMode(int debugMode);
	int getDebugMode() const;
	
	DebugDrawModes mode; // How to initialise this enum?
};