#pragma once

#include "Components/Component.h"
#include "LinearMath/btMotionState.h"
#include "LinearMath/btTransform.h"

class MotionState : public btMotionState {
public:
	MotionState(Component* componentCollider);
	~MotionState();

	void getWorldTransform(btTransform& centerOfMassWorldTrans) const;
	void setWorldTransform(const btTransform& centerOfMassWorldTrans);

private:
	Component* collider = nullptr;
	bool freezeRotation = false;
	btTransform massCenterOffset = btTransform::getIdentity();
};
