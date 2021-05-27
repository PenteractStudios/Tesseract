#pragma once

#include "Resources/Resource.h"

#include "Geometry/Triangle.h"
#include "Math/float4x4.h"
#include <string>
#include <vector>

class ResourceMesh : public Resource {
public:
	struct Bone {
		float4x4 transform;
		std::string boneName;
	};

	struct Attach {
		unsigned numBones = 0u;
		unsigned bones[4] = {0u, 0u, 0u, 0u};
		float weights[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	};

public:
	REGISTER_RESOURCE(ResourceMesh, ResourceType::MESH);

	void Load() override;
	void Unload() override;

	std::vector<Triangle> ExtractTriangles(const float4x4& modelMatrix) const;

public:
	unsigned vbo = 0;
	unsigned ebo = 0;
	unsigned vao = 0;
	unsigned numVertices = 0;
	unsigned numIndices = 0;
	unsigned numBones = 0;
	std::vector<ResourceMesh::Bone> bones;		// The bones and it's transform from a Mesh
	std::vector<float> meshVertices;
	std::vector<unsigned> meshIndices;
};