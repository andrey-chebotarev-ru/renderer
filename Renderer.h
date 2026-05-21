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





class Renderer
{
public:
    bool fill;


    Renderer(size_t screenWidth, size_t screenHeight, float fov)
        : screenWidth(screenWidth), screenHeight(screenHeight), rasterizer(screenWidth, screenHeight), viewport(0, screenWidth, 0, screenHeight)
    {
        screenCenter = Vec2(float(screenWidth) / 2, screenHeight / 2);
        focusDistance = screenCenter.x / tanf(Math::degToRad(fov) / 2);
        aspect = float(screenWidth) / screenHeight;

        backfaceCulling = false;
        zTest = false;
        fill = false;
    }

    

    void setFramebuffer(int pitch, uint8_t* pixels);
    void setTexture(const Texture& texture, size_t index);
    
    void drawScene(const World& world, const std::vector<Model>& models, const Triangle3DList& extras, const Camera& cam, Data& data);

    

private:
    static const float nearPlaneDistance, farPlaneDistance;
    int screenWidth, screenHeight;

    bool backfaceCulling;
    bool zTest;

    Vec2 screenCenter;
    float focusDistance;
    float aspect;
    
    Viewport viewport;

    Clipper clipper;
    ViewportClipper viewportClipper;
    Rasterizer rasterizer;

    void transformToWorld(Triangle3& tri, const Transform& tr);

    Vec3 transformToView(const Vec3& vec, const Camera& cam);
    void transformToView(Triangle3& tri, const Camera& cam);

    Vec2 project(const Vec3& vec);
    Triangle2 project(const Triangle3& tri, const Texture& tex);
    
    int drawModel(const Model& model, const Camera& cam);
    int drawWorld(const World& world, const Camera& cam, bool fill);
    int drawExtras(const Triangle3DList& tris, const Camera& cam);
};

