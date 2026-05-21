#include "Renderer.h";

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */



#include <iostream>
#include <vector>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

#include "Vector.h"
#include "Camera.h"
#include "Model.h"
#include "Constructor.h"
#include "World.h"
#include "BSP.h"

#define VIEWPORT_WIDTH 1024
#define VIEWPORT_HEIGHT 768

static int windowWidth = 0, windowHeight = 0;

 /* We will use this renderer to draw into this window every frame. */
static SDL_Window* window = NULL;
static SDL_Renderer* sdlRenderer = NULL;
static SDL_Texture* sdlFramebufferTexture = NULL;
static Renderer gameRenderer(VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 90.0f);
static Camera camera;
static std::vector<Model> models = { Model(Vec3(60, 0, 60), Vec3(0, 0, 0), 2, Constructor::cube(Vec2(1, 1), Vec3(20, 20, 20), 0)) };
static World world;
static BSP bsp;
static bool fill = false;
static bool portals = false;

static std::vector<SDL_Surface*> sdlTexture(100);


bool loadTexture(const char* filename, int index)
{
    SDL_Surface* loadedTexture = NULL;

    if (!(loadedTexture = IMG_Load(filename))) {
        SDL_Log("Couldn't load image: %s", SDL_GetError());
        return false;
    }

    if (!(sdlTexture[index] = SDL_ConvertSurface(loadedTexture, SDL_PIXELFORMAT_RGBA32))) {
        SDL_DestroySurface(loadedTexture);
        SDL_Log("Couldn't convert texture: %s", SDL_GetError());
        return false;
    }

    SDL_DestroySurface(loadedTexture);


    SDL_LockSurface(sdlTexture[index]);
    
    gameRenderer.setTexture(Texture(sdlTexture[index]->w, sdlTexture[index]->h, sdlTexture[index]->pitch, (uint8_t*)sdlTexture[index]->pixels), index);

    return true;
}

void freeTexture()
{
    for (auto texture : sdlTexture) {
        SDL_UnlockSurface(texture);
        SDL_DestroySurface(texture);
    }
}




/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
   

    Vec2List points = {
        {0, 0},
        {100, 0},
        {100, 150},
        {120, 150},
        {120, 100},
        {200, 100},
        {200, 0},
        {300, 0},
        {450, 60},
        {500, 110},
        {300, 200},
        {-50, 200}
    };

    /*
    Vec2List points = {
        {0, 0},
        {100, 0},
        {100, 100},
        {200, 50},
        {200, 200},
        {120, 200},
        {220, 250},
        {110, 400},
        {50, 400},
        {0, 250}
    };
    */

    Triangle3DList list = Constructor::map(points, 100, 1);

    bsp.compile(list);

    camera.position = Vec3(20, 0, 20);
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

    windowWidth = displayMode->w;
    windowHeight = displayMode->h;


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

    
    if (!loadTexture("wall.jpg", 1) || !loadTexture("box.jpg", 2) || !loadTexture("floor.jpg", 3) || !loadTexture("ceil.jpg", 4)) {
        return SDL_APP_FAILURE;
    }
   
    SDL_SetWindowRelativeMouseMode(window, true);
    SDL_SetWindowFullscreen(window, true);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

static float boxYawSpeed = 0, boxPitchSpeed = 0;
static bool boxMode = false;

static bool moveForward = false, moveBackward = false, moveLeft = false, moveRight = false;

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    switch (event->type) {
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;
    case SDL_EVENT_KEY_DOWN:
        if (event->key.key == SDLK_E) {
            boxMode = !boxMode;
        }
        if (event->key.key == SDLK_Q) {
            fill = !fill;
        }
        if (event->key.key == SDLK_F) {
            portals = !portals;
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
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (event->button.button == SDL_BUTTON_RIGHT) {
        }
        break;
    case SDL_EVENT_MOUSE_MOTION:
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

    static float moveSpeed = 50.0f;

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

    Data data;

    if (SDL_LockTexture(sdlFramebufferTexture, NULL, (void**)&pixels, &pitch))
    {
        
        world.clear();
        bsp.render(camera, world);

        gameRenderer.fill = fill;
        gameRenderer.setFramebuffer(pitch, pixels);

        gameRenderer.drawScene(world, models, (portals ? bsp.portalTris : Triangle3DList()), camera, data);


        SDL_UnlockTexture(sdlFramebufferTexture);  /* upload the changes (and frees the temporary surface)! */
    }

    /* as you can see from this, rendering draws over whatever was drawn before it. */
    SDL_SetRenderDrawColor(sdlRenderer, 66, 66, 66, SDL_ALPHA_OPAQUE);  /* grey, full alpha */
    SDL_RenderClear(sdlRenderer);  /* start with a blank canvas. */

    dst_rect.x = dst_rect.y = 0;
    dst_rect.w = windowWidth;
    dst_rect.h = windowHeight;
    SDL_RenderTexture(sdlRenderer, sdlFramebufferTexture, NULL, &dst_rect);

    SDL_SetRenderDrawColor(sdlRenderer, 51, 102, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderDebugTextFormat(sdlRenderer, 10, 10, "%.2f", fps);
    SDL_RenderDebugTextFormat(sdlRenderer, 10, 20, "%i", data.count);
    SDL_RenderDebugTextFormat(sdlRenderer, 10, 30, "x: %f y: %f z: %f", camera.position.x, camera.position.y, camera.position.z);
    SDL_RenderDebugTextFormat(sdlRenderer, 10, 40, "%i", bsp.leaf(camera));
    /*
    Triangle2 triProj = gameRenderer.projectedTrianglesList[0];
    Triangle3 tri = gameRenderer.clippedTrianglesList[0];


    SDL_SetRenderDrawColor(sdlRenderer, 51, 102, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderDebugTextFormat(sdlRenderer, 10, 10, "Vert: (%f %f %f) (%f %f %f) (%f %f %f)", tri.v0.position.x, tri.v0.position.y, tri.v0.position.z, tri.v1.position.x, tri.v1.position.y, tri.v1.position.z, tri.v2.position.x, tri.v2.position.y, tri.v2.position.z);
    SDL_RenderDebugTextFormat(sdlRenderer, 10, 50, "Proj: (%f %f %f) (%f %f %f) (%f %f %f)", triProj.v0.position.x, triProj.v0.position.y, triProj.v0.invZ, triProj.v1.position.x, triProj.v1.position.y, triProj.v1.invZ, triProj.v2.position.x, triProj.v2.position.y, triProj.v2.invZ);
    */
    SDL_RenderPresent(sdlRenderer);  /* put it all on the screen! */

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    SDL_DestroyTexture(sdlFramebufferTexture);
    freeTexture();
    /* SDL will clean up the window/renderer for us. */
}