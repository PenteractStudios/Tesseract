#pragma once

class btRigidBody;

class Collider {
public:
	Collider(btRigidBody* body);
	~Collider();

	void Push(float x, float y, float z);
	void GetTransform(float* matrix) const;
	void SetTransform(const float* matrix) const;
	void SetPos(float x, float y, float z);

	virtual void OnCollision() = 0;

private:
	btRigidBody* body;

// TODO: implement events for the collision listeners
};
