#include "Renderer.h";
#include "Geometry.h"

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */



#include <iostream>
#include <vector>
#include <stdio.h>
#include <conio.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

#include "Vector.h"
#include "Camera.h"
#include "Model.h"
#include "RenderFaces.h"
#include "BSP.h"
#include "Debug.h"
#include "Math.h"
#include "PortalRenderer.h"
#include "MapFile.h"
#include "BrushFaceBuilder.h"
#include "TextureConverter.h"
#include "ResourceLoader.h"
#include "PVS.h"
#include "WadLoader.h"
#include "Triangle.h"
#include "Surface.h"

#define VIEWPORT_WIDTH 1600
#define VIEWPORT_HEIGHT 900

static int windowWidth = 0, windowHeight = 0;

 /* We will use this renderer to draw into this window every frame. */
static SDL_Window* window = NULL;
static SDL_Renderer* sdlRenderer = NULL;
static SDL_Texture* sdlFramebufferTexture = NULL;
static TextureManager textureManager;
static Debug debug(&sdlRenderer);
static SurfaceCache surfaceCache;
static Renderer gameRenderer(VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 90.0f, textureManager, surfaceCache, debug);
static Camera camera;
static std::vector<Model> models;
static RenderFaces world;
static Tree tree;
static BSP bsp(tree);
static ResourceLoader loader(tree);
static PortalRenderer portalRenderer(gameRenderer, bsp);
static PVS pvs(tree);
static WadLoader wadLoader(textureManager);


static bool fill = false;
static bool drawPortals = false;
static bool renderOnlyCurrentLeaf = false;
static float pitchOffset = 0.0f;

static BrushFaceBuilder faceBuilder;
static TextureConverter converter;

PVSTable pvsTable;
BoolList* pvsRow = nullptr;


static bool fullscreen = true;

static bool leaked = false;
static bool update = true;
static bool updatePvs = true;

const double targetFrameTime = 1000.0 / 100.0;

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    std::string name, path;

    std::cout << "Map name: ";
    std::cin >> name;

    Vec3List playerStart;

    StringList textureNames;

    if (!loader.load(std::format("maps/{}.bin", name), textureNames, pvsTable, playerStart)) {
      
        Map map;
        MapFile mapFile;
        
        if (!mapFile.load(std::format("sources/{}.map", name))) {
            std::cerr << std::format("Could not open 'sources/{}.map'", name);
            _getch();
            return SDL_APP_FAILURE;
        }
        
        mapFile.parse(map);
        
        FaceList faces;
        textureNames = faceBuilder.build(faces, playerStart, map);

        for (const auto& textureName : textureNames) {
            Debug::printf("Texture name: '{}'", textureName);
        }

        leaked = bsp.compile(faces, playerStart);

        std::cout << "Calculating PVS...\n";
        pvs.calculate(pvsTable);

        loader.save(std::format("maps/{}.bin", name), textureNames, pvsTable, playerStart);

        std::cout << "Compilation complete.\n";

        //return SDL_APP_FAILURE;
    }

    

    IndicesList remap = wadLoader.load(textureNames, std::format("maps/{}.wad", name), "common");

    for (auto& face : tree.faces) {
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

    for (auto& face : tree.faces) {
        const Texture& tex = textureManager.get(face.face.texIdx);
        face.face.surfIdx = surfaceCache.cache(face.face, tex);
    }

    std::cout << "Press any key...\n";
    _getch();

    camera.position = playerStart[0];
    
   

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    const SDL_DisplayMode* displayMode;

    if (!(displayMode = SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay()))) {
        SDL_Log("Couldn't get currend display mode: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

#ifdef _DEBUG
    fullscreen = false;
#endif

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

    

    if (fullscreen) {
        SDL_SetWindowFullscreen(window, true);
    }
    
    SDL_SetWindowRelativeMouseMode(window, true);
    SDL_RaiseWindow(window);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

static float boxYawSpeed = 0, boxPitchSpeed = 0;
static bool boxMode = false;

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
        if (event->key.key == SDLK_PAGEUP) {
            pitchOffset += 10;
        }
        if (event->key.key == SDLK_PAGEDOWN) {
            pitchOffset -= 10;
        }
        if (event->key.key == SDLK_END) {
            pitchOffset = 0;
        }
        if (event->key.key == SDLK_LCTRL) {
            ctrlPressed = true;
        }
        if (event->key.key == SDLK_P) {
            renderOnlyCurrentLeaf = !renderOnlyCurrentLeaf;
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
        if (event->key.key == SDLK_V) {
            updatePvs = !updatePvs;
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

    gameRenderer.setPitchOffset(pitchOffset);

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
           // portalRenderer.render(camera);
        }

        if (updatePvs) {
            pvsRow = &pvsTable[bsp.getLeafIdx(camera.position)];
        }

        bsp.render(camera.position, world, *pvsRow, drawPortals, renderOnlyCurrentLeaf);


        
       

        count = gameRenderer.drawScene(world, models, camera, fill);

        /*
        RenderFaces rf;
        Face f;
        FaceList fs;
        f.poly = Poly3D();
        f.poly.addVert(Vertex3D(Vec3(-100, -100, 100), Vec2(-64, -96)));
        f.poly.addVert(Vertex3D(Vec3(100, -100, 100), Vec2(0, -96)));
        f.poly.addVert(Vertex3D(Vec3(100, 100, 100), Vec2(0, 0)));
        f.poly.addVert(Vertex3D(Vec3(-100, 100, 100), Vec2(-64, 0)));
        std::reverse(f.poly.verts.begin(), f.poly.verts.end());
        f.texIdx = 23;
        fs.push_back(f);
        rf.addGeometry(fs);

        gameRenderer.drawWorld(rf, camera, false);
        */


   
        if (drawPortals) {
            //gameRenderer.drawExtras(bsp.renderPortals(camera.position), camera);
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

    if (leaked) {
        SDL_RenderDebugText(sdlRenderer, 10, 60, "Leak detected!");
    }

    if (!update) {
        SDL_RenderDebugText(sdlRenderer, 10, 70, "Hold");
    }
    

    SDL_RenderPresent(sdlRenderer);  /* put it all on the screen! */

    double elapsed = double(SDL_GetTicks() - now);

    if (elapsed < targetFrameTime) {
        SDL_Delay(Uint32(targetFrameTime) - elapsed);
    }

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    SDL_DestroyTexture(sdlFramebufferTexture);

    /* SDL will clean up the window/renderer for us. */
}