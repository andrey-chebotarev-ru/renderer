#include "BaseRenderer.h"

#include <algorithm>

#include "common/Geometry.h"

void BaseRenderer::setFramebuffer(int pitch, uint8_t* pixels)
{
    rasterizer.setFramebuffer(pitch, pixels);
}

void BaseRenderer::transformToWorld(Triangle3& tri, const Transform& tr) const
{
    for (int i = 0; i < 3; i++) {
        tri.v[i].pos = Math::transform(tri.v[i].pos, tr);
    }
}

void BaseRenderer::transformToWorld(Poly3D& poly, const Transform& tr) const
{
    for (auto& vert : poly.verts) {
        vert.pos = Math::transform(vert.pos, tr);
    }
}

Vec3 BaseRenderer::transformToView(const Vec3& vec, const Camera& cam) const
{
    Vec3 out = vec - cam.position;

    return Math::rotate(out, -cam.rotation);
}

void BaseRenderer::transformToView(Triangle3& tri, const Camera& cam) const
{
    for (int i = 0; i < 3; i++) {
        tri.v[i].pos = transformToView(tri.v[i].pos, cam);
    }
}

void BaseRenderer::transformToView(Poly3D& poly, const Camera& cam) const
{
    for (auto& vert : poly.verts) {
        vert.pos = transformToView(vert.pos, cam);
    }
}

Vec2 BaseRenderer::project(const Vec3& vec) const
{
    Vec2 proj;
    float invZ = 1.0f / vec.z;

    proj.x = screenCenter.x + (vec.x * invZ) * focusDistance;
    proj.y = screenCenter.y + pitchOffset + (vec.y * invZ) * focusDistance;

    return proj;
}

Triangle2 BaseRenderer::project(const Triangle3& tri) const
{
    Triangle2 proj;

    for (int i = 0; i < 3; i++) {
        float invZ = 1.0f / tri.v[i].pos.z;

        proj.v[i].pos = project(tri.v[i].pos);
        proj.v[i].uz = tri.v[i].uv.x * invZ;
        proj.v[i].vz = tri.v[i].uv.y * invZ;
        proj.v[i].iz = invZ;
    }

    proj.fill = tri.fill;

    return proj;
}

Poly2D BaseRenderer::project(const Poly3D& poly) const
{
    Poly2D proj;

    for (const auto& vert : poly.verts) {
        float invZ = 1.0f / vert.pos.z;

        Vertex2D projVert;

        projVert.pos = project(vert.pos);
        projVert.uz = vert.uv.x * invZ;
        projVert.vz = vert.uv.y * invZ;
        projVert.iz = invZ;

        proj.addVert(projVert);
    }

    return proj;
}

Triangle3DList BaseRenderer::clip(const Triangle3& in) const
{
    return clipper.clip(in, nearPlaneDistance, farPlaneDistance);
}

Triangle2DList BaseRenderer::clip(const Triangle2& in) const
{
    return viewportClipper.clip(in, viewport);
}

Poly3D BaseRenderer::clip(const Poly3D& in) const
{
    return clipper.clip(in, nearPlaneDistance, farPlaneDistance);
}

Poly2D BaseRenderer::clip(const Poly2D& in) const
{
    return viewportClipper.clip(in, viewport);
}

uint8_t BaseRenderer::getMipLevel(const Triangle3& tri) const
{
    return 0;
}

uint8_t BaseRenderer::getMipLevel(const Poly3D& poly) const
{
    float minZ = poly.verts[0].pos.z;

    for (const auto& vert : poly.verts) {
        minZ = std::min(minZ, vert.pos.z);
    }

    if (minZ < 130) {
        return 0;
    }

    if (minZ < 300) {
        return 1;
    }

    if (minZ < 600) {
        return 2;
    }

    return 3;
}

