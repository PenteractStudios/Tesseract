#pragma once

#include "Components/Component.h"
#include "LinearMath/btMotionState.h"
#include "LinearMath/btTransform.h"

#include "Math/float3.h"

class MotionState : public btMotionState {
public:
	MotionState() {}
	MotionState(Component* componentCollider, float3 centerOffset);
	~MotionState();

	void getWorldTransform(btTransform& centerOfMassWorldTrans) const;
	void setWorldTransform(const btTransform& centerOfMassWorldTrans);

private:
	Component* collider = nullptr;
	bool freezeRotation = false;
	btTransform massCenterOffset = btTransform::getIdentity();
};
