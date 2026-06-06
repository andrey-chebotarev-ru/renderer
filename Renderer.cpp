#include "Renderer.h"

#include <algorithm>

#include "Geometry.h"

const float Renderer::nearPlaneDistance = 1.0f;
const float Renderer::farPlaneDistance = 1000.0f;

void Renderer::setFramebuffer(int pitch, uint8_t* pixels)
{
    rasterizer.setFramebuffer(pitch, pixels);
}

void Renderer::transformToWorld(Triangle3& tri, const Transform& tr) const
{
    for (int i = 0; i < 3; i++) {
        tri.v[i].pos = Math::transform(tri.v[i].pos, tr);
    }
}

void Renderer::transformToWorld(Poly3D& poly, const Transform& tr) const
{
    for (auto& vert : poly.verts) {
        vert.pos = Math::transform(vert.pos, tr);
    }
}

Vec3 Renderer::transformToView(const Vec3& vec, const Camera& cam) const
{
    Vec3 out = vec - cam.position;

    return Math::rotate(out, -cam.rotation);
}

void Renderer::transformToView(Triangle3& tri, const Camera& cam) const
{
    for (int i = 0; i < 3; i++) {
        tri.v[i].pos = transformToView(tri.v[i].pos, cam);
    }
}

void Renderer::transformToView(Poly3D& poly, const Camera& cam) const
{
    for (auto& vert : poly.verts) {
        vert.pos = transformToView(vert.pos, cam);
    }
}

Vec2 Renderer::project(const Vec3& vec) const
{
    Vec2 proj;
    float invZ = 1.0f / vec.z;

    proj.x = screenCenter.x + (vec.x * invZ) * focusDistance * aspect;
    proj.y = screenCenter.y + (vec.y * invZ) * focusDistance;

    return proj;
}

Triangle2 Renderer::project(const Triangle3& tri, int texWidth, int texHeight) const
{
    Triangle2 proj;

    for (int i = 0; i < 3; i++) {
        float invZ = 1.0f / tri.v[i].pos.z;

        proj.v[i].pos = project(tri.v[i].pos);
        proj.v[i].uz = (tri.v[i].uv.x * texWidth) * invZ;
        proj.v[i].vz = (tri.v[i].uv.y * texHeight) * invZ;
        proj.v[i].iz = invZ;
    }

    proj.fill = tri.fill;

    return proj;
}

Poly2D Renderer::project(const Poly3D& poly, int texWidth, int texHeight) const
{
    Poly2D proj;

    for (const auto& vert : poly.verts) {
        float invZ = 1.0f / vert.pos.z;

        Vertex2D projVert;

        projVert.pos = project(vert.pos);
        projVert.uz = (vert.uv.x * texWidth) * invZ;
        projVert.vz = (vert.uv.y * texHeight) * invZ;
        projVert.iz = invZ;

        proj.addVert(projVert);
    }

    return proj;
}

Triangle3DList Renderer::clip(const Triangle3& in) const
{
    return clipper.clip(in, nearPlaneDistance, farPlaneDistance);
}

Triangle2DList Renderer::clip(const Triangle2& in) const
{
    return viewportClipper.clip(in, viewport);
}

Poly3D Renderer::clip(const Poly3D& in) const
{
    return clipper.clip(in, nearPlaneDistance, farPlaneDistance);
}

Poly2D Renderer::clip(const Poly2D& in) const
{
    return viewportClipper.clip(in, viewport);
}

Triangle2DList Renderer::prepareForRendering(const Triangle3& in, const Camera& cam, bool backfaceCulling, int texWidth, int texHeight) const
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
        Triangle2 projectedTri = project(clippedTri, texWidth, texHeight);

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

bool Renderer::prepareForRendering(Poly2D& out, const Poly3D& in, const Camera& cam, bool backfaceCulling, int texWidth, int texHeight) const
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

    // Projection
    Poly2D projectedPoly = project(poly, texWidth, texHeight);

    // Viewport clipping
    out = clip(projectedPoly);

    return out.isValid();
}

int Renderer::drawModel(const Model& model, const Camera& cam)
{
    int count = 0;

    for (int triIdx = 0; triIdx < model.getSize(); triIdx++) {
        Triangle3 tri = model.getTriangle(triIdx);
        Transform tr = Transform(Vec3(1.0f, 1.0f, 1.0f), model.getRotation(), model.getPosition());

        transformToWorld(tri, tr);

        Texture tex = textureManager.get(model.getTextureIndex());

        Triangle2DList preparedTris = prepareForRendering(tri, cam, true, tex.getWidth(), tex.getHeight());

        if (preparedTris.empty()) {
            continue;
        }
        
        for (const auto& preparedTri : preparedTris) {
            rasterizer.drawTriangle<true>(preparedTri, tex);
            count++;
        }
    }

    return count;
}

int Renderer::drawWorld(const World& world, const Camera& cam, bool fill)
{
    int count = 0;

    for (int faceIdx = 0; faceIdx < world.getSize(); faceIdx++) {

        const Face& face = world.getFace(faceIdx);

        if (face.type == FaceType::Bounds) {
            continue;
        }

        if (face.type == FaceType::Debug)
        {
            Color fillColor = face.debugColor;

            Poly2D renderPoly;
            
            if (!prepareForRendering(renderPoly, face.poly, cam, false)) {
                continue;
            }
            
            // Drawing
            count += rasterizer.drawColoredPolygon<false, true>(renderPoly, fillColor);
            drawWireframe(renderPoly, Color(0, 0, 0));
        }
        else
        {
            Texture tex = textureManager.get(face.texIdx);
            Color fillColor = face.debugColor;

            Poly2D renderPoly;
            if (!prepareForRendering(renderPoly, face.poly, cam, true, tex.getWidth(), tex.getHeight())) {
                continue;
            }

            // Drawing
            if (fill) {
                count += rasterizer.drawColoredPolygon<false, false>(renderPoly, fillColor);
                drawWireframe(renderPoly, Color(0, 0, 0));
            }
            else {
                count += rasterizer.drawPolygon<false>(renderPoly, tex);
            }
        }
        
    }

    return count;
}

int Renderer::drawScene(const World& world, const ModelList&models, const Camera& cam, bool debug)
{
    rasterizer.clearBuffers();
    
    int count = drawWorld(world, cam, debug);

    for (const Model& model : models) {
        count += drawModel(model, cam);
    }

    return count;
}

void Renderer::drawExtras(const FaceList& faces, const Camera& cam)
{
    for (const auto& face : faces) {
        Poly2D poly;

        if (!prepareForRendering(poly, face.poly, cam, false)) {
            continue;
        }

        rasterizer.drawColoredPolygon<true, true>(poly, face.debugColor);
    }
}

void Renderer::drawLabels(const LabelList& labels, const Camera& cam)
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

void Renderer::drawWireframe(const Poly2D& poly, const Color& color)
{
    for (int edgeIdx = 0; edgeIdx < poly.getEdgeCount(); edgeIdx++) {
        const Edge2D& edge = poly.getEdge(edgeIdx);

        rasterizer.drawLine(edge.vert1.pos, edge.vert2.pos, color);
    }
}

void Renderer::drawAABB(const AABB2D& aabb, const Color& color)
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