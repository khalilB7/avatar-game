#include "partage.h"
#include <stdio.h>

int initGame1(Game1 *g, SDL_Renderer *renderer)
{
    g->background = IMG_LoadTexture(renderer, "background.png");

    if (g->background != NULL) {
        SDL_QueryTexture(g->background, NULL, NULL, &g->bg_w, &g->bg_h);
    } else {
        g->bg_w = 1600; 
        g->bg_h = 1200;
    }

    // Set up cameras. They are only 400px wide now because of the split screen!
    g->camera1 = (SDL_Rect){0, 0, 400, 600};
    g->camera2 = (SDL_Rect){0, 0, 400, 600};

    g->nbPlatforms = 3;

    // Fixe
    g->platforms[0].rect = (SDL_Rect){400, 400, 100, 20};
    g->platforms[0].type = 0;

    // Player 1 (Blue) - Starts on the left side of the map
    g->platforms[1].rect = (SDL_Rect){200, 300, 50, 50};
    g->platforms[1].type = 1;

    // Player 2 (Red) - Starts further right on the map
    g->platforms[2].rect = (SDL_Rect){600, 300, 50, 50};
    g->platforms[2].type = 2;

    g->startTime = SDL_GetTicks();
    g->font = TTF_OpenFont("arial.ttf", 20);

    return 1;
}

void updateGame1(Game1 *g)
{
    const Uint8 *state = SDL_GetKeyboardState(NULL);

    // --- PLAYER 1 (BLUE) CONTROLS: ZQSD ---
    // (Using physical scancodes so it automatically maps correctly to your AZERTY keys)
    if (state[SDL_SCANCODE_W]) g->platforms[1].rect.y -= 5; // Z
    if (state[SDL_SCANCODE_S]) g->platforms[1].rect.y += 5; // S
    if (state[SDL_SCANCODE_A]) g->platforms[1].rect.x -= 5; // Q
    if (state[SDL_SCANCODE_D]) g->platforms[1].rect.x += 5; // D

    // --- PLAYER 2 (RED) CONTROLS: ARROWS ---
    if (state[SDL_SCANCODE_UP])    g->platforms[2].rect.y -= 5;
    if (state[SDL_SCANCODE_DOWN])  g->platforms[2].rect.y += 5;
    if (state[SDL_SCANCODE_LEFT])  g->platforms[2].rect.x -= 5;
    if (state[SDL_SCANCODE_RIGHT]) g->platforms[2].rect.x += 5;

    // --- CAMERA 1 (Follows P1) ---
    g->camera1.x = g->platforms[1].rect.x + (g->platforms[1].rect.w / 2) - (g->camera1.w / 2);
    g->camera1.y = g->platforms[1].rect.y + (g->platforms[1].rect.h / 2) - (g->camera1.h / 2);

    if (g->camera1.x < 0) g->camera1.x = 0;
    if (g->camera1.y < 0) g->camera1.y = 0;
    if (g->camera1.x > g->bg_w - g->camera1.w) g->camera1.x = g->bg_w - g->camera1.w;
    if (g->camera1.y > g->bg_h - g->camera1.h) g->camera1.y = g->bg_h - g->camera1.h;

    // --- CAMERA 2 (Follows P2) ---
    g->camera2.x = g->platforms[2].rect.x + (g->platforms[2].rect.w / 2) - (g->camera2.w / 2);
    g->camera2.y = g->platforms[2].rect.y + (g->platforms[2].rect.h / 2) - (g->camera2.h / 2);

    if (g->camera2.x < 0) g->camera2.x = 0;
    if (g->camera2.y < 0) g->camera2.y = 0;
    if (g->camera2.x > g->bg_w - g->camera2.w) g->camera2.x = g->bg_w - g->camera2.w;
    if (g->camera2.y > g->bg_h - g->camera2.h) g->camera2.y = g->bg_h - g->camera2.h;
}

void renderGame1(Game1 *g, SDL_Renderer *renderer)
{
    // ==========================================
    // VIEWPORT 1: LEFT SCREEN (PLAYER 1)
    // ==========================================
    SDL_Rect left_viewport = {0, 0, 400, 600};
    SDL_RenderSetViewport(renderer, &left_viewport);
    SDL_RenderCopy(renderer, g->background, &g->camera1, NULL);

    for(int i = 0; i < g->nbPlatforms; i++) {
        SDL_Rect p = g->platforms[i].rect;
        p.x -= g->camera1.x; // Shift relative to Camera 1
        p.y -= g->camera1.y;

        if(g->platforms[i].type == 0) SDL_SetRenderDrawColor(renderer, 0,255,0,255);      // Green
        else if(g->platforms[i].type == 1) SDL_SetRenderDrawColor(renderer, 0,0,255,255); // Blue (P1)
        else SDL_SetRenderDrawColor(renderer, 255,0,0,255);                               // Red (P2)
        SDL_RenderFillRect(renderer, &p);
    }

    // ==========================================
    // VIEWPORT 2: RIGHT SCREEN (PLAYER 2)
    // ==========================================
    SDL_Rect right_viewport = {400, 0, 400, 600};
    SDL_RenderSetViewport(renderer, &right_viewport);
    SDL_RenderCopy(renderer, g->background, &g->camera2, NULL);

    for(int i = 0; i < g->nbPlatforms; i++) {
        SDL_Rect p = g->platforms[i].rect;
        p.x -= g->camera2.x; // Shift relative to Camera 2
        p.y -= g->camera2.y;

        if(g->platforms[i].type == 0) SDL_SetRenderDrawColor(renderer, 0,255,0,255);
        else if(g->platforms[i].type == 1) SDL_SetRenderDrawColor(renderer, 0,0,255,255);
        else SDL_SetRenderDrawColor(renderer, 255,0,0,255);
        SDL_RenderFillRect(renderer, &p);
    }

    // ==========================================
    // UI / DIVIDER LINE (FULL SCREEN)
    // ==========================================
    SDL_RenderSetViewport(renderer, NULL); // Reset back to the full 800x600 window
    
    // Draw the vertical line right down the middle
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White line
    SDL_RenderDrawLine(renderer, 400, 0, 400, 600);
}

void freeGame1(Game1 *g)
{
    if (g->background) SDL_DestroyTexture(g->background);
    if (g->font) TTF_CloseFont(g->font);
}