Triangle2DList BaseRenderer::prepareForRendering(const Triangle3& in, const Camera& cam, bool backfaceCulling) const
{
    Triangle2DList out;

    // Backface culling
    if (backfaceCulling && Geometry::test(cam.position, in.getPlane()) != Side::Front) {
        return out;
    }

    Triangle3 tri = in;

    // Transform
    transformToView(tri, cam);

    // 3D clipping
    Triangle3DList clippedTris = clip(tri);

    if (clippedTris.empty()) {
        return out;
    }

    for (const auto& clippedTri : clippedTris) {
        Triangle2 projectedTri = project(clippedTri);

        // Viewport clipping
        Triangle2DList vpClippedTris = clip(projectedTri);

        if (vpClippedTris.empty()) {
            return out;
        }

        for (const auto& vpClippedTri : vpClippedTris) {
            out.push_back(vpClippedTri);
        }
    }

    return out;
}

bool BaseRenderer::prepareForRendering(Poly2D& out, const Poly3D& in, const Camera& cam, uint8_t& mipLevel, bool backfaceCulling) const
{
    // Backface culling
    if (backfaceCulling && Geometry::test(cam.position, in.getPlane()) != Side::Front) {
        return false;
    }

    Poly3D poly = in;

    // Transform
    transformToView(poly, cam);

    // 3D clipping
    poly = clip(poly);

    if (!poly.isValid()) {
        return false;
    }

    // Mip
    mipLevel = getMipLevel(poly);

    // Projection
    Poly2D projectedPoly = project(poly);

    // Viewport clipping
    out = clip(projectedPoly);

    return out.isValid();
}

/*
int BaseRenderer::drawModel(const Model& model, const Camera& cam)
{
    int count = 0;

    for (int triIdx = 0; triIdx < model.getSize(); triIdx++) {
        Triangle3 tri = model.getTriangle(triIdx);
        Transform tr = Transform(Vec3(1.0f, 1.0f, 1.0f), model.getRotation(), model.getPosition());

        transformToWorld(tri, tr);

        const CachedSurface& surf = surfaceCache.get(model.getTextureIndex());

        Triangle2DList preparedTris = prepareForRendering(tri, cam, true);

        if (preparedTris.empty()) {
            continue;
        }
        
        for (const auto& preparedTri : preparedTris) {
            rasterizer.drawTriangle<true, false>(preparedTri, surf, 0);
            count++;
        }
    }

    return count;
}
*/

void BaseRenderer::drawLabels(const LabelList& labels, const Camera& cam)
{
    for (const auto& label : labels) {
        Vec3 pos = label.pos;

        // Transform
        pos = transformToView(pos, cam);

        // Simple clipping
        if (pos.z < 1.0f) {
            continue;
        }

        // Projection
        Vec2 proj = project(pos);

        debug.drawText(proj.x, proj.y, label.color, label.label);
    }
}

void BaseRenderer::drawWireframe(const Poly2D& poly, const Color& color)
{
    for (int edgeIdx = 0; edgeIdx < poly.getEdgeCount(); edgeIdx++) {
        const Edge2D& edge = poly.getEdge(edgeIdx);

        rasterizer.drawLine(edge.vert1.pos, edge.vert2.pos, color);
    }
}

void BaseRenderer::drawAABB(const Bounds::AABB2& aabb, const Color& color)
{
    Vec2 v00 = Vec2(aabb.min.x, aabb.min.y);
    Vec2 v01 = Vec2(aabb.min.x, aabb.max.y);
    Vec2 v10 = Vec2(aabb.max.x, aabb.min.y);
    Vec2 v11 = Vec2(aabb.max.x, aabb.max.y);

    rasterizer.drawLine(v00, v10, color);
    rasterizer.drawLine(v10, v11, color);
    rasterizer.drawLine(v11, v01, color);
    rasterizer.drawLine(v01, v00, color);
}

void BaseRenderer::setPixel(int x, int y, const Color& color)
{
    rasterizer.setPixel(x, y, color);
}