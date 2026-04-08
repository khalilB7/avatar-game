#include "integrated.h"
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL_ttf.h>
#define LEVEL2_WIDTH 4000
#define LEVEL2_HEIGHT 1080

// Global renderer and window (should be initialized in your main.c)
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

static int isMouseOver(SDL_Rect rect, int x, int y) {
    return x >= rect.x && x <= rect.x + rect.w && y >= rect.y && y <= rect.y + rect.h;
}

// Helper to render surfaces in SDL2
void renderSurface(SDL_Texture* tex, SDL_Rect* src, SDL_Rect* dst,SDL_Renderer* renderer) {
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_RenderCopy(renderer, tex, src, dst);
    SDL_DestroyTexture(tex);
}

void afficherJoueurWithCamera(Joueur j, SDL_Rect camera) {
    SDL_Rect renderPos = j.pos;
    renderPos.x -= camera.x;

    renderSurface(j.sprites[j.currentAction][j.frame], NULL, &renderPos);

    SDL_Rect bar = {renderPos.x, renderPos.y - 20, j.health, 10};
    if (j.health <= 0) SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    else SDL_SetRenderDrawColor(renderer, 255, 105, 180, 255);

    SDL_RenderFillRect(renderer, &bar);
}



GameState run_level2() {
    SDL_Surface *backgroundSurf = NULL, *gameOverSurf = NULL, *winSurf = NULL;
    SDL_Texture *backgroundTex = NULL, *pauseTex = NULL, *playTex = NULL;
    SDL_Rect pauseBtnPos = {10, 10, 50, 50};
    SDL_Event event;
    int running = 1, paused = 0, was_moving = 0;
    GameState nextState = STATE_MENU;
    SDL_Rect camera = {0, 0, 1920, 1080};

    // Load Assets as Textures
    backgroundSurf = IMG_Load("backtwila.png");
    backgroundTex = SDL_CreateTextureFromSurface(renderer, backgroundSurf);




    Joueur j;
    initJoueur(&j,renderer);
    e.pos.x = 1500; j.pos.y = 650; e.pos.y = 570;

    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        const Uint8 *keystates = SDL_GetKeyboardState(NULL);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) { nextState = STATE_EXIT; running = 0; }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) { running = 0; }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (isMouseOver(pauseBtnPos, event.button.x, event.button.y)) paused = !paused;
            }
        }

        if (!paused) {
            // Movement Logic
            if (keystates[SDL_SCANCODE_RIGHT]) {
                j.pos.x += keystates[SDL_SCANCODE_B] ? 15 : 5;
                j.currentAction = keystates[SDL_SCANCODE_B] ? 2 : 1;
                was_moving = 1;
            } else if (keystates[SDL_SCANCODE_LEFT]) {
                j.pos.x -= keystates[SDL_SCANCODE_B] ? 15 : 5;
                j.currentAction = keystates[SDL_SCANCODE_B] ? 2 : 1;
                was_moving = 1;
            } else if (keystates[SDL_SCANCODE_SPACE]) {
                j.currentAction = 4;
                was_moving = 0;
            } else {
                if (was_moving) j.currentAction = 0;
                was_moving = 0;
            }

            // Boundary checks
            if (j.pos.x < 0) j.pos.x = 0;
            if (j.pos.x > LEVEL2_WIDTH - j.pos.w) j.pos.x = LEVEL2_WIDTH - j.pos.w;

            
            animerJoueur(&j);
           
        }

        // Camera Follow
        camera.x = (j.pos.x + (j.pos.w / 2)) - (1920 / 2);
        if (camera.x < 0) camera.x = 0;
        if (camera.x > LEVEL2_WIDTH - camera.w) camera.x = LEVEL2_WIDTH - camera.w;

        // Rendering
        SDL_RenderClear(renderer);

        SDL_Rect bgSrc = {camera.x, 0, 1920, 1080};
        SDL_RenderCopy(renderer, backgroundTex, &bgSrc, NULL);

        afficherJoueurWithCamera(j, camera);
        if (j.pos.x >= LEVEL2_WIDTH - 200) { running = 0; nextState = STATE_HIGHSCORE; }
        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    // Cleanup
    SDL_DestroyTexture(backgroundTex);
    SDL_DestroyTexture(pauseTex);
    SDL_DestroyTexture(playTex);
    SDL_FreeSurface(backgroundSurf);
    SDL_FreeSurface(gameOverSurf);
    SDL_FreeSurface(winSurf);
    TTF_CloseFont(font);
    return nextState;
}
