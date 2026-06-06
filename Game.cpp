#include "Renderer.h";
#include "Geometry.h"

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */



#include <iostream>
#include <vector>
#include <stdio.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

#include "Vector.h"
#include "Camera.h"
#include "Model.h"
#include "Constructor.h"
#include "World.h"
#include "BSP.h"
#include "Debug.h"
#include "Math.h"
#include "PortalRenderer.h"
#include "MapFile.h"

#define VIEWPORT_WIDTH 1024
#define VIEWPORT_HEIGHT 768

static int windowWidth = 0, windowHeight = 0;

 /* We will use this renderer to draw into this window every frame. */
static SDL_Window* window = NULL;
static SDL_Renderer* sdlRenderer = NULL;
static SDL_Texture* sdlFramebufferTexture = NULL;
static TextureManager textureManager;
static Debug debug(&sdlRenderer);
static Renderer gameRenderer(VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 90.0f, textureManager, debug);
static Constructor constructor(textureManager);
static Camera camera;
static std::vector<Model> models;
static World world;
static BSP bsp;
static PortalRenderer portalRenderer(gameRenderer, bsp);
static bool fill = false;
static bool drawPortals = false;

static bool fullscreen = true;

static int randomAngle()
{
    return SDL_rand(360);
}


struct Angles
{
    static const int count = 12;

    int values[count];
    bool loaded = false, saved = false;

    Angles()
    {
        for (int i = 0; i < count; i++) {
            values[i] = randomAngle();
        }
    }

    void save()
    {
        FILE* f;
        fopen_s(&f, "angles.dat", "w");

        if (!f) {
            return;
        }

        fwrite(values, sizeof(int), count, f);
        fclose(f);

        saved = true;
    }

    void load()
    {
        FILE* f;
        fopen_s(&f, "angles.dat", "r");

        if (!f) {
            return;
        }

        fread(values, sizeof(int), count, f);
        fclose(f);

        loaded = true;
    }
};

static Angles angles;
static bool leaked = false;
static bool update = true;

bool loadTexture(const char* name)
{
    SDL_Surface* loadedTexture;
    SDL_Surface* convertedTexture;


    char buf[256];

    sprintf_s(buf, "textures/%s.jpg", name);

    if (!(loadedTexture = IMG_Load(buf))) {
        SDL_Log("Couldn't load image: %s", SDL_GetError());
        return false;
    }

    if (!(convertedTexture = SDL_ConvertSurface(loadedTexture, SDL_PIXELFORMAT_RGBA32))) {
        SDL_DestroySurface(loadedTexture);
        SDL_Log("Couldn't convert texture: %s", SDL_GetError());
        return false;
    }

    SDL_DestroySurface(loadedTexture);


    SDL_LockSurface(convertedTexture);
    
    textureManager.add(name, convertedTexture->w, convertedTexture->h, convertedTexture->pitch, (uint8_t*)convertedTexture->pixels);
    
    SDL_UnlockSurface(convertedTexture);
    SDL_DestroySurface(convertedTexture);

    return true;
}

bool loadTextures()
{
    bool loaded = loadTexture("brick1");

    loaded = loaded && loadTexture("brick2");
    loaded = loaded && loadTexture("cells1");
    loaded = loaded && loadTexture("cells2");
    loaded = loaded && loadTexture("cells3");
    loaded = loaded && loadTexture("rock1");
    loaded = loaded && loadTexture("lava1");
    loaded = loaded && loadTexture("wood1");

    return loaded;
}


