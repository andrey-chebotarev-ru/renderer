#include "Renderer.h"

#include "BSP.h"
#include "PVS.h"

void Renderer::prepare(const Map& inMap, const IndicesList& remap)
{
    renderEntities.clear();
    translucentEntities.clear();

    map = inMap;

    Debug::printf("Caching surfaces...");

    for (auto& face : map.tree.faces) {
        face.face.texIdx = remap[face.face.texIdx];

        const Texture& tex = textureManager.get(face.face.texIdx);

        float minU = face.face.poly.verts[0].uv.x;
        float minV = face.face.poly.verts[0].uv.y;

        for (const auto& vert : face.face.poly.verts) {
            minU = std::min(minU, vert.uv.x);
            minU = std::min(minV, vert.uv.y);
        }

        float offsetU = -std::floor(minU / tex.getWidth()) * tex.getWidth();
        float offsetV = -std::floor(minV / tex.getHeight()) * tex.getHeight();

        for (auto& vert : face.face.poly.verts) {
            vert.uv.x += offsetU;
            vert.uv.y += offsetV;
        }
    }



    Debug::printf("Caching surfaces...");

    for (auto& face : map.tree.faces) {
        const Texture& tex = textureManager.get(face.face.texIdx);
        face.face.surfIdx = surfaceCache.cache(face.face, tex, 1.0f);
    }



    for (auto& ent : map.entities) {

        if (ent.getClassname() == "worldspawn" || ent.getClassname().starts_with("func_")) {

            if (!ent.faces.size()) {
                continue;
            }

            RenderEntity renderEntity;
            Bounds::AABB3 aabb = Bounds::create(ent.faces);

            BSP::findTouchedLeaves(map.tree, map.tree.rootNodeIdx, aabb, renderEntity.touchedLeaves);

            std::string renderamt = ent.getProperty("renderamt");
            int renderamtVal = 255;

            bool translucent = false;

            if (renderamt != "" && renderamt != "0") {
                renderamtVal = atoi(renderamt.c_str());

                if (renderamtVal != 255) {

                    translucent = true;
                    renderEntity.alpha = renderamtVal;
                }
            }

            for (auto& face : ent.faces) {
                face.texIdx = remap[face.texIdx];
                const Texture& tex = textureManager.get(face.texIdx);
                face.surfIdx = surfaceCache.cache(face, tex, renderEntity.alpha);
                renderEntity.faces.push_back(face);
            }

            renderEntity.aabb = Bounds::create(renderEntity.faces);

            if (translucent) {
                translucentEntities.push_back(renderEntity);
            }
            else {
                renderEntities.push_back(renderEntity);
            }

            
        }
    }
}

bool Renderer::checkEntityVisibility(const RenderEntity& ent, const Camera& cam, const BoolList& pvsRow)
{
    if (cam.aabbInFrustrum(ent.aabb)) {
        for (auto leafIdx : ent.touchedLeaves) {
            if (pvsRow[leafIdx]) {
                return true;
            }
        }
    }

    return false;
}

void Renderer::drawFaces(const FaceList& faces, const Camera& cam, bool fill, RenderStat& stat)
{
    for (const auto& face : faces) {

        if (face.type == FaceType::Bounds) {
            continue;
        }

        uint8_t mipLevel = 0;

        if (face.type == FaceType::Debug)
        {
            Color fillColor = face.debugColor;

            Poly2D renderPoly;

            if (!prepareForRendering(renderPoly, face.poly, cam, mipLevel, false)) {
                continue;
            }

            // Drawing
            stat.trisCount += rasterizer.drawColoredPolygon<false>(renderPoly, fillColor);
            drawWireframe(renderPoly, Color(0, 0, 0));
        }
        else
        {
            const CachedSurface& surf = surfaceCache.get(face.surfIdx);
            Color fillColor = face.debugColor;

            bool notWater = (face.type != FaceType::Liquid);

            Poly2D renderPoly;
            if (!prepareForRendering(renderPoly, face.poly, cam, mipLevel, notWater)) {
                continue;
            }

            // Drawing
            if (fill) {
                stat.trisCount += rasterizer.drawColoredPolygon<false>(renderPoly, fillColor);
                drawWireframe(renderPoly, Color(0, 0, 0));
            }
            else {
                if (surf.alpha == 1.0f) {
                    stat.polyCount += rasterizer.drawPolygon<false, false>(renderPoly, surf, mipLevel);
                }
                else {
                    stat.polyCount += rasterizer.drawPolygon<false, true>(renderPoly, surf, mipLevel);
                }
            }
        }

    }
}

