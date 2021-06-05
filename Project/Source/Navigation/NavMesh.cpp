#include "NavMesh.h"

#include "Application.h"
#include "Modules/ModuleScene.h"
#include "Modules/ModuleDebugDraw.h"
#include "Modules/ModuleCamera.h"
#include "Scene.h"
#include "Components/ComponentMeshRenderer.h"
#include "Utils/Logging.h"

#include "Recast/Recast.h"
#include "Recast/RecastAlloc.h"
#include "Recast/RecastAssert.h"
#include "Detour/DetourStatus.h"
#include "Detour/DetourNavMesh.h"
#include "Detour/DetourNavMeshBuilder.h"
#include "Detour/DetourNavMeshQuery.h"
#include "Detour/DetourCommon.h"
#include "Detour/InputGeom.h"
#include "Detour/DetourAssert.h"
#include "DetourTileCache/DetourTileCache.h"
#include "DetourTileCache/DetourTileCacheBuilder.h"
#include "DetourCrowd/DetourCrowd.h"
#include "DebugUtils/RecastDebugDraw.h"
#include "DebugUtils/DetourDebugDraw.h"
#include "fastlz/fastlz.h"
#include "GL/glew.h"

#include "Utils/Leaks.h"

#define MAX_AGENTS 128

// This value specifies how many layers (or "floors") each navmesh tile is expected to have.
static const int EXPECTED_LAYERS_PER_TILE = 4;

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

enum SamplePartitionType {
	SAMPLE_PARTITION_WATERSHED,
	SAMPLE_PARTITION_MONOTONE,
	SAMPLE_PARTITION_LAYERS,
};

static const int MAX_LAYERS = 32;

struct TileCacheData {
	unsigned char* data;
	int dataSize;
};

struct FastLZCompressor : public dtTileCacheCompressor {
	virtual int maxCompressedSize(const int bufferSize) {
		return (int) (bufferSize * 1.05f);
	}

	virtual dtStatus compress(const unsigned char* buffer, const int bufferSize, unsigned char* compressed, const int /*maxCompressedSize*/, int* compressedSize) {
		*compressedSize = fastlz_compress((const void* const) buffer, bufferSize, compressed);
		return DT_SUCCESS;
	}

	virtual dtStatus decompress(const unsigned char* compressed, const int compressedSize, unsigned char* buffer, const int maxBufferSize, int* bufferSize) {
		*bufferSize = fastlz_decompress(compressed, compressedSize, buffer, maxBufferSize);
		return *bufferSize < 0 ? DT_FAILURE : DT_SUCCESS;
	}
};

struct LinearAllocator : public dtTileCacheAlloc {
	unsigned char* buffer;
	int capacity;
	int top;
	int high;

	LinearAllocator(const int cap)
		: buffer(0)
		, capacity(0)
		, top(0)
		, high(0) {
		resize(cap);
	}

	~LinearAllocator() {
		dtFree(buffer);
	}

	void resize(const int cap) {
		if (buffer) dtFree(buffer);
		buffer = (unsigned char*) dtAlloc(cap, DT_ALLOC_PERM);
		capacity = cap;
	}

	virtual void reset() {
		high = dtMax(high, top);
		top = 0;
	}

	virtual void* alloc(const int size) {
		if (!buffer)
			return 0;
		if (top + size > capacity)
			return 0;
		unsigned char* mem = &buffer[top];
		top += size;
		return mem;
	}

	virtual void free(void* /*ptr*/) {
		// Empty
	}
};

struct MeshProcess : public dtTileCacheMeshProcess {
	InputGeom* m_geom;

	inline MeshProcess()
		: m_geom(0) {
	}

	inline void init(InputGeom* geom) {
		m_geom = geom;
	}

	virtual void process(struct dtNavMeshCreateParams* params, unsigned char* polyAreas, unsigned short* polyFlags) {
		// Update poly flags from areas.
		for (int i = 0; i < params->polyCount; ++i) {
			if (polyAreas[i] == DT_TILECACHE_WALKABLE_AREA)
				polyAreas[i] = SAMPLE_POLYAREA_GROUND;

			if (polyAreas[i] == SAMPLE_POLYAREA_GROUND || polyAreas[i] == SAMPLE_POLYAREA_GRASS || polyAreas[i] == SAMPLE_POLYAREA_ROAD) {
				polyFlags[i] = SAMPLE_POLYFLAGS_WALK;
			} else if (polyAreas[i] == SAMPLE_POLYAREA_WATER) {
				polyFlags[i] = SAMPLE_POLYFLAGS_SWIM;
			} else if (polyAreas[i] == SAMPLE_POLYAREA_DOOR) {
				polyFlags[i] = SAMPLE_POLYFLAGS_WALK | SAMPLE_POLYFLAGS_DOOR;
			}
		}

		// Pass in off-mesh connections.
		if (m_geom) {
			params->offMeshConVerts = m_geom->getOffMeshConnectionVerts();
			params->offMeshConRad = m_geom->getOffMeshConnectionRads();
			params->offMeshConDir = m_geom->getOffMeshConnectionDirs();
			params->offMeshConAreas = m_geom->getOffMeshConnectionAreas();
			params->offMeshConFlags = m_geom->getOffMeshConnectionFlags();
			params->offMeshConUserID = m_geom->getOffMeshConnectionId();
			params->offMeshConCount = m_geom->getOffMeshConnectionCount();
		}
	}
};

