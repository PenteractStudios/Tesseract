#pragma once

#include "Utils/Pool.h"

#include "Math/myassert.h"
#include <unordered_map>

template<typename K, typename V>
class PoolMap {
public:
	PoolMap() {}

	PoolMap(unsigned amount) {
		values.Allocate(amount);
	}

	void Allocate(unsigned amount) {
		values.Allocate(amount);
		keyToPointerMap.clear();
	}

	void Clear() {
		values.Clear();
		keyToPointerMap.clear();
	}

	template<typename... VArgs>
	V* Obtain(const K& key, VArgs&&... vArgs) {
		assert(values.Capacity() > 0);								// ERROR: The pool map hasn't been initialized
		assert(keyToPointerMap.find(key) == keyToPointerMap.end()); // ERROR: The given key already exists in the map

		V* value = values.Obtain(std::forward<VArgs>(vArgs)...);
		if (value == nullptr) return nullptr;

		keyToPointerMap.emplace(key, value);
		return value;
	}

	void ChangeKey(const K& oldKey, const K& newKey) {
		auto it = keyToPointerMap.find(oldKey);
		if (it == keyToPointerMap.end()) return;

		V* value = it->second;
		keyToPointerMap.erase(oldKey);
		keyToPointerMap.emplace(newKey, value);
	}

	V* Find(const K& key) const {
		auto it = keyToPointerMap.find(key);
		return it != keyToPointerMap.end() ? it->second : nullptr;
	}

	void Release(const K& key) {
		auto it = keyToPointerMap.find(key);
		if (it == keyToPointerMap.end()) return;

		V* value = it->second;
		keyToPointerMap.erase(key);
		values.Release(value);
	}

	size_t Count() {
		return values.Count();
	}

	// Iteration

	typename Pool<V>::Iterator begin() {
		return values.begin();
	}

	typename Pool<V>::ConstIterator begin() const {
		return values.begin();
	}

	typename Pool<V>::Iterator end() {
		return values.end();
	}

	typename Pool<V>::ConstIterator end() const {
		return values.end();
	}

private:
	Pool<V> values;
	std::unordered_map<K, V*> keyToPointerMap;
};