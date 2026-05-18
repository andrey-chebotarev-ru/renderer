#include "Renderer.h"

#include <cstring>
#include <iostream>
#include <assert.h>
#include <vector>
#include <algorithm>

#include "Math.h"
#include "Vector.h"
#include "Camera.h"
#include "Triangle.h"
#include "Model.h"
#include "Clipper.h"
#include "World.h"

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

void Renderer::transformToWorld(Triangle3& tri, const Transform& tr)
{
    for (int i = 0; i < 3; i++) {
        tri.v[i].position = Math::transform(tri.v[i].position, tr);
    }
}

Vec3 Renderer::transformToView(const Vec3& vec, const Camera& cam)
{
    Vec3 out = vec - cam.position;

    return Math::rotate(out, -cam.rotation);
}

void Renderer::transformToView(Triangle3& tri, const Camera& cam)
{
    for (int i = 0; i < 3; i++) {
        tri.v[i].position = transformToView(tri.v[i].position, cam);
    }
}

Vec2 Renderer::project(const Vec3& vec)
{
    Vec2 proj;
    float invZ = 1.0f / vec.z;

    proj.x = screenCenter.x + (vec.x * invZ) * focusDistance * aspect;
    proj.y = screenCenter.y + (vec.y * invZ) * focusDistance;

    return proj;
}

Triangle2 Renderer::project(const Triangle3& tri, const Texture& tex)
{
    Triangle2 proj;

    int texWidth = tex.getWidth();
    int texHeight = tex.getHeight();

    for (int i = 0; i < 3; i++) {
        float invZ = 1.0f / tri.v[i].position.z;

        proj.v[i].pos = project(tri.v[i].position);
        proj.v[i].uz = (tri.v[i].uv.x * texWidth) * invZ;
        proj.v[i].vz = (tri.v[i].uv.y * texHeight) * invZ;
        proj.v[i].iz = invZ;
    }

    proj.fill = tri.fill;

    return proj;
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
        if (backfaceCulling && Math::distanceToPlane(tri.getPlane(), cam.position) < 0.0f) {
            continue;
        }

        // Transform
        transformToView(tri, cam);

        // 3D clipping
        Triangle3DList clippedTriangles = clipper.clipTriangle(tri, nearPlaneDistance, farPlaneDistance);

        if (clippedTriangles.empty()) {
            continue;
        }

        Texture tex = rasterizer.getTexture(model.getTextureIndex());

        for (const Triangle3& clipped : clippedTriangles) {

            // Projection
            Triangle2 projected = project(clipped, tex);

            // Viewport clipping
            Triangle2DList vpClippedTriangles = viewportClipper.clipTriangle(projected, viewport);

            // Drawing
            for (const Triangle2& vpClipped : vpClippedTriangles) {
                rasterizer.drawTriangle(vpClipped, tex, zTest, fill);
                count++;
            }
        }
    }

    return count;
}

int Renderer::drawWorld(const World& world, const Camera& cam)
{
    int count = 0;

    for (int i = 0; i < world.getSize(); i++) {

        Triangle3 tri = world.getTriangle(i);

        // Backface culling
        if (backfaceCulling && Math::distanceToPlane(tri.getPlane(), cam.position) < 0.0f) {
            continue;
        }

        // Transform
        transformToView(tri, cam);

        // 3D clipping
        Triangle3DList clippedTriangles = clipper.clipTriangle(tri, nearPlaneDistance, farPlaneDistance);

        if (clippedTriangles.empty()) {
            continue;
        }

        Texture tex = rasterizer.getTexture(tri.texId);

        for (const Triangle3& clipped : clippedTriangles) {

            // Projection
            Triangle2 projected = project(clipped, tex);

            // Viewport clipping
            Triangle2DList vpClippedTriangles = viewportClipper.clipTriangle(projected, viewport);

            // Drawing
            for (const Triangle2& vpClipped : vpClippedTriangles) {
                rasterizer.drawTriangle(vpClipped, tex, zTest, fill);
                count++;
            }
        }
    }

    return count;
}

void Renderer::drawScene(const World& world, const std::vector<Model>& models, const Camera& cam, Data& data)
{
    rasterizer.clearBuffers();

    zTest = false;
    data.count = drawWorld(world, cam);

    zTest = true;
    for (const Model& model : models) {
        data.count += drawModel(model, cam);
    }
}
