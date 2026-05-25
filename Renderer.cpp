#include "Renderer.h"

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

void Renderer::transformToWorld(Poly3D& poly, const Transform& tr)
{
    for (auto& vert : poly.verts) {
        vert.position = Math::transform(vert.position, tr);
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

void Renderer::transformToView(Poly3D& poly, const Camera& cam)
{
    for (auto& vert : poly.verts) {
        vert.position = transformToView(vert.position, cam);
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

Poly2D Renderer::project(const Poly3D& poly, const Texture& tex)
{
    Poly2D proj;

    int texWidth = tex.getWidth();
    int texHeight = tex.getHeight();

    for (const auto& vert : poly.verts) {
        float invZ = 1.0f / vert.position.z;

        Vertex2D projVert;

        projVert.pos = project(vert.position);
        projVert.uz = (vert.uv.x * texWidth) * invZ;
        projVert.vz = (vert.uv.y * texHeight) * invZ;
        projVert.iz = invZ;

        proj.verts.push_back(projVert);
    }

    proj.fill = poly.fill;

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
        Triangle3DList clippedTris = clipper.clip(tri, nearPlaneDistance, farPlaneDistance);

        if (clippedTris.empty()) {
            continue;
        }

        Texture tex = rasterizer.getTexture(model.getTextureIndex());

        for (const Triangle3& clipped : clippedTris) {

            // Projection
            Triangle2 projected = project(clipped, tex);

            // Viewport clipping
            Triangle2DList vpClippedTriangles = viewportClipper.clip(projected, viewport);

            // Drawing
            for (const auto& vpClipped : vpClippedTriangles) {
                rasterizer.drawTriangle(vpClipped, tex, zTest);
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

        Poly3D poly = world.getPolygon(i);

        // Backface culling
        if (backfaceCulling && Math::distanceToPlane(poly.getPlane(), cam.position) < 0.0f) {
            continue;
        }

        // Transform
        transformToView(poly, cam);

        // 3D clipping
        Poly3D clippedPoly = clipper.clip(poly, nearPlaneDistance, farPlaneDistance);

        if (clippedPoly.verts.empty()) {
            continue;
        }

        Texture tex = rasterizer.getTexture(poly.texId);

        // Projection
        Poly2D projected = project(clippedPoly, tex);

        // Viewport clipping
        Poly2D vpClippedPoly = viewportClipper.clip(projected, viewport);

        // Drawing
        if (fill) {
            count += rasterizer.drawSolidPolygon(vpClippedPoly, zTest, false);
        }
        else {
            count += rasterizer.drawPolygon(vpClippedPoly, tex, zTest);
        }
    }

    return count;
}

int Renderer::drawExtras(const Poly3DList& polys, const Camera& cam)
{
    int count = 0;

    for (auto poly : polys) {
        // Transform
        transformToView(poly, cam);

        // 3D clipping
        Poly3D clippedPoly = clipper.clip(poly, nearPlaneDistance, farPlaneDistance);

        if (clippedPoly.verts.empty()) {
            continue;
        }

        Texture tex = rasterizer.getTexture(poly.texId);

        // Projection
        Poly2D projected = project(clippedPoly, tex);

        // Viewport clipping
        Poly2D vpClippedPoly = viewportClipper.clip(projected, viewport);

        count += rasterizer.drawSolidPolygon(vpClippedPoly, zTest, true);
    }

    return count;
}

void Renderer::drawScene(const World& world, const std::vector<Model>& models, const Poly3DList& extras, const Camera& cam, Data& data)
{
    rasterizer.clearBuffers();
    
    backfaceCulling = true;

    zTest = false;
    data.count = drawWorld(world, cam, fill);

    zTest = true;
    drawExtras(extras, cam);

    zTest = true;
    for (const Model& model : models) {
        data.count += drawModel(model, cam);
    }

    
    
}

void Renderer::drawLabel(const Label& label, const Camera& cam)
{
    Vec3 pos = label.pos;

    // Transform
    pos = transformToView(pos, cam);

    if (pos.z < 1.0f) {
        return;
    }

    // Projection
    Vec2 proj = project(pos);

    debug.drawText(proj.x, proj.y, label.color, label.label);
}

void Renderer::drawScene(const Poly2DList& polys, const Poly2D& cutter)
{
    rasterizer.clearBuffers();

    Texture tex = rasterizer.getTexture(1);

    for (int i = 0; i < polys.size(); i++) {
        if (fill) {
            rasterizer.drawSolidPolygon(polys[i], false, false);
        }
        else {
            rasterizer.drawPolygon(polys[i], tex, false);
        }
    }

    rasterizer.drawSolidPolygon(cutter, false, false);
}
