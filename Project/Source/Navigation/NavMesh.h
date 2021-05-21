#pragma once

#include "Recast/SampleInterfaces.h"
//#include "Recast/NavMeshTesterTool.h"

/// These are just sample areas to use consistent values across the samples.
/// The use should specify these base on his needs.
enum SamplePolyAreas {
	SAMPLE_POLYAREA_GROUND,
	SAMPLE_POLYAREA_WATER,
	SAMPLE_POLYAREA_ROAD,
	SAMPLE_POLYAREA_DOOR,
	SAMPLE_POLYAREA_GRASS,
	SAMPLE_POLYAREA_JUMP,
};

enum SamplePolyFlags {
	SAMPLE_POLYFLAGS_WALK = 0x01,	  // Ability to walk (ground, grass, road)
	SAMPLE_POLYFLAGS_SWIM = 0x02,	  // Ability to swim (water).
	SAMPLE_POLYFLAGS_DOOR = 0x04,	  // Ability to move through doors.
	SAMPLE_POLYFLAGS_JUMP = 0x08,	  // Ability to jump.
	SAMPLE_POLYFLAGS_DISABLED = 0x10, // Disabled polygon
	SAMPLE_POLYFLAGS_ALL = 0xffff	  // All abilities.
};

class NavMesh {
public:
	NavMesh();
	~NavMesh();

	bool Build();
	void Render();

public:
	enum SamplePartitionType {
		SAMPLE_PARTITION_WATERSHED,
		SAMPLE_PARTITION_MONOTONE,
		SAMPLE_PARTITION_LAYERS,
	};

	enum DrawMode {
		DRAWMODE_NAVMESH,
		DRAWMODE_NAVMESH_TRANS,
		DRAWMODE_NAVMESH_BVTREE,
		DRAWMODE_NAVMESH_NODES,
		DRAWMODE_NAVMESH_INVIS,
		DRAWMODE_MESH,
		DRAWMODE_VOXELS,
		DRAWMODE_VOXELS_WALKABLE,
		DRAWMODE_COMPACT,
		DRAWMODE_COMPACT_DISTANCE,
		DRAWMODE_COMPACT_REGIONS,
		DRAWMODE_REGION_CONNECTIONS,
		DRAWMODE_RAW_CONTOURS,
		DRAWMODE_BOTH_CONTOURS,
		DRAWMODE_CONTOURS,
		DRAWMODE_POLYMESH,
		DRAWMODE_POLYMESH_DETAIL,
		MAX_DRAWMODE
	};

	// AGENT
	float agentHeight = 2.0f;
	float agentRadius = 0.5f;
	float agentMaxClimb = 0.9f; // Step height in Unity
	float agentMaxSlope = 45;

	// RASTERIZATION
	float cellSize = 0.30f;
	float cellHeight = 0.20f;

	// REGION
	int regionMinSize = 8;
	int regionMergeSize = 20;

	// PARTITIONING
	int partitionType = 0;

	// POLYGONIZATION
	int edgeMaxLen = 12;
	float edgeMaxError = 1.3f;
	int vertsPerPoly = 6;

	// DETAIL MESH
	int detailSampleDist = 6;
	int detailSampleMaxError = 1;

	// INTERMEDIATE RESULTS
	bool keepInterResults = false;

	// TILING
	int tileSize = 56;

	// DRAW MODE
	DrawMode drawMode = DRAWMODE_NAVMESH;

private:
	


	BuildContext* ctx;

	class InputGeom* geom;
	class dtNavMesh* navMesh;
	class dtNavMeshQuery* navQuery;
	class dtCrowd* crowd;

	//BuildContext* m_ctx;

	class dtTileCache* tileCache;
	struct LinearAllocator* talloc;
	struct FastLZCompressor* tcomp;
	struct MeshProcess* tmproc;

	int maxTiles;
	int maxPolysPerTile;

	float cacheBuildTimeMs;
	int cacheCompressedSize;
	int cacheRawSize;
	int cacheLayerCount;
	int cacheBuildMemUsage;

	rcHeightfield* solid;
	unsigned char* triareas = nullptr;

	rcCompactHeightfield* chf;
	rcContourSet* cset;
	rcPolyMesh* pmesh;
	rcPolyMeshDetail* dmesh;

	unsigned char navMeshDrawFlags;

	//NavMeshTesterTool* tool;
};
