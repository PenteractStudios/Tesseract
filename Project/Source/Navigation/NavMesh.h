#pragma once

#include "Recast/SampleInterfaces.h"
#include "Utils/Buffer.h"

class dtCrowd;
class dtNavMeshQuery;

class NavMesh {
public:
	NavMesh();				// Allocates navQuery data and crowd memory
	~NavMesh();				// Releases memory

	bool Build();			// Generates the navMesh from Scene Meshes (Vertices and triangles of MeshRenderer), and saves the data in navData and navDataSize. Also inits navMesh, navQuery and crowd.
	void DrawGizmos();		// Drawas the Bounding Box and the NavMesh
	void Load(Buffer<char>& buffer);	// Loads NavMesh from buffer and Inits data
	void CleanUp();						// Releases memory

	bool IsGenerated();					// Returns true if navMesh is valid
	dtCrowd* GetCrowd();				// Returns crowd
	dtNavMeshQuery* GetNavMeshQuery();	// Returns navQuery

public:

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

	// NAV MESH BUILD PARAMETERS
	// AGENT
	float agentHeight = 2.0f;
	float agentRadius = 0.5f;
	float agentMaxClimb = 0.9f;
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

	// NAV DATA TO SAVE
	unsigned char* navData = 0;
	int navDataSize = 0;

private:
	void InitCrowd();			// Inits crowd with MAX_AGENTS

private:
	BuildContext* ctx = nullptr;

	class dtNavMesh* navMesh = nullptr;
	class dtNavMeshQuery* navQuery = nullptr;
	class dtCrowd* crowd = nullptr;

	rcHeightfield* solid = nullptr;
	unsigned char* triareas = nullptr;

	rcCompactHeightfield* chf;
	rcContourSet* cset;
	rcPolyMesh* pmesh;
	rcPolyMeshDetail* dmesh;

	unsigned char navMeshDrawFlags = 0;
};
