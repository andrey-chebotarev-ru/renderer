#include <vector>
#include <assert.h>


#include "Vector.h"
#include "Math.h"
#include "ViewportClipper.h"
#include "Rasterizer.h"
#include "Model.h"
#include "Camera.h"
#include "Clipper.h"
#include "World.h"

struct Data
{
    int count;
};

struct Transform
{
    Vec3 position;
    Vec3 rotation;
    Vec3 scale;

    Transform(Vec3 position, Vec3 rotation, Vec3 scale)
        : position(position), rotation(rotation), scale(scale)
    {
    }
};



class Renderer
{
public:

    Renderer(size_t screenWidth, size_t screenHeight, float fov)
        : screenWidth(screenWidth), screenHeight(screenHeight), rasterizer(screenWidth, screenHeight), viewport(0, screenWidth, 0, screenHeight)
    {
        screenCenter = Vec2(float(screenWidth) / 2, screenHeight / 2);
        focusDistance = screenCenter.x / tanf(Math::degToRad(fov) / 2);
        aspect = float(screenWidth) / screenHeight;
    }

    

    void setFramebuffer(int pitch, uint8_t* pixels);
    void setTexture(const Texture& texture, size_t index);
    
    void drawScene(const World& world, const std::vector<Model>& models, const Camera& cam, Data& data);

    

private:
    static const float nearPlaneDistance, farPlaneDistance;
    int screenWidth, screenHeight;

    Vec2 screenCenter;
    float focusDistance;
    float aspect;
    
    Viewport viewport;

    Clipper clipper;
    ViewportClipper viewportClipper;
    Rasterizer rasterizer;

    Vec3 rotateY(Vec3& vec, float yaw);
    Vec3 rotateX(Vec3& vec, float pitch);
    Vec3 rotateZ(Vec3& vec, float roll);

    void transformToWorld(Vec3& vec, const Transform& tr);
    void transformToWorld(Triangle3& tri, const Transform& tr);

    void transformToView(Vec3& vec, const Camera& cam);
    void transformToView(Triangle3& tri, const Camera& cam);

    Vec2 project(const Vec3& vec);
    Triangle2 project(const Triangle3& tri, const Texture& tex);
    
    float testTriangle(Triangle3 triangle, Vec3 position);

    int drawModel(const Model& model, const Camera& cam);
    int drawWorld(const World& world, const Camera& cam);
};

