#pragma once

#include "Component.h"

#include "Geometry/AABB.h"
#include "Geometry/OBB.h"


class ComponentBoundingBox2D : public Component {
public:
	REGISTER_COMPONENT(ComponentBoundingBox2D, ComponentType::BOUNDING_BOX_2D, false);

	void Init() override;
	void Update() override;

	void Save(JsonValue jComponent) const override;
	void Load(JsonValue jComponent) override;
	bool CanBeRemoved() const override;

	//void SetLocalBoundingBox(const AABB2D& boundingBox);
	void SetLocalBoundingBox(const AABB& boundingBox);
	void CalculateWorldBoundingBox(bool force = false);
	void DrawGizmos();
	void Invalidate();
	const AABB& GetWorldAABB();
	const OBB& GetWorldOBB();

private:
	//AABB2D localAABB = {{0, 0}, {0, 0}};
	//AABB2D worldAABB = {{0, 0}, {0, 0}};

	AABB localAABB = {{0, 0, 0}, {0, 0, 0}};
	AABB worldAABB = {{0, 0, 0}, {0, 0, 0}};

	OBB worldOBB = {worldAABB};

	bool dirty = true;	
};