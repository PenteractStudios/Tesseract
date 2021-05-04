#include "NavMesh.h"

//#include "Recast/Recast.h"
////#include "Detour/DetourStatus.h"
//#include "Detour/DetourNavMesh.h"
//#include "DetourTileCache/DetourTileCache.h"


#include "Utils/Logging.h"
#include "Utils/Leaks.h"

// This value specifies how many layers (or "floors") each navmesh tile is expected to have.
static const int EXPECTED_LAYERS_PER_TILE = 4;


NavMesh::NavMesh() {
}

//bool NavMesh::Build() {
//	dtStatus status;
//
//	if (!geom || !geom->getMesh()) {
//		LOG("buildTiledNavigation: No vertices and triangles.");
//		return false;
//	}
//
//	tmproc->init(geom);
//
//	// Init cache
//	const float* bmin = geom->getNavMeshBoundsMin();
//	const float* bmax = geom->getNavMeshBoundsMax();
//	int gw = 0, gh = 0;
//	rcCalcGridSize(bmin, bmax, cellSize, &gw, &gh);
//	const int ts = (int) tileSize;
//	const int tw = (gw + ts - 1) / ts;
//	const int th = (gh + ts - 1) / ts;
//
//	// Generation params.
//	rcConfig cfg;
//	memset(&cfg, 0, sizeof(cfg));
//	cfg.cs = cellSize;
//	cfg.ch = cellHeight;
//	cfg.walkableSlopeAngle = agentMaxSlope;
//	cfg.walkableHeight = (int) ceilf(agentHeight / cfg.ch);
//	cfg.walkableClimb = (int) floorf(agentMaxClimb / cfg.ch);
//	cfg.walkableRadius = (int) ceilf(agentRadius / cfg.cs);
//	cfg.maxEdgeLen = (int) (edgeMaxLen / cellSize);
//	cfg.maxSimplificationError = edgeMaxError;
//	cfg.minRegionArea = (int) rcSqr(regionMinSize);	  // Note: area = size*size
//	cfg.mergeRegionArea = (int) rcSqr(regionMergeSize); // Note: area = size*size
//	cfg.maxVertsPerPoly = (int) vertsPerPoly;
//	cfg.tileSize = (int) tileSize;
//	cfg.borderSize = cfg.walkableRadius + 3; // Reserve enough padding.
//	cfg.width = cfg.tileSize + cfg.borderSize * 2;
//	cfg.height = cfg.tileSize + cfg.borderSize * 2;
//	cfg.detailSampleDist = detailSampleDist < 0.9f ? 0 : cellSize * detailSampleDist;
//	cfg.detailSampleMaxError = cellHeight * detailSampleMaxError;
//	rcVcopy(cfg.bmin, bmin);
//	rcVcopy(cfg.bmax, bmax);
//
//	// Tile cache params.
//	dtTileCacheParams tcparams;
//	memset(&tcparams, 0, sizeof(tcparams));
//	rcVcopy(tcparams.orig, bmin);
//	tcparams.cs = cellSize;
//	tcparams.ch = cellHeight;
//	tcparams.width = (int) tileSize;
//	tcparams.height = (int) tileSize;
//	tcparams.walkableHeight = agentHeight;
//	tcparams.walkableRadius = agentRadius;
//	tcparams.walkableClimb = agentMaxClimb;
//	tcparams.maxSimplificationError = edgeMaxError;
//	tcparams.maxTiles = tw * th * EXPECTED_LAYERS_PER_TILE;
//	tcparams.maxObstacles = 128;
//
//	dtFreeTileCache(tileCache);
//
//	tileCache = dtAllocTileCache();
//	if (!tileCache) {
//		LOG("buildTiledNavigation: Could not allocate tile cache.");
//		return false;
//	}
//	status = tileCache->init(&tcparams, talloc, tcomp, tmproc);
//	if (dtStatusFailed(status)) {
//		LOG("buildTiledNavigation: Could not init tile cache.");
//		return false;
//	}
//
//	dtFreeNavMesh(navMesh);
//
//	navMesh = dtAllocNavMesh();
//	if (!navMesh) {
//		LOG("buildTiledNavigation: Could not allocate navmesh.");
//		return false;
//	}
//
//	dtNavMeshParams params;
//	memset(&params, 0, sizeof(params));
//	rcVcopy(params.orig, bmin);
//	params.tileWidth = tileSize * cellSize;
//	params.tileHeight = tileSize * cellSize;
//	params.maxTiles = maxTiles;
//	params.maxPolys = maxPolysPerTile;
//
//	status = navMesh->init(&params);
//	if (dtStatusFailed(status)) {
//		LOG("buildTiledNavigation: Could not init navmesh.");
//		return false;
//	}
//
//	status = navQuery->init(navMesh, 2048);
//	if (dtStatusFailed(status)) {
//		LOG("buildTiledNavigation: Could not init Detour navmesh query");
//		return false;
//	}
//
//	// Preprocess tiles.
//
//	ctx->resetTimers();
//
//	cacheLayerCount = 0;
//	cacheCompressedSize = 0;
//	cacheRawSize = 0;
//
//	for (int y = 0; y < th; ++y) {
//		for (int x = 0; x < tw; ++x) {
//			TileCacheData tiles[MAX_LAYERS];
//			memset(tiles, 0, sizeof(tiles));
//			int ntiles = rasterizeTileLayers(ctx, geom, x, y, cfg, tiles, MAX_LAYERS);
//
//			for (int i = 0; i < ntiles; ++i) {
//				TileCacheData* tile = &tiles[i];
//				status = tileCache->addTile(tile->data, tile->dataSize, DT_COMPRESSEDTILE_FREE_DATA, 0);
//				if (dtStatusFailed(status)) {
//					dtFree(tile->data);
//					tile->data = 0;
//					continue;
//				}
//
//				cacheLayerCount++;
//				cacheCompressedSize += tile->dataSize;
//				cacheRawSize += calcLayerBufferSize(tcparams.width, tcparams.height);
//			}
//		}
//	}
//
//	// Build initial meshes
//	ctx->startTimer(RC_TIMER_TOTAL);
//	for (int y = 0; y < th; ++y)
//		for (int x = 0; x < tw; ++x)
//			tileCache->buildNavMeshTilesAt(x, y, navMesh);
//	ctx->stopTimer(RC_TIMER_TOTAL);
//
//	cacheBuildTimeMs = ctx->getAccumulatedTime(RC_TIMER_TOTAL) / 1000.0f;
//	cacheBuildMemUsage = talloc->high;
//
//	const dtNavMesh* nav = navMesh;
//	int navmeshMemUsage = 0;
//	for (int i = 0; i < nav->getMaxTiles(); ++i) {
//		const dtMeshTile* tile = nav->getTile(i);
//		if (tile->header)
//			navmeshMemUsage += tile->dataSize;
//	}
//	printf("navmeshMemUsage = %.1f kB", navmeshMemUsage / 1024.0f);
//
//	if (tool)
//		tool->init(this);
//	initToolStates(this);
//
//	return true;
//}
