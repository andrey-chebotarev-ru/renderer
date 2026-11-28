#pragma once

#include "BaseRenderer.h"

struct RenderOptions
{
	bool renderTextures = true;
	bool renderEnts = true;
	bool renderPortals = false;
	bool updatePVS = true;
};

struct RenderStat
{
	size_t polyCount = 0;
	size_t trisCount = 0;
	size_t entCount = 0;
	size_t spanCount = 0;
};

class Renderer : public BaseRenderer
{
public:

	Renderer(size_t screenWidth, size_t screenHeight, float fov, float nearPlaneDistance, float farPlaneDistance, const TextureManager& textureManager, SurfaceCache& surfaceCache, const Debug& debug)
		: BaseRenderer(screenWidth, screenHeight, fov, nearPlaneDistance, farPlaneDistance, debug), textureManager(textureManager), surfaceCache(surfaceCache) {	
	}

	void prepare(const Map& inMap, const IndicesList& remap);

	bool checkEntityVisibility(const RenderEntity& ent, const Camera& cam, const BoolList& pvsRow);

	void drawFaces(const FaceList& faces, const Camera& cam, bool fill, RenderStat& stat);
	void drawEntities(const RenderEntityList& ents, const Camera& cam, const BoolList& pvsRow, RenderStat& stat);

	RenderStat drawScene(const Camera& cam, const RenderOptions& opts);

	void drawPortals(const Camera& cam, RenderStat& stat);
	void drawPortalLabels(const Camera& cam);
	
private:

	Map map;

	size_t currentLeafIdx;

	RenderEntityList renderEntities;
	RenderEntityList translucentEntities;
	FaceList renderFaces;

	const TextureManager& textureManager;
	SurfaceCache& surfaceCache;


};