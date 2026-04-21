#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "minimap.h"

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

    SDL_Window *window = SDL_CreateWindow("Game Collision Fixed", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Minimap minimap;
    initMinimap(&minimap, renderer);

    // Background Image
    SDL_Surface *bgSurf = IMG_Load("MiniMap.jpeg");
    SDL_Texture *bgTex = SDL_CreateTextureFromSurface(renderer, bgSurf);
    SDL_FreeSurface(bgSurf);

    // Player (Mrebbe3 akhdher)
    SDL_Rect player = {50, 450, 40, 40};

    // --- EL-COLLISION HNA ---
    // N-hattou rects fou9 el-blays elli fihom el-7jar f-taswira
    SDL_Rect obstacles[] = {
        {0, 510, 250, 90},    // El-9a3 3al issar
        {500, 510, 300, 90},  // El-9a3 3al imine
        {130, 360, 240, 40},  // El-platforme el-loula
        {470, 240, 150, 30},  // Platforme sghira fil wast
        {0, 0, 800, 10},      // Essama (Screen Top)
        {0, 0, 10, 600},      // L-7it l-issar
        {790, 0, 10, 600}     // L-7it l-imine
    };
    int numObs = 7;

    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
        }

        const Uint8 *ks = SDL_GetKeyboardState(NULL);
        SDL_Rect next = player;
        int speed = 5;

        if (ks[SDL_SCANCODE_RIGHT]) next.x += speed;
        if (ks[SDL_SCANCODE_LEFT])  next.x -= speed;
        if (ks[SDL_SCANCODE_UP])    next.y -= speed;
        if (ks[SDL_SCANCODE_DOWN])  next.y += speed;

        // Check collision m3a kol obstacle
        int hit = 0;
        for (int i = 0; i < numObs; i++) {
            if (checkCollision(next, obstacles[i])) {
                hit = 1;
                break;
            }
        }

        // Ken ma dakhalech fi 7it, n-7arkouh
        if (!hit) {
            player = next;
        }

        updateMinimap(&minimap, player);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // 1. Nirsmou el-Background
        SDL_RenderCopy(renderer, bgTex, NULL, NULL);

        // 2. Nirsmou el-Player
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &player);

        // 3. Nirsmou el-Minimap
        renderMinimap(&minimap, renderer);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    freeMinimap(&minimap);
    SDL_DestroyTexture(bgTex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
