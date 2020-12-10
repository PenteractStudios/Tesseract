#pragma once

#include "Component.h"

enum class MaterialType
{
	STANDARD,
	SPECULAR
};

class ComponentMaterial : public Component
{
public:
	static const ComponentType static_type = ComponentType::MATERIAL;

	ComponentMaterial(GameObject& owner);

public:
	unsigned texture = 0;
	MaterialType material_type = MaterialType::SPECULAR;

	//Specular
	float Kd = 1;
	float Ks = 0;
	int n = 1;


};

