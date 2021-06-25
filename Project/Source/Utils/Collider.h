#pragma once
#include <typeinfo>

struct Collider
{
	void* collider;
	const std::type_info& tid;
};