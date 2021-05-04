#pragma once

class NavMesh {
public:
	NavMesh();

	bool Build();

public:
	enum SamplePartitionType {
		SAMPLE_PARTITION_WATERSHED,
		SAMPLE_PARTITION_MONOTONE,
		SAMPLE_PARTITION_LAYERS,
	};

	// AGENT
	float agentHeight = 2.0f;
	float agentRadius = 0.5f;
	float agentMaxClimb = 0.4f; // Step height in Unity
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

};