struct RasterizationContext {
	RasterizationContext()
		: solid(0)
		, triareas(0)
		, lset(0)
		, chf(0)
		, ntiles(0) {
		memset(tiles, 0, sizeof(TileCacheData) * MAX_LAYERS);
	}

	~RasterizationContext() {
		rcFreeHeightField(solid);
		delete[] triareas;
		rcFreeHeightfieldLayerSet(lset);
		rcFreeCompactHeightfield(chf);
		for (int i = 0; i < MAX_LAYERS; ++i) {
			dtFree(tiles[i].data);
			tiles[i].data = 0;
		}
	}

	rcHeightfield* solid;
	unsigned char* triareas;
	rcHeightfieldLayerSet* lset;
	rcCompactHeightfield* chf;
	TileCacheData tiles[MAX_LAYERS];
	int ntiles;
};

static int calcLayerBufferSize(const int gridWidth, const int gridHeight) {
	const int headerSize = dtAlign4(sizeof(dtTileCacheLayerHeader));
	const int gridSize = gridWidth * gridHeight;
	return headerSize + gridSize * 4;
}

static int rasterizeTileLayers(float* verts, int nVerts, int nTris, BuildContext* ctx, rcChunkyTriMesh* chunkyMesh, const int tx, const int ty, const rcConfig& cfg, TileCacheData* tiles, const int maxTiles) {
	
	FastLZCompressor comp;
	RasterizationContext rc;

	// Tile bounds.
	const float tcs = cfg.tileSize * cfg.cs;

	rcConfig tcfg;
	memcpy(&tcfg, &cfg, sizeof(tcfg));

	tcfg.bmin[0] = cfg.bmin[0] + tx * tcs;
	tcfg.bmin[1] = cfg.bmin[1];
	tcfg.bmin[2] = cfg.bmin[2] + ty * tcs;
	tcfg.bmax[0] = cfg.bmin[0] + (tx + 1) * tcs;
	tcfg.bmax[1] = cfg.bmax[1];
	tcfg.bmax[2] = cfg.bmin[2] + (ty + 1) * tcs;
	tcfg.bmin[0] -= tcfg.borderSize * tcfg.cs;
	tcfg.bmin[2] -= tcfg.borderSize * tcfg.cs;
	tcfg.bmax[0] += tcfg.borderSize * tcfg.cs;
	tcfg.bmax[2] += tcfg.borderSize * tcfg.cs;

	// Allocate voxel heightfield where we rasterize our input data to.
	rc.solid = rcAllocHeightfield();
	if (!rc.solid) {
		LOG("buildNavigation: Out of memory 'solid'.");
		return 0;
	}
	if (!rcCreateHeightfield(ctx, *rc.solid, tcfg.width, tcfg.height, tcfg.bmin, tcfg.bmax, tcfg.cs, tcfg.ch)) {
		LOG("buildNavigation: Could not create solid heightfield.");
		return 0;
	}

	// Allocate array that can hold triangle flags.
	// If you have multiple meshes you need to process, allocate
	// and array which can hold the max number of triangles you need to process.
	rc.triareas = new unsigned char[nTris];
	if (!rc.triareas) {
		LOG("buildNavigation: Out of memory 'm_triareas' (%d).", nTris);
		return 0;
	}

	float tbmin[2], tbmax[2];
	tbmin[0] = tcfg.bmin[0];
	tbmin[1] = tcfg.bmin[2];
	tbmax[0] = tcfg.bmax[0];
	tbmax[1] = tcfg.bmax[2];
	int cid[512]; // TODO: Make grow when returning too many items.
	const int ncid = rcGetChunksOverlappingRect(chunkyMesh, tbmin, tbmax, cid, 512);
	if (!ncid) {
		return 0; // empty
	}

	for (int i = 0; i < ncid; ++i) {
		const rcChunkyTriMeshNode& node = chunkyMesh->nodes[cid[i]];
		const int* tris = &chunkyMesh->tris[node.i * 3];
		const int ntris = node.n;

		memset(rc.triareas, 0, ntris * sizeof(unsigned char));
		rcMarkWalkableTriangles(ctx, tcfg.walkableSlopeAngle, verts, nVerts, tris, ntris, rc.triareas);

		if (!rcRasterizeTriangles(ctx, verts, nVerts, tris, rc.triareas, ntris, *rc.solid, tcfg.walkableClimb))
			return 0;
	}

	// Once all geometry is rasterized, we do initial pass of filtering to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as filter spans where the character cannot possibly stand.
	rcFilterLowHangingWalkableObstacles(ctx, tcfg.walkableClimb, *rc.solid);
	rcFilterLedgeSpans(ctx, tcfg.walkableHeight, tcfg.walkableClimb, *rc.solid);
	rcFilterWalkableLowHeightSpans(ctx, tcfg.walkableHeight, *rc.solid);

	rc.chf = rcAllocCompactHeightfield();
	if (!rc.chf) {
		LOG("buildNavigation: Out of memory 'chf'.");
		return 0;
	}
	if (!rcBuildCompactHeightfield(ctx, tcfg.walkableHeight, tcfg.walkableClimb, *rc.solid, *rc.chf)) {
		LOG("buildNavigation: Could not build compact data.");
		return 0;
	}

	// Erode the walkable area by agent radius.
	if (!rcErodeWalkableArea(ctx, tcfg.walkableRadius, *rc.chf)) {
		LOG("buildNavigation: Could not erode.");
		return 0;
	}

	//// (Optional) Mark areas.
	//const ConvexVolume* vols = geom->getConvexVolumes();
	//for (int i = 0; i < geom->getConvexVolumeCount(); ++i) {
	//	rcMarkConvexPolyArea(ctx, vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, (unsigned char) vols[i].area, *rc.chf);
	//}

	rc.lset = rcAllocHeightfieldLayerSet();
	if (!rc.lset) {
		LOG("buildNavigation: Out of memory 'lset'.");
		return 0;
	}
	if (!rcBuildHeightfieldLayers(ctx, *rc.chf, tcfg.borderSize, tcfg.walkableHeight, *rc.lset)) {
		LOG("buildNavigation: Could not build heighfield layers.");
		return 0;
	}

	rc.ntiles = 0;
	for (int i = 0; i < rcMin(rc.lset->nlayers, MAX_LAYERS); ++i) {
		TileCacheData* tile = &rc.tiles[rc.ntiles++];
		const rcHeightfieldLayer* layer = &rc.lset->layers[i];

		// Store header
		dtTileCacheLayerHeader header;
		header.magic = DT_TILECACHE_MAGIC;
		header.version = DT_TILECACHE_VERSION;

		// Tile layer location in the navmesh.
		header.tx = tx;
		header.ty = ty;
		header.tlayer = i;
		dtVcopy(header.bmin, layer->bmin);
		dtVcopy(header.bmax, layer->bmax);

		// Tile info.
		header.width = (unsigned char) layer->width;
		header.height = (unsigned char) layer->height;
		header.minx = (unsigned char) layer->minx;
		header.maxx = (unsigned char) layer->maxx;
		header.miny = (unsigned char) layer->miny;
		header.maxy = (unsigned char) layer->maxy;
		header.hmin = (unsigned short) layer->hmin;
		header.hmax = (unsigned short) layer->hmax;

		dtStatus status = dtBuildTileCacheLayer(&comp, &header, layer->heights, layer->areas, layer->cons, &tile->data, &tile->dataSize);
		if (dtStatusFailed(status)) {
			return 0;
		}
	}

	// Transfer ownsership of tile data from build context to the caller.
	int n = 0;
	for (int i = 0; i < rcMin(rc.ntiles, maxTiles); ++i) {
		tiles[n++] = rc.tiles[i];
		rc.tiles[i].data = 0;
		rc.tiles[i].dataSize = 0;
	}

	return n;
}

