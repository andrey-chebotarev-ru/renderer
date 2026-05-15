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

#define VIEWPORT_WIDTH 1024
#define VIEWPORT_HEIGHT 768

static int windowWidth = 0, windowHeight = 0;

 /* We will use this renderer to draw into this window every frame. */
static SDL_Window* window = NULL;
static SDL_Renderer* sdlRenderer = NULL;
static SDL_Texture* sdlFramebufferTexture = NULL;
static Renderer gameRenderer(VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 90.0f);
static Camera camera;
static std::vector<Model> models = {};
static World world;

static std::vector<SDL_Surface*> sdlTexture(100);


// // { Constructor::test(Vec3(0, 0, 100), Vec3(0, 0, 0), 1) };

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

    
    world.addGeometry(Constructor::room(Vec2(1, 1), Vec3(256, 256, 256), 1));
    models.push_back(Model(Vec3(0, 0, 0), Vec3(0, 0, 0), 2, Constructor::cube(Vec2(1.0f, 1.0f), Vec3(40, 40, 40), 2)));



    
    //camera.position = Vec3(0, 0, 0);
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

    
    if (!loadTexture("texture.jpg", 1) || !loadTexture("texture2.jpg", 2)) {
        return SDL_APP_FAILURE;
    }
   
    SDL_SetWindowRelativeMouseMode(window, true);
    SDL_SetWindowFullscreen(window, true);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

static float forwardDistance = 0, rightDistance = 0, yawSpeed = 0, pitchSpeed = 0;
static float boxForwardSpeed = 0, boxYawSpeed = 0, boxPitchSpeed = 0;

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    static Vec2 lastMousePosition;

    static bool dragging = false;

    switch (event->type) {
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;
    case SDL_EVENT_KEY_DOWN:
        if (event->key.key == SDLK_PAGEUP) {
            pitchSpeed = 0.2f;
        }
        if (event->key.key == SDLK_PAGEDOWN) {
            pitchSpeed = -0.2f;
        }
        if (event->key.key == SDLK_W) {
            forwardDistance = 0.4;
        }
        if (event->key.key == SDLK_S) {
            forwardDistance = -0.4;
        }
        if (event->key.key == SDLK_D) {
            rightDistance = 0.4;
        }
        if (event->key.key == SDLK_A) {
            rightDistance = -0.4;
        }
        if (event->key.key == SDLK_LEFT) {
            boxYawSpeed = 0.2;
        }
        if (event->key.key == SDLK_RIGHT) {
            boxYawSpeed = -0.2;
        }
        if(event->key.key == SDLK_UP) {
            boxPitchSpeed = -0.2;
        }
        if (event->key.key == SDLK_DOWN) {
            boxPitchSpeed = 0.2;
        }
        if (event->key.key == SDLK_R) {
            boxForwardSpeed = 0.2;
        }
        if (event->key.key == SDLK_F) {
            boxForwardSpeed = -0.2;
        }
        if (event->key.key == SDLK_ESCAPE) {
            return SDL_APP_SUCCESS;
        }
        break;
    case SDL_EVENT_KEY_UP:
        if (event->key.key == SDLK_W || event->key.key == SDLK_S) {
            forwardDistance = 0;
        }
        if (event->key.key == SDLK_D || event->key.key == SDLK_A) {
            rightDistance = 0;
        }
        if (event->key.key == SDLK_LEFT || event->key.key == SDLK_RIGHT) {
            boxYawSpeed = 0;
        }
        if (event->key.key == SDLK_UP || event->key.key == SDLK_DOWN) {
            boxPitchSpeed = 0;
        }
        if (event->key.key == SDLK_R || event->key.key == SDLK_F) {
            boxForwardSpeed = 0;
        }
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (event->button.button == SDL_BUTTON_RIGHT) {
            dragging = true;
            lastMousePosition.x = event->button.x;
            lastMousePosition.y = event->button.y;
        }
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (event->button.button == SDL_BUTTON_RIGHT) {
            dragging = false;
        }
        break;
    case SDL_EVENT_MOUSE_MOTION:
        if (dragging) {
            
        }
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

    static float moveSpeed = 200.0f;

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


    if (forwardDistance != 0) {
        camera.moveForward(forwardDistance * moveSpeed * deltaTime);
    }

    if (rightDistance != 0) {
        camera.moveRight(rightDistance * moveSpeed * deltaTime);
    }

    if (boxYawSpeed != 0) {
        models[0].yaw(boxYawSpeed * moveSpeed * deltaTime);
    }

    if (boxPitchSpeed != 0) {
        models[0].pitch(boxPitchSpeed * moveSpeed * deltaTime);
    }

    if (boxForwardSpeed != 0) {
        models[0].move(Vec3(0, 0, boxForwardSpeed * moveSpeed * deltaTime));
    }
    
    Data data;
    if (SDL_LockTexture(sdlFramebufferTexture, NULL, (void**)&pixels, &pitch))
    {

        gameRenderer.setFramebuffer(pitch, pixels);
        gameRenderer.drawScene(world, models, camera, data);



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