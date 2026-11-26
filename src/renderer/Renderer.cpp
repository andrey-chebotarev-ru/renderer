#include "Renderer.h"

#include "BSP.h"
#include "PVS.h"

void Renderer::prepare(const Map& inMap, const IndicesList& remap)
{
    renderEntities.clear();

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
        face.face.surfIdx = surfaceCache.cache(face.face, tex);
    }



    for (auto& ent : map.entities) {

        if (ent.getClassname() == "worldspawn" || ent.getClassname().starts_with("func_")) {

            if (!ent.faces.size()) {
                continue;
            }

            RenderEntity renderEntity;
            Bounds::AABB3 aabb = Bounds::create(ent.faces);

            BSP::findTouchedLeaves(map.tree, map.tree.rootNodeIdx, aabb, renderEntity.touchedLeaves);

            for (auto& face : ent.faces) {
                face.texIdx = remap[face.texIdx];
                const Texture& tex = textureManager.get(face.texIdx);
                face.surfIdx = surfaceCache.cache(face, tex);
                renderEntity.faces.push_back(face);
            }

            renderEntities.push_back(renderEntity);
        }
    }
}

int Renderer::drawFaces(const Camera& cam, bool fill)
{
    int count = 0;

    for (const auto& face : renderFaces) {

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
            count += rasterizer.drawColoredPolygon<false, true>(renderPoly, fillColor);
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
                count += rasterizer.drawColoredPolygon<false, false>(renderPoly, fillColor);
                drawWireframe(renderPoly, Color(0, 0, 0));
            }
            else {
                if (notWater) {
                    count += rasterizer.drawPolygon<false, false>(renderPoly, surf, mipLevel);
                }
                else {
                    count += rasterizer.drawPolygon<false, true>(renderPoly, surf, mipLevel);
                }
            }
        }

    }

    return count;
}

int Renderer::drawEntities(const Camera& cam, const BoolList& pvsRow, bool debug, bool fill)
{
    int count = 0;

    for (const auto &renderEntity : renderEntities) {

        bool visible = false;

        for (auto leafIdx : renderEntity.touchedLeaves) {
            if (pvsRow[leafIdx]) {
                visible = true;
                break;
            }
        }

        if (!visible) {
            continue;
        }

        for (const auto& face : renderEntity.faces) {

            uint8_t mipLevel = 0;

            if (face.type == FaceType::Debug)
            {
                Color fillColor = face.debugColor;

                Poly2D renderPoly;

                if (!prepareForRendering(renderPoly, face.poly, cam, mipLevel, false)) {
                    continue;
                }

                // Drawing
                count += rasterizer.drawColoredPolygon<false, true>(renderPoly, fillColor);
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
                    count += rasterizer.drawColoredPolygon<false, false>(renderPoly, fillColor);
                    drawWireframe(renderPoly, Color(0, 0, 0));
                }
                else {
                    if (notWater) {
                        count += rasterizer.drawPolygon<true, false>(renderPoly, surf, mipLevel);
                    }
                    else {
                        count += rasterizer.drawPolygon<true, true>(renderPoly, surf, mipLevel);
                    }
                }
            }

        }
    }

    return count;
}

int Renderer::drawScene(const Camera& cam, bool debug, bool freezePvs)
{
    static const BoolList* pvsRowPtr = nullptr;

    renderFaces.clear();
    rasterizer.clearBuffers();

    size_t leafIdx = BSP::getLeafIdx(map.tree, cam.position);

    if (!freezePvs) {
        pvsRowPtr = &map.pvsTable[leafIdx];
    }

    BSP::render(map.tree, cam, renderFaces, *pvsRowPtr);

    int count = drawFaces(cam, debug);

    count += drawEntities(cam, *pvsRowPtr, debug, false);

    return count;
}