NavMesh::NavMesh()
{
	navMeshDrawFlags = DU_DRAWNAVMESH_OFFMESHCONS | DU_DRAWNAVMESH_CLOSEDLIST;
	navQuery = dtAllocNavMeshQuery();
	crowd = dtAllocCrowd();

	ctx = new BuildContext();
	talloc = new LinearAllocator(32000);
	tcomp = new FastLZCompressor;
	tmproc = new MeshProcess;
}

NavMesh::~NavMesh() {
	CleanUp();

	dtFreeCrowd(crowd);
	crowd = nullptr;
	dtFreeNavMeshQuery(navQuery);
	navQuery = nullptr;
	delete ctx;
	ctx = nullptr;
}

bool NavMesh::Build() {
	CleanUp();

	std::vector<float> verts = App->scene->scene->GetVertices();
	const int nverts = verts.size();
	std::vector<int> tris = App->scene->scene->GetTriangles();
	const int ntris = tris.size() / 3;

	if (nverts == 0) {
		LOG("Building navigation:");
		LOG("There's no mesh to build");
		return false;
	}

	float bmin[3] = {FLT_MAX, FLT_MAX, FLT_MAX};
	float bmax[3] = {FLT_MIN, FLT_MIN, FLT_MIN};
	for (ComponentBoundingBox boundingBox : App->scene->scene->boundingBoxComponents) {
		AABB currentBB = boundingBox.GetWorldAABB();
		float3 currentBBMin = currentBB.minPoint;
		float3 currentBBMax = currentBB.maxPoint;
		bmin[0] = currentBBMin.x < bmin[0] ? currentBBMin.x : bmin[0];
		bmin[1] = currentBBMin.y < bmin[1] ? currentBBMin.y : bmin[1];
		bmin[2] = currentBBMin.z < bmin[2] ? currentBBMin.z : bmin[2];
		bmax[0] = currentBBMax.x > bmax[0] ? currentBBMax.x : bmax[0];
		bmax[1] = currentBBMax.y > bmax[1] ? currentBBMax.y : bmax[1];
		bmax[2] = currentBBMax.z > bmax[2] ? currentBBMax.z : bmax[2];
	}

	//
	// Step 1. Initialize build config.
	//

	// Init build configuration from GUI
	rcConfig cfg;
	memset(&cfg, 0, sizeof(cfg));
	cfg.cs = cellSize;
	cfg.ch = cellHeight;
	cfg.walkableSlopeAngle = agentMaxSlope;
	cfg.walkableHeight = static_cast<int>(ceilf(agentHeight / cfg.ch));
	cfg.walkableClimb = static_cast<int>(floorf(agentMaxClimb / cfg.ch));
	cfg.walkableRadius = static_cast<int>(ceilf(agentRadius / cfg.cs));
	cfg.maxEdgeLen = static_cast<int>(edgeMaxLen / cellSize);
	cfg.maxSimplificationError = edgeMaxError;
	cfg.minRegionArea = static_cast<int>(rcSqr(regionMinSize));		// Note: area = size*size
	cfg.mergeRegionArea = static_cast<int>(rcSqr(regionMergeSize)); // Note: area = size*size
	cfg.maxVertsPerPoly = static_cast<int>(vertsPerPoly);
	cfg.tileSize = (int) tileSize;
	cfg.borderSize = cfg.walkableRadius + 3; // Reserve enough padding.
	cfg.width = cfg.tileSize + cfg.borderSize * 2;
	cfg.height = cfg.tileSize + cfg.borderSize * 2;
	cfg.detailSampleDist = detailSampleDist < 0.9f ? 0 : cellSize * detailSampleDist;
	cfg.detailSampleMaxError = cellHeight * detailSampleMaxError;

	// Set the area where the navigation will be build.
	// Here the bounds of the input mesh are used, but the
	// area could be specified by an user defined box, etc.
	rcVcopy(cfg.bmin, bmin);
	rcVcopy(cfg.bmax, bmax);
	int gw = 0, gh = 0;
	rcCalcGridSize(bmin, bmax, cellSize, &gw, &gh);
	const int ts = (int) tileSize;
	const int tw = (gw + ts - 1) / ts;
	const int th = (gh + ts - 1) / ts;

	LOG("Building navigation:");
	LOG(" - %d x %d cells", cfg.width, cfg.height);
	LOG(" - %.1fK verts, %.1fK tris", nverts / 1000.0f, ntris / 1000.0f);

	int tileBits = rcMin((int) dtIlog2(dtNextPow2(tw * th * EXPECTED_LAYERS_PER_TILE)), 14);
	if (tileBits > 14) tileBits = 14;
	int polyBits = 22 - tileBits;
	maxTiles = 1 << tileBits;
	maxPolysPerTile = 1 << polyBits;

	// Tile cache params.
	dtTileCacheParams tcparams;
	memset(&tcparams, 0, sizeof(tcparams));
	rcVcopy(tcparams.orig, bmin);
	tcparams.cs = cellSize;
	tcparams.ch = cellHeight;
	tcparams.width = (int) tileSize;
	tcparams.height = (int) tileSize;
	tcparams.walkableHeight = agentHeight;
	tcparams.walkableRadius = agentRadius;
	tcparams.walkableClimb = agentMaxClimb;
	tcparams.maxSimplificationError = edgeMaxError;
	tcparams.maxTiles = tw * th * EXPECTED_LAYERS_PER_TILE;
	tcparams.maxObstacles = 128;

	dtFreeTileCache(tileCache);

	tileCache = dtAllocTileCache();
	if (!tileCache) {
		LOG("buildTiledNavigation: Could not allocate tile cache.");
		return false;
	}
	dtStatus status = tileCache->init(&tcparams, talloc, tcomp, tmproc);
	if (dtStatusFailed(status)) {
		LOG("buildTiledNavigation: Could not init tile cache.");
		return false;
	}

	dtFreeNavMesh(navMesh);

	navMesh = dtAllocNavMesh();
	if (!navMesh) {
		LOG("buildTiledNavigation: Could not allocate navmesh.");
		return false;
	}

	dtNavMeshParams params;
	memset(&params, 0, sizeof(params));
	rcVcopy(params.orig, bmin);
	params.tileWidth = tileSize * cellSize;
	params.tileHeight = tileSize * cellSize;
	params.maxTiles = maxTiles;
	params.maxPolys = maxPolysPerTile;

	status = navMesh->init(&params);
	if (dtStatusFailed(status)) {
		LOG("buildTiledNavigation: Could not init navmesh.");
		return false;
	}

	status = navQuery->init(navMesh, 2048);
	if (dtStatusFailed(status)) {
		LOG("buildTiledNavigation: Could not init Detour navmesh query");
		return false;
	}

	// Preprocess tiles.

	cacheLayerCount = 0;
	cacheCompressedSize = 0;
	cacheRawSize = 0;

	rcChunkyTriMesh* chunkyMesh = new rcChunkyTriMesh;
	if (!chunkyMesh) {
		ctx->log(RC_LOG_ERROR, "buildTiledNavigation: Out of memory 'm_chunkyMesh'.");
		return false;
	}
	if (!rcCreateChunkyTriMesh(&verts[0], &tris[0], ntris, 256, chunkyMesh)) {
		ctx->log(RC_LOG_ERROR, "buildTiledNavigation: Failed to build chunky mesh.");
		return false;
	}	

	for (int y = 0; y < th; ++y) {
		for (int x = 0; x < tw; ++x) {
			TileCacheData tiles[MAX_LAYERS];
			memset(tiles, 0, sizeof(tiles));
			int ntiles = rasterizeTileLayers(&verts[0], nverts, ntris, ctx, chunkyMesh, x, y, cfg, tiles, MAX_LAYERS);

			for (int i = 0; i < ntiles; ++i) {
				TileCacheData* tile = &tiles[i];
				status = tileCache->addTile(tile->data, tile->dataSize, DT_COMPRESSEDTILE_FREE_DATA, 0);
				if (dtStatusFailed(status)) {
					dtFree(tile->data);
					tile->data = 0;
					continue;
				}

				cacheLayerCount++;
				cacheCompressedSize += tile->dataSize;
				cacheRawSize += calcLayerBufferSize(tcparams.width, tcparams.height);
			}
		}
	}

	// Build initial meshes
	for (int y = 0; y < th; ++y)
		for (int x = 0; x < tw; ++x)
			tileCache->buildNavMeshTilesAt(x, y, navMesh);
	ctx->stopTimer(RC_TIMER_TOTAL);

	const dtNavMesh* nav = navMesh;
	int navmeshMemUsage = 0;
	for (int i = 0; i < nav->getMaxTiles(); ++i) {
		const dtMeshTile* tile = nav->getTile(i);
		if (tile->header)
			navmeshMemUsage += tile->dataSize;
	}
	printf("navmeshMemUsage = %.1f kB", navmeshMemUsage / 1024.0f);

	//if (tool)
	//	tool->init(this);
	//initToolStates(this);


	return true;
	// !!!!TODO: code below needs to be refactored and most likely deleted.



	//
	// Step 2. Rasterize input polygon soup.
	//

	// Allocate voxel heightfield where we rasterize our input data to.
	solid = rcAllocHeightfield();
	if (!solid) {
		LOG("buildNavigation: Out of memory 'solid'.");
		return false;
	}
	if (!rcCreateHeightfield(ctx, *solid, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch)) {
		LOG("buildNavigation: Could not create solid heightfield.");
		return false;
	}

	// Allocate array that can hold triangle area types.
	// If you have multiple meshes you need to process, allocate
	// and array which can hold the max number of triangles you need to process.
	triareas = new unsigned char[ntris];
	if (!triareas) {
		LOG("buildNavigation: Out of memory 'triareas' (%d).", ntris);
		return false;
	}

	// Find triangles which are walkable based on their slope and rasterize them.
	// If your input data is multiple meshes, you can transform them here, calculate
	// the are type for each of the meshes and rasterize them.
	memset(triareas, 0, ntris * sizeof(unsigned char));
	rcMarkWalkableTriangles(ctx, cfg.walkableSlopeAngle, &verts[0], nverts, &tris[0], ntris, triareas);
	if (!rcRasterizeTriangles(ctx, &verts[0], nverts, &tris[0], triareas, ntris, *solid, cfg.walkableClimb)) {
		LOG("buildNavigation: Could not rasterize triangles.");
		return false;
	}

	if (!keepInterResults) {
		delete[] triareas;
		triareas = 0;
	}

	//
	// Step 3. Filter walkables surfaces.
	//

	// Once all geoemtry is rasterized, we do initial pass of filtering to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as filter spans where the character cannot possibly stand.
	rcFilterLowHangingWalkableObstacles(ctx, cfg.walkableClimb, *solid);
	rcFilterLedgeSpans(ctx, cfg.walkableHeight, cfg.walkableClimb, *solid);
	rcFilterWalkableLowHeightSpans(ctx, cfg.walkableHeight, *solid);

	//
	// Step 4. Partition walkable surface to simple regions.
	//

	// Compact the heightfield so that it is faster to handle from now on.
	// This will result more cache coherent data as well as the neighbours
	// between walkable cells will be calculated.
	chf = rcAllocCompactHeightfield();
	if (!chf) {
		LOG("buildNavigation: Out of memory 'chf'.");
		return false;
	}
	if (!rcBuildCompactHeightfield(ctx, cfg.walkableHeight, cfg.walkableClimb, *solid, *chf)) {
		LOG("buildNavigation: Could not build compact data.");
		return false;
	}

	if (!keepInterResults) {
		rcFreeHeightField(solid);
		solid = nullptr;
	}

	// Erode the walkable area by agent radius.
	if (!rcErodeWalkableArea(ctx, cfg.walkableRadius, *chf)) {
		LOG("buildNavigation: Could not erode.");
		return false;
	}

	// (Optional) Mark areas.
	//const ConvexVolume* vols = geom->getConvexVolumes();
	//for (int i = 0; i < geom->getConvexVolumeCount(); ++i)
	//	rcMarkConvexPolyArea(ctx, vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, (unsigned char) vols[i].area, *chf);

	// Partition the heightfield so that we can use simple algorithm later to triangulate the walkable areas.
	// There are 3 martitioning methods, each with some pros and cons:
	// 1) Watershed partitioning
	//   - the classic Recast partitioning
	//   - creates the nicest tessellation
	//   - usually slowest
	//   - partitions the heightfield into nice regions without holes or overlaps
	//   - the are some corner cases where this method creates produces holes and overlaps
	//      - holes may appear when a small obstacles is close to large open area (triangulation can handle this)
	//      - overlaps may occur if you have narrow spiral corridors (i.e stairs), this make triangulation to fail
	//   * generally the best choice if you precompute the nacmesh, use this if you have large open areas
	// 2) Monotone partioning
	//   - fastest
	//   - partitions the heightfield into regions without holes and overlaps (guaranteed)
	//   - creates long thin polygons, which sometimes causes paths with detours
	//   * use this if you want fast navmesh generation
	// 3) Layer partitoining
	//   - quite fast
	//   - partitions the heighfield into non-overlapping regions
	//   - relies on the triangulation code to cope with holes (thus slower than monotone partitioning)
	//   - produces better triangles than monotone partitioning
	//   - does not have the corner cases of watershed partitioning
	//   - can be slow and create a bit ugly tessellation (still better than monotone)
	//     if you have large open areas with small obstacles (not a problem if you use tiles)
	//   * good choice to use for tiled navmesh with medium and small sized tiles

	if (partitionType == SAMPLE_PARTITION_WATERSHED) {
		// Prepare for region partitioning, by calculating distance field along the walkable surface.
		if (!rcBuildDistanceField(ctx, *chf)) {
			LOG("buildNavigation: Could not build distance field.");
			return false;
		}

		// Partition the walkable surface into simple regions without holes.
		if (!rcBuildRegions(ctx, *chf, 0, cfg.minRegionArea, cfg.mergeRegionArea)) {
			LOG("buildNavigation: Could not build watershed regions.");
			return false;
		}
	} else if (partitionType == SAMPLE_PARTITION_MONOTONE) {
		// Partition the walkable surface into simple regions without holes.
		// Monotone partitioning does not need distancefield.
		if (!rcBuildRegionsMonotone(ctx, *chf, 0, cfg.minRegionArea, cfg.mergeRegionArea)) {
			LOG("buildNavigation: Could not build monotone regions.");
			return false;
		}
	} else // SAMPLE_PARTITION_LAYERS
	{
		// Partition the walkable surface into simple regions without holes.
		if (!rcBuildLayerRegions(ctx, *chf, 0, cfg.minRegionArea)) {
			LOG("buildNavigation: Could not build layer regions.");
			return false;
		}
	}

	//
	// Step 5. Trace and simplify region contours.
	//

	// Create contours.
	cset = rcAllocContourSet();
	if (!cset) {
		LOG("buildNavigation: Out of memory 'cset'.");
		return false;
	}
	if (!rcBuildContours(ctx, *chf, cfg.maxSimplificationError, cfg.maxEdgeLen, *cset)) {
		LOG("buildNavigation: Could not create contours.");
		return false;
	}

	//
	// Step 6. Build polygons mesh from contours.
	//

	// Build polygon navmesh from the contours.
	pmesh = rcAllocPolyMesh();
	if (!pmesh) {
		LOG("buildNavigation: Out of memory 'pmesh'.");
		return false;
	}
	if (!rcBuildPolyMesh(ctx, *cset, cfg.maxVertsPerPoly, *pmesh)) {
		LOG("buildNavigation: Could not triangulate contours.");
		return false;
	}

	//
	// Step 7. Create detail mesh which allows to access approximate height on each polygon.
	//

	dmesh = rcAllocPolyMeshDetail();
	if (!dmesh) {
		LOG("buildNavigation: Out of memory 'pmdtl'.");
		return false;
	}

	if (!rcBuildPolyMeshDetail(ctx, *pmesh, *chf, cfg.detailSampleDist, cfg.detailSampleMaxError, *dmesh)) {
		LOG("buildNavigation: Could not build detail mesh.");
		return false;
	}

	if (!keepInterResults) {
		rcFreeCompactHeightfield(chf);
		chf = nullptr;
		rcFreeContourSet(cset);
		cset = nullptr;
	}

	// At this point the navigation mesh data is ready, you can access it from pmesh.
	// See duDebugDrawPolyMesh or dtCreateNavMeshData as examples how to access the data.

	//
	// (Optional) Step 8. Create Detour data from Recast poly mesh.
	//

	// The GUI may allow more max points per polygon than Detour can handle.
	// Only build the detour navmesh if we do not exceed the limit.
	if (cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON) {
		navData = nullptr;
		navDataSize = 0;

		// Update poly flags from areas.
		for (int i = 0; i < pmesh->npolys; ++i) {
			if (pmesh->areas[i] == RC_WALKABLE_AREA)
				pmesh->areas[i] = SAMPLE_POLYAREA_GROUND;

			if (pmesh->areas[i] == SAMPLE_POLYAREA_GROUND || pmesh->areas[i] == SAMPLE_POLYAREA_GRASS || pmesh->areas[i] == SAMPLE_POLYAREA_ROAD) {
				pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK;
			} else if (pmesh->areas[i] == SAMPLE_POLYAREA_WATER) {
				pmesh->flags[i] = SAMPLE_POLYFLAGS_SWIM;
			} else if (pmesh->areas[i] == SAMPLE_POLYAREA_DOOR) {
				pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK | SAMPLE_POLYFLAGS_DOOR;
			}
		}

		dtNavMeshCreateParams params;
		memset(&params, 0, sizeof(params));
		params.verts = pmesh->verts;
		params.vertCount = pmesh->nverts;
		params.polys = pmesh->polys;
		params.polyAreas = pmesh->areas;
		params.polyFlags = pmesh->flags;
		params.polyCount = pmesh->npolys;
		params.nvp = pmesh->nvp;
		params.detailMeshes = dmesh->meshes;
		params.detailVerts = dmesh->verts;
		params.detailVertsCount = dmesh->nverts;
		params.detailTris = dmesh->tris;
		params.detailTriCount = dmesh->ntris;
		params.walkableHeight = agentHeight;
		params.walkableRadius = agentRadius;
		params.walkableClimb = agentMaxClimb;
		rcVcopy(params.bmin, pmesh->bmin);
		rcVcopy(params.bmax, pmesh->bmax);
		params.cs = cfg.cs;
		params.ch = cfg.ch;
		params.buildBvTree = true;

		if (!dtCreateNavMeshData(&params, &navData, &navDataSize)) {
			LOG("Could not build Detour navmesh.");
			return false;
		}

		navMesh = dtAllocNavMesh();
		if (!navMesh) {
			dtFree(navData);
			navData = nullptr;
			navDataSize = 0;
			LOG("Could not create Detour navmesh");
			return false;
		}

		dtStatus status;

		status = navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
		if (dtStatusFailed(status)) {
			dtFree(navData);
			navData = nullptr;
			navDataSize = 0;
			LOG("Could not init Detour navmesh");
			return false;
		}

		status = navQuery->init(navMesh, 2048);
		if (dtStatusFailed(status)) {
			LOG("Could not init Detour navmesh query");
			return false;
		}
	}


	// Show performance stats.
	LOG(">> Polymesh: %d vertices  %d polygons", pmesh->nverts, pmesh->npolys);

	InitCrowd();

	return true;
}