/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    if (!loadTextures()) {
        return SDL_APP_FAILURE;
    }


    MapFile mapFile;
    mapFile.load("1.map");
    mapFile.parse();

    return SDL_APP_FAILURE;

    angles.load();

    // Model(Vec3(60, 0, 60), Vec3(0, 0, 0), 2, constructor.cube(Vec2(1, 1), Vec3(20, 20, 20), 0))

    // v1
    Vec2List points = {
        {0, 0},
        {50, 0},
        {50, 50},
        {100, 50},
        {100, 0},
        {200, 50},
        {250, 50},
        {250, 100},
        {100, 100},
        {100, 120},
        {0, 120},
       
    };

    Vec2List points2 = {
        {250, 50},
        {350, 50},
        {350, 200},
        {300, 200},
        {300, 300},
        {350, 300}, //p
        {350, 550},
        {200, 550},

        {200, 450},
        {300, 450},
        {300, 350}, //p
        {250, 350},
        {250, 150},
        {300, 150},
        {300, 100},
        {250, 100},

    };

    Vec2List points3 = {
        {100, 120},
        {100, 150},
        {200, 150},
        {200, 450},
        {200, 550},
        {0, 550},
        {0, 120}
    };
    




    FaceList list = constructor.map(points, 100, "brick1", "cells1", "cells2");
    list.erase(list.begin() + 7);
    list.erase(list.begin() + 9);

    
   

    FaceList list2 = constructor.map(points2, 100, "brick2", "rock1", "cells3");
    list2.erase(list2.begin());
    list2.erase(list2.begin() + 7);

    FaceList list3 = constructor.map(points3, 100, "rock1", "lava1", "cells1");
    list3.erase(list3.begin());
    list3.erase(list3.begin() + 3);
    
    list.insert(list.end(), list2.begin(), list2.end());
    list.insert(list.end(), list3.begin(), list3.end());

    
    Poly3DList cube1 = constructor.polyCube(Vec3(10, 300, 10));

    constructor.insert(list, cube1, "brick2", Vec3(100, 0, 200));
    constructor.insert(list, cube1, "brick2", Vec3(100, 0, 250));
    constructor.insert(list, cube1, "brick2", Vec3(100, 0, 300));
    
    Poly3DList cube2 = constructor.polyCube(Vec3(50, 50, 50));

    constructor.insert(list, cube2, "cells2", Vec3(50, 0, 250), Vec3(Math::degToRad(0), Math::degToRad(0), Math::degToRad(0)));
    

   
    

    //list.insert(list.end(), list3.begin(), list3.end());

   
    //list.insert(list.end(), wall.begin(), wall.end());
    //list.insert(list.end(), wall.begin(), wall.end());

    Vec3 pos = { 20, 0, 70 };

    leaked = bsp.compile(list, pos);

    camera.position = pos;
    //camera.yaw = 0;
    
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    const SDL_DisplayMode* displayMode;

    if (!(displayMode = SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay()))) {
        SDL_Log("Couldn't get currend display mode: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    /*
    if (fullscreen) {
        windowWidth = displayMode->w;
        windowHeight = displayMode->h;
    }
    else {
        windowWidth = VIEWPORT_WIDTH;
        windowHeight = VIEWPORT_HEIGHT;
    }
    */

    windowWidth = VIEWPORT_WIDTH;
    windowHeight = VIEWPORT_HEIGHT;


    if (!SDL_CreateWindowAndRenderer("Game", windowWidth, windowHeight, SDL_WINDOW_RESIZABLE, &window, &sdlRenderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetRenderLogicalPresentation(sdlRenderer, windowWidth, windowHeight, SDL_LOGICAL_PRESENTATION_LETTERBOX);



    sdlFramebufferTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    if (!sdlFramebufferTexture) {
        SDL_Log("Couldn't create streaming texture: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetWindowRelativeMouseMode(window, true);

    if (fullscreen) {
        SDL_SetWindowFullscreen(window, true);
    }
    

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

static float boxYawSpeed = 0, boxPitchSpeed = 0;
static bool boxMode = false;
static bool renderOnlyCurrentLeaf = false;

static bool moveForward = false, moveBackward = false, moveLeft = false, moveRight = false;

static bool ctrlPressed = false;
static bool drawAABBs = false;
/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    switch (event->type) {
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;
    case SDL_EVENT_KEY_DOWN:
        if (event->key.key == SDLK_LCTRL) {
            ctrlPressed = true;
        }
        if (event->key.key == SDLK_P && ctrlPressed) {
            angles.save();
        }
        if (event->key.key == SDLK_E) {
            boxMode = !boxMode;
        }
        if (event->key.key == SDLK_Q) {
            fill = !fill;
        }
        if (event->key.key == SDLK_R) {
            renderOnlyCurrentLeaf = !renderOnlyCurrentLeaf;
        }
        if (event->key.key == SDLK_F) {
            drawPortals = !drawPortals;
        }
        if (event->key.key == SDLK_C) {
            drawAABBs = !drawAABBs;
        }
        if (event->key.key == SDLK_U) {
            update = !update;
        }
        if (event->key.key == SDLK_W) {
            moveForward = true;
        }
        if (event->key.key == SDLK_S) {
            moveBackward = true;
        }
        if (event->key.key == SDLK_D) {
            moveRight = true;
        }
        if (event->key.key == SDLK_A) {
            moveLeft = true;
        }
        if (event->key.key == SDLK_LEFT) {
            boxYawSpeed = 60;
        }
        if (event->key.key == SDLK_RIGHT) {
            boxYawSpeed = -60;
        }
        if (event->key.key == SDLK_UP) {
            boxPitchSpeed = -60;
        }
        if (event->key.key == SDLK_DOWN) {
            boxPitchSpeed = 60;
        }
        if (event->key.key == SDLK_ESCAPE) {
            return SDL_APP_SUCCESS;
        }
        break;
    case SDL_EVENT_KEY_UP:
        if (event->key.key == SDLK_LCTRL) {
            ctrlPressed = false;
        }
        if (event->key.key == SDLK_W) {
            moveForward = false;
        }
        if (event->key.key == SDLK_S) {
            moveBackward = false;
        }
        if (event->key.key == SDLK_D) {
            moveRight = false;
        }
        if (event->key.key == SDLK_A) {
            moveLeft = false;
        }
        if (event->key.key == SDLK_LEFT || event->key.key == SDLK_RIGHT) {
            boxYawSpeed = 0;
        }
        if (event->key.key == SDLK_UP || event->key.key == SDLK_DOWN) {
            boxPitchSpeed = 0;
        }
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (event->button.button == SDL_BUTTON_RIGHT) {
        }
        if (event->button.button == SDL_BUTTON_LEFT) {
        }
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        break;
        
    case SDL_EVENT_MOUSE_MOTION:
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        break;
    }
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void* appstate)
{
    SDL_FRect dst_rect;
    const Uint64 now = SDL_GetTicks();
    SDL_Surface* surface = NULL;

    uint8_t* pixels = nullptr;
    int pitch;

    static float moveSpeed = 90.0f;

    float dx, dy;
    float sensitivity = 0.1;
    SDL_GetRelativeMouseState(&dx, &dy);
    


    static int frames = 0;
    static double fpsTimer = 0.0;
    static double fps = 0.0;
    static Uint64 previousCounter = SDL_GetPerformanceCounter();
    Uint64 currentCounter = SDL_GetPerformanceCounter();
    double deltaTime = (double)(currentCounter - previousCounter) / SDL_GetPerformanceFrequency();

    previousCounter = currentCounter;

    fpsTimer += deltaTime;
    frames++;

    if (fpsTimer >= 1.0) {
        fps = frames / fpsTimer;

        frames = 0;
        fpsTimer = 0.0;
    }

    if (dx != 0) {
        camera.yaw(dx * sensitivity);
    }

    if (dy != 0) {
        camera.pitch(-dy * sensitivity);
    }

    Vec3 move = Vec3(0, 0, 0);
    if (moveForward) {
        move += camera.getForwardVector();
    }
    if (moveBackward) {
        move -= camera.getForwardVector();
    }
    if (moveRight) {
        move += camera.getRightVector();
    }
    if (moveLeft) {
        move -= camera.getRightVector();
    }

    if (move.length() > 0.0f) {
        move = Vec3::normalize(move);
        if (!boxMode) {
            camera.move(move * moveSpeed * deltaTime);
        }
        else {
            models[0].move(move * moveSpeed * deltaTime);
        }
        
    }

    if (boxYawSpeed != 0) {
        models[0].yaw(boxYawSpeed * deltaTime);
    }

    if (boxPitchSpeed != 0) {
        models[0].pitch(boxPitchSpeed * deltaTime);
    }

    int count = 0;

    if (SDL_LockTexture(sdlFramebufferTexture, NULL, (void**)&pixels, &pitch))
    {
        gameRenderer.setFramebuffer(pitch, pixels);

        world.clear();

        if (update) {
            portalRenderer.render(camera);
        }
        
        bsp.render(camera.position, world, drawPortals);



       

        count = gameRenderer.drawScene(world, models, camera, fill);
   
        if (drawPortals) {
            //gameRenderer.drawExtras(bsp.renderPortals(camera.position), camera);
        }

        
        if (drawAABBs) {
            portalRenderer.drawAABBs(camera);
            for (const auto& intersection : portalRenderer.intersections) {
                gameRenderer.drawAABB(intersection, Color(255, 255, 255));
            }
        }

        SDL_UnlockTexture(sdlFramebufferTexture);  /* upload the changes (and frees the temporary surface)! */
    }

    

    /* as you can see from this, rendering draws over whatever was drawn before it. */
    SDL_SetRenderDrawColor(sdlRenderer, 66, 66, 66, SDL_ALPHA_OPAQUE);  /* grey, full alpha */
    SDL_RenderClear(sdlRenderer);  /* start with a blank canvas. */

    dst_rect.x = dst_rect.y = 0;
    dst_rect.w = windowWidth;
    dst_rect.h = windowHeight;
    SDL_RenderTexture(sdlRenderer, sdlFramebufferTexture, NULL, &dst_rect);

    if (drawPortals) {
        gameRenderer.drawLabels(bsp.renderPortalLabels(camera.position), camera);
    }
    
    int leafIdx = bsp.getLeafIdx(camera.position);
    const Leaf& leaf = bsp.getLeaf(leafIdx);
    
    SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderDebugTextFormat(sdlRenderer, 10, 10, "FPS: %.2f", fps);
    SDL_RenderDebugTextFormat(sdlRenderer, 10, 20, "Triangles: %i", count);
    SDL_RenderDebugTextFormat(sdlRenderer, 10, 30, "Leaf: %i (%s) %s", leafIdx, leaf.valid ? "Inside" : "Void", leaf.outside ? "(Outside leaf)" : "");

    if (angles.loaded) {
        SDL_RenderDebugText(sdlRenderer, 10, 40, "Angles loaded");
    }

    if (angles.saved) {
        SDL_RenderDebugText(sdlRenderer, 10, 50, "Angles saved");
    }

    if (leaked) {
        SDL_RenderDebugText(sdlRenderer, 10, 60, "Leak detected!");
    }

    if (!update) {
        SDL_RenderDebugText(sdlRenderer, 10, 70, "Hold");
    }
    

    SDL_RenderPresent(sdlRenderer);  /* put it all on the screen! */

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    SDL_DestroyTexture(sdlFramebufferTexture);

    /* SDL will clean up the window/renderer for us. */
}