#include "Renderer.h"

#include <algorithm>

#include "Geometry.h"

const float Renderer::nearPlaneDistance = 1.0f;
const float Renderer::farPlaneDistance = 500.0f;

void Renderer::setFramebuffer(int pitch, uint8_t* pixels)
{
    rasterizer.setFramebuffer(pitch, pixels);
}

void Renderer::setTexture(const Texture& texture, size_t index)
{
    rasterizer.setTexture(texture, index);
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
    Poly2D projected = project(poly, texWidth, texHeight);

    // Viewport clipping
    out = clip(projected);

    return out.isValid();
}

int Renderer::drawModel(const Model& model, const Camera& cam)
{
    int count = 0;

    for (int i = 0; i < model.getSize(); i++) {

        Triangle3 tri = model.getTriangle(i);
        Transform tr = Transform(Vec3(1.0f, 1.0f, 1.0f), model.getRotation(), model.getPosition());
        
        // Transform
        transformToWorld(tri, tr);

        // Backface culling
        if (backfaceCulling && Geometry::test(cam.position, tri.getPlane()) != Side::Front) {
            continue;
        }

        // Transform
        transformToView(tri, cam);

        // 3D clipping
        Triangle3DList clippedTris = clip(tri);

        if (clippedTris.empty()) {
            continue;
        }

        Texture tex = rasterizer.getTexture(model.getTextureIndex());

        for (const Triangle3& clipped : clippedTris) {

            // Projection
            Triangle2 projected = project(clipped, tex.getWidth(), tex.getHeight());

            // Viewport clipping
            Triangle2DList vpClippedTriangles = clip(projected);

            // Drawing
            for (const auto& vpClipped : vpClippedTriangles) {
                rasterizer.drawTriangle<true>(vpClipped, tex);
                count++;
            }
        }
    }

    return count;
}

int Renderer::drawWorld(const World& world, const Camera& cam, bool fill)
{
    int count = 0;

    for (int i = 0; i < world.getSize(); i++) {

        Face face = world.getFace(i);

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
            count += rasterizer.drawSolidPolygon<false, true>(renderPoly, fillColor);
            drawWireframe(renderPoly, Color(0, 0, 0));
        }
        else
        {
            Texture tex = rasterizer.getTexture(face.texIdx);
            Color fillColor = face.debugColor;

            Poly2D renderPoly;
            if (!prepareForRendering(renderPoly, face.poly, cam, true, tex.getWidth(), tex.getHeight())) {
                continue;
            }

            // Drawing
            if (fill) {
                count += rasterizer.drawSolidPolygon<false, false>(renderPoly, fillColor);
                drawWireframe(renderPoly, Color(0, 0, 0));
            }
            else {
                count += rasterizer.drawPolygon<false>(renderPoly, tex);
            }
        }
        
    }

    return count;
}

void Renderer::drawScene(const World& world, const ModelList&models, const Camera& cam, Data& data)
{
    rasterizer.clearBuffers();
    
    backfaceCulling = true;

    zTest = false;
    data.count = drawWorld(world, cam, fill);

    zTest = true;
    for (const Model& model : models) {
        data.count += drawModel(model, cam);
    }
}

void Renderer::drawExtras(const FaceList& faces, const Camera& cam)
{
    zTest = true;

    for (auto face : faces) {
        // Transform
        transformToView(face.poly, cam);

        // 3D clipping
        Poly3D clippedPoly = clip(face.poly);

        if (clippedPoly.verts.empty()) {
            continue;
        }

        Color fillColor = face.debugColor;

        // Projection
        Poly2D projected = project(clippedPoly, 0, 0);

        // Viewport clipping
        Poly2D vpClippedPoly = clip(projected);

        rasterizer.drawSolidPolygon<true, true>(vpClippedPoly, fillColor);
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
        Edge2D edge = poly.getEdge(edgeIdx);

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