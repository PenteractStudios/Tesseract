#pragma once

#include "Globals.h"

template<typename T>
class Buffer {
public:
	Buffer()
		: size(0)
		, data(nullptr) {}

	Buffer(size_t size_)
		: size(size_) {
		data = new T[size];
	}

	Buffer(Buffer<T>&& buffer)
		: size(buffer.size) {
		char* oldData = data;
		data = buffer.data;
		buffer.data = oldData;
	}

	~Buffer() {
		Clear();
	}

	void Allocate(size_t size_) {
		Clear();

		size = size_;
		data = new T[size];
	}

	void Clear() {
		size = 0;
		RELEASE_ARRAY(data);
	}

	T* Data() const {
		return data;
	}

	size_t Size() const {
		return size;
	}

	Buffer<T>& operator=(Buffer<T>&& buffer) {
		size = buffer.size;
		T* oldData = data;
		data = buffer.data;
		buffer.data = oldData;

		return *this;
	}

	T& operator[](unsigned index) const {
		return data[index];
	}

private:
	// Copy constructor and assignment are not allowed
	Buffer(const Buffer<T>&);
	Buffer<T>& operator=(const Buffer<T>&);

private:
	size_t size = 0;
	T* data = nullptr;
};