void Renderer::drawEntities(const RenderEntityList& ents, const Camera& cam, const BoolList& pvsRow, RenderStat& stat)
{
    for (const auto &ent : ents) {

        if (!checkEntityVisibility(ent, cam, pvsRow)) {
            continue;
        }

        for (const auto& face : ent.faces) {

            uint8_t mipLevel = 0;

            const CachedSurface& surf = surfaceCache.get(face.surfIdx);
            Color fillColor = face.debugColor;

            Poly2D renderPoly;
            if (!prepareForRendering(renderPoly, face.poly, cam, mipLevel, true)) {
                continue;
            }

            // Drawing

            if (surf.alpha == 255) {
                stat.polyCount += rasterizer.drawPolygon<true, false>(renderPoly, surf, mipLevel);
            }
            else {
                stat.polyCount += rasterizer.drawPolygon<true, true>(renderPoly, surf, mipLevel);
            }
            
            stat.entCount++;
        }
    }
}

RenderStat Renderer::drawScene(const Camera& cam, bool debug, bool includePortals, bool updateWorldPVS, bool updateEntsPVS)
{
    static const BoolList* worldPVSRow = nullptr;
    static const BoolList* entitiesPVSRow = nullptr;

    RenderStat stat;

    renderFaces.clear();
    rasterizer.clearBuffers();

   currentLeafIdx = BSP::getLeafIdx(map.tree, cam.position);

    if (updateWorldPVS) {
        worldPVSRow = &map.pvsTable[currentLeafIdx];
    }

    if (updateEntsPVS) {
        entitiesPVSRow = &map.pvsTable[currentLeafIdx];
    }

    BSP::render(map.tree, cam, renderFaces, *worldPVSRow);

    drawFaces(renderFaces, cam, debug, stat);

    drawEntities(renderEntities, cam, *entitiesPVSRow, stat);
    drawEntities(translucentEntities, cam, *entitiesPVSRow, stat);

    if (includePortals) {
        drawPortals(cam, stat);
    }

    return stat;
}

void Renderer::drawPortals(const Camera& cam, RenderStat& stat)
{
    const BSP::Leaf& leaf = map.tree.getLeaf(currentLeafIdx);

    FaceList faces;

    for (int portalIdx : leaf.portalIdxs) {
        const BSP::Portal& portal = map.tree.getPortal(portalIdx);

        faces.push_back(Face(portal.poly, -1, FaceType::Debug, Color::getDebugColor(currentLeafIdx + portal.frontLeafIdx)));
    }

    drawFaces(faces, cam, true, stat);
}

void Renderer::drawPortalLabels(const Camera& cam)
{
    const BSP::Leaf& leaf = map.tree.getLeaf(currentLeafIdx);

    LabelList portalLabels;
    static char buf[256];

    for (int portalIdx : leaf.portalIdxs) {
        const BSP::Portal& portal = map.tree.getPortal(portalIdx);

        sprintf_s(buf, "#%i %i <=> %i", portalIdx, currentLeafIdx, portal.frontLeafIdx);
        portalLabels.push_back(Label(portal.poly.getCenter(), Color::getDebugColor(currentLeafIdx - portal.frontLeafIdx), buf));
    }

    drawLabels(portalLabels, cam);
}