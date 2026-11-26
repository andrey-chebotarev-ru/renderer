#pragma once

#include "BaseRenderer.h"

class Renderer : public BaseRenderer
{
public:

	Renderer(size_t screenWidth, size_t screenHeight, float fov, float nearPlaneDistance, float farPlaneDistance, const TextureManager& textureManager, SurfaceCache& surfaceCache, const Debug& debug)
		: BaseRenderer(screenWidth, screenHeight, fov, nearPlaneDistance, farPlaneDistance, debug), textureManager(textureManager), surfaceCache(surfaceCache) {	
	}

	void prepare(const Map& inMap, const IndicesList& remap);

	int drawFaces(const Camera& cam, bool debug);
	int drawEntities(const Camera& cam, const BoolList& pvsRow, bool debug, bool fill);

	int drawScene(const Camera& cam, bool debug, bool freezePvs);
	
private:

	Map map;

	RenderEntityList renderEntities;
	FaceList renderFaces;

	const TextureManager& textureManager;
	SurfaceCache& surfaceCache;
};