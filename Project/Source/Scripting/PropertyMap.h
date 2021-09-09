#pragma once

#include "Math/float2.h"
#include "Math/float3.h"
#include <variant>
#include <string>
#include <unordered_map>

#define PROPERTY_VARIANT std::variant<bool, char, unsigned char, short, unsigned short, int, unsigned int, long long, unsigned long long, float, double, std::string, float2, float3>

class PropertyMap {
public:
	void Clear();

	template<typename T>
	T Get(const char* name, const T& defaultValue);

	template<typename T>
	void Set(const char* name, const T& value);

private:
	std::unordered_map<std::string, PROPERTY_VARIANT> properties;
};

template<typename T>
inline T PropertyMap::Get(const char* name, const T& defaultValue) {
	auto it = properties.find(name);
	if (it != properties.end()) {
		return std::get<T>(it->second);
	} else {
		return defaultValue;
	}
}

template<typename T>
inline void PropertyMap::Set(const char* name, const T& value) {
	PROPERTY_VARIANT& variant = properties[name];
	variant.emplace<T>(value);
}