void NavMesh::DrawGizmos() {
	if (navData == nullptr) {
		return;
	}

	DebugDrawGL dds;

	glClearColor(.1f, .1f, .1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glLoadMatrixf(App->camera->GetProjectionMatrix().Transposed().ptr());

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLoadMatrixf(App->camera->GetViewMatrix().Transposed().ptr());

	glEnable(GL_FOG);
	glDepthMask(GL_TRUE);

	const float texScale = 1.0f / (cellSize * 10.0f);

	glDisable(GL_FOG);
	glDepthMask(GL_FALSE);

	// Draw bounds
	float bmin[3] = {FLT_MAX, FLT_MAX, FLT_MAX};
	float bmax[3] = {FLT_MIN, FLT_MIN, FLT_MIN};
	for (ComponentBoundingBox boundingBox : App->scene->scene->boundingBoxComponents) {
		AABB currentBB = boundingBox.GetWorldAABB();
		float3 currentBBMin = currentBB.minPoint;
		float3 currentBBMax = currentBB.maxPoint;
		bmin[0] = currentBBMin.x < bmin[0] ? currentBBMin.x : bmin[0];
		bmin[1] = currentBBMin.y < bmin[1] ? currentBBMin.y : bmin[1];
		bmin[2] = currentBBMin.z < bmin[2] ? currentBBMin.z : bmin[2];
		bmax[0] = currentBBMax.x > bmax[0] ? currentBBMax.x : bmax[0];
		bmax[1] = currentBBMax.y > bmax[1] ? currentBBMax.y : bmax[1];
		bmax[2] = currentBBMax.z > bmax[2] ? currentBBMax.z : bmax[2];
	}

	duDebugDrawBoxWire(&dds, bmin[0], bmin[1], bmin[2], bmax[0], bmax[1], bmax[2], duRGBA(255, 255, 255, 128), 1.0f);

	dds.begin(DU_DRAW_POINTS, 5.0f);
	dds.vertex(bmin[0], bmin[1], bmin[2], duRGBA(255, 255, 255, 128));
	dds.end();

	if (navMesh && navQuery && (drawMode == DRAWMODE_NAVMESH || drawMode == DRAWMODE_NAVMESH_TRANS || drawMode == DRAWMODE_NAVMESH_BVTREE || drawMode == DRAWMODE_NAVMESH_NODES || drawMode == DRAWMODE_NAVMESH_INVIS)) {
		if (drawMode != DRAWMODE_NAVMESH_INVIS)
			duDebugDrawNavMeshWithClosedList(&dds, *navMesh, *navQuery, navMeshDrawFlags);
		if (drawMode == DRAWMODE_NAVMESH_BVTREE)
			duDebugDrawNavMeshBVTree(&dds, *navMesh);
		if (drawMode == DRAWMODE_NAVMESH_NODES)
			duDebugDrawNavMeshNodes(&dds, *navQuery);
		duDebugDrawNavMeshPolysWithFlags(&dds, *navMesh, SAMPLE_POLYFLAGS_DISABLED, duRGBA(0, 0, 0, 128));
	}

	glDepthMask(GL_TRUE);

	if (chf && drawMode == DRAWMODE_COMPACT)
		duDebugDrawCompactHeightfieldSolid(&dds, *chf);

	if (chf && drawMode == DRAWMODE_COMPACT_DISTANCE)
		duDebugDrawCompactHeightfieldDistance(&dds, *chf);
	if (chf && drawMode == DRAWMODE_COMPACT_REGIONS)
		duDebugDrawCompactHeightfieldRegions(&dds, *chf);
	if (solid && drawMode == DRAWMODE_VOXELS) {
		glEnable(GL_FOG);
		duDebugDrawHeightfieldSolid(&dds, *solid);
		glDisable(GL_FOG);
	}
	if (solid && drawMode == DRAWMODE_VOXELS_WALKABLE) {
		glEnable(GL_FOG);
		duDebugDrawHeightfieldWalkable(&dds, *solid);
		glDisable(GL_FOG);
	}
	if (cset && drawMode == DRAWMODE_RAW_CONTOURS) {
		glDepthMask(GL_FALSE);
		duDebugDrawRawContours(&dds, *cset);
		glDepthMask(GL_TRUE);
	}
	if (cset && drawMode == DRAWMODE_BOTH_CONTOURS) {
		glDepthMask(GL_FALSE);
		duDebugDrawRawContours(&dds, *cset, 0.5f);
		duDebugDrawContours(&dds, *cset);
		glDepthMask(GL_TRUE);
	}
	if (cset && drawMode == DRAWMODE_CONTOURS) {
		glDepthMask(GL_FALSE);
		duDebugDrawContours(&dds, *cset);
		glDepthMask(GL_TRUE);
	}
	if (chf && cset && drawMode == DRAWMODE_REGION_CONNECTIONS) {
		duDebugDrawCompactHeightfieldRegions(&dds, *chf);

		glDepthMask(GL_FALSE);
		duDebugDrawRegionConnections(&dds, *cset);
		glDepthMask(GL_TRUE);
	}
	if (pmesh && drawMode == DRAWMODE_POLYMESH) {
		glDepthMask(GL_FALSE);
		duDebugDrawPolyMesh(&dds, *pmesh);
		glDepthMask(GL_TRUE);
	}
	if (dmesh && drawMode == DRAWMODE_POLYMESH_DETAIL) {
		glDepthMask(GL_FALSE);
		duDebugDrawPolyMeshDetail(&dds, *dmesh);
		glDepthMask(GL_TRUE);
	}

	glDepthMask(GL_TRUE);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void NavMesh::Load(Buffer<char>& buffer) {
	CleanUp();

	navMesh = dtAllocNavMesh();
	if (!navMesh) {
		LOG("Could not create Detour navmesh");
		return;
	}

	dtStatus status;

	unsigned int bufferSize = buffer.Size();
	char* bufferData = buffer.ObtainData();

	status = navMesh->init(reinterpret_cast<unsigned char*>(bufferData), bufferSize, DT_TILE_FREE_DATA);
	if (dtStatusFailed(status)) {
		LOG("Could not init Detour navmesh");
		return;
	}
	navData = reinterpret_cast<unsigned char*>(bufferData);
	navDataSize = bufferSize;

	status = navQuery->init(navMesh, 2048);
	if (dtStatusFailed(status)) {
		LOG("Could not init Detour navmesh query");
		return;
	}

	InitCrowd();
}

void NavMesh::CleanUp() {
	dtFreeNavMesh(navMesh);
	navMesh = nullptr;
	navData = nullptr;
	navDataSize = 0;
	delete[] triareas;
	triareas = nullptr;
	rcFreeHeightField(solid);
	solid = nullptr;
	rcFreeCompactHeightfield(chf);
	chf = nullptr;
	rcFreeContourSet(cset);
	cset = nullptr;
	rcFreePolyMesh(pmesh);
	pmesh = nullptr;
	rcFreePolyMeshDetail(dmesh);
	dmesh = nullptr;
}

dtCrowd* NavMesh::GetCrowd() {
	return crowd;
}

bool NavMesh::IsGenerated() {
	return navData != nullptr;
}

dtNavMeshQuery* NavMesh::GetNavMeshQuery() {
	return navQuery;
}

void NavMesh::InitCrowd() {
	crowd->init(MAX_AGENTS, agentRadius, navMesh);

	// Make polygons with 'disabled' flag invalid.
	crowd->getEditableFilter(0)->setExcludeFlags(SAMPLE_POLYFLAGS_DISABLED);

	// Setup local avoidance params to different qualities.
	dtObstacleAvoidanceParams params;
	// Use mostly default settings, copy from dtCrowd.
	memcpy(&params, crowd->getObstacleAvoidanceParams(0), sizeof(dtObstacleAvoidanceParams));

	// Low (11)
	params.velBias = 0.5f;
	params.adaptiveDivs = 5;
	params.adaptiveRings = 2;
	params.adaptiveDepth = 1;
	crowd->setObstacleAvoidanceParams(0, &params);

	// Medium (22)
	params.velBias = 0.5f;
	params.adaptiveDivs = 5;
	params.adaptiveRings = 2;
	params.adaptiveDepth = 2;
	crowd->setObstacleAvoidanceParams(1, &params);

	// Good (45)
	params.velBias = 0.5f;
	params.adaptiveDivs = 7;
	params.adaptiveRings = 2;
	params.adaptiveDepth = 3;
	crowd->setObstacleAvoidanceParams(2, &params);

	// High (66)
	params.velBias = 0.5f;
	params.adaptiveDivs = 7;
	params.adaptiveRings = 3;
	params.adaptiveDepth = 3;

	crowd->setObstacleAvoidanceParams(3, &params);
}
