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

Vec3 Renderer::rotateY(Vec3& vec, float yaw)
{
    Vec3 out;

    out.x = vec.x * cosf(yaw) + vec.z * sinf(yaw);
    out.y = vec.y;
    out.z = -vec.x * sinf(yaw) + vec.z * cosf(yaw);

    return out;
}

Vec3 Renderer::rotateX(Vec3& vec, float pitch)
{
    Vec3 out;

    out.x = vec.x;
    out.y = vec.y * cosf(pitch) - vec.z * sinf(pitch);
    out.z = vec.y * sinf(pitch) + vec.z * cosf(pitch);

    return out;
}

Vec3 Renderer::rotateZ(Vec3& vec, float roll)
{
    Vec3 out;

    out.x = vec.x * cosf(roll) - vec.y * sinf(roll);
    out.y = vec.x * sinf(roll) + vec.y * cosf(roll);
    out.z = vec.z;

    return out;
}

void Renderer::transformToWorld(Vec3& vec, const Transform& tr)
{
    // Scale
    vec *= tr.scale;

    // Rotate
    vec = rotateY(vec, tr.rotation.y);
    vec = rotateX(vec, tr.rotation.x);
    vec = rotateZ(vec, tr.rotation.z);

    // Translate
    vec += tr.position;
}

void Renderer::transformToWorld(Triangle3& tri, const Transform& tr)
{
    for (int i = 0; i < 3; i++) {
        transformToWorld(tri.v[i].position, tr);
    }

    tri.normal = Vec3::normalize(Vec3::cross(tri.v1.position - tri.v0.position, tri.v2.position - tri.v0.position));
}

void Renderer::transformToView(Vec3& vec, const Camera& cam)
{
    vec -= cam.position;

    vec = rotateY(vec, -cam.rotation.y);
    vec = rotateX(vec, -cam.rotation.x);
    vec = rotateZ(vec, -cam.rotation.z);
}

void Renderer::transformToView(Triangle3& tri, const Camera& cam)
{
    for (int i = 0; i < 3; i++) {
        transformToView(tri.v[i].position, cam);
    }

    tri.normal = Vec3::normalize(Vec3::cross(tri.v1.position - tri.v0.position, tri.v2.position - tri.v0.position));
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

    return proj;
}

float Renderer::testTriangle(Triangle3 triangle, Vec3 position)
{
    return Vec3::dot(triangle.normal, position - triangle.v0.position);
}

int Renderer::drawModel(const Model& model, const Camera& cam)
{
    int count = 0;

    for (int i = 0; i < model.getSize(); i++) {

        Triangle3 tri = model.getTriangle(i);
        Transform tr = Transform(model.getPosition(), model.getRotation(), Vec3(1.0f, 1.0f, 1.0f));
        
        // Transform
        transformToWorld(tri, tr);

        // Backface culling
        if (testTriangle(tri, cam.position) < 0.0f) {
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
                rasterizer.drawTriangle(vpClipped, tex);
                count++;
            }
        }
    }

    return count;
}

void Renderer::drawScene(const std::vector<Model>& models, const Camera& cam, Data& data)
{
    rasterizer.clearBuffers();

    data.count = 0;

    for (const Model& model : models) {
        data.count += drawModel(model, cam);
    }
}
