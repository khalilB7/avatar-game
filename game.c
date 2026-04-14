#include "game.h"
#include <stdio.h>

int initGame(Game *g, SDL_Renderer *renderer)
{
    g->background = IMG_LoadTexture(renderer, "background.jpeg");
    
    // NEW: Get the actual dimensions of your background image
    if (g->background != NULL) {
        SDL_QueryTexture(g->background, NULL, NULL, &g->bg_w, &g->bg_h);
    } else {
        g->bg_w = 800; // Fallback just in case the image doesn't load
        g->bg_h = 600;
    }

    g->camera.x = 0;
    g->camera.y = 0;
    g->camera.w = 800;
    g->camera.h = 600;

    g->nbPlatforms = 3;

    // plateforme fixe
    g->platforms[0].rect = (SDL_Rect){200, 400, 100, 20};
    g->platforms[0].type = 0;

    // mobile (the blue one)
    g->platforms[1].rect = (SDL_Rect){400, 300, 100, 20};
    g->platforms[1].type = 1;
    g->platforms[1].dir = 1;

    // destructible
    g->platforms[2].rect = (SDL_Rect){600, 450, 100, 20};
    g->platforms[2].type = 2;

    g->startTime = SDL_GetTicks();
    g->font = TTF_OpenFont("arial.ttf", 20);

    return 1;
}
void updateGame(Game *g)
{
    // 1. Smooth Keyboard Input for 4-way movement
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    
    if (state[SDL_SCANCODE_UP])    g->platforms[1].rect.y -= 5;
    if (state[SDL_SCANCODE_DOWN])  g->platforms[1].rect.y += 5;
    if (state[SDL_SCANCODE_LEFT])  g->platforms[1].rect.x -= 5;
    if (state[SDL_SCANCODE_RIGHT]) g->platforms[1].rect.x += 5;

    // 2. Camera Logic - Now tracking both X and Y!
    g->camera.x = g->platforms[1].rect.x + (g->platforms[1].rect.w / 2) - (g->camera.w / 2);
    g->camera.y = g->platforms[1].rect.y + (g->platforms[1].rect.h / 2) - (g->camera.h / 2);

    // 3. Clamp the camera so it doesn't show black space on any of the 4 borders
    if (g->camera.x < 0) g->camera.x = 0; 
    if (g->camera.y < 0) g->camera.y = 0; 
    
    if (g->camera.x > g->bg_w - g->camera.w) g->camera.x = g->bg_w - g->camera.w; 
    if (g->camera.y > g->bg_h - g->camera.h) g->camera.y = g->bg_h - g->camera.h; 

}void renderGame(Game *g, SDL_Renderer *renderer)
{
    // CHANGED: Instead of passing NULL, we pass &g->camera. 
    // This tells SDL to only grab the portion of the background image that the camera can see.
    SDL_RenderCopy(renderer, g->background, &g->camera, NULL);

    for(int i = 0; i < g->nbPlatforms; i++)
    {
        SDL_Rect p = g->platforms[i].rect;
        
        // This math is exactly the same: shift platforms based on camera position
        p.x -= g->camera.x;
        p.y -= g->camera.y;

        if(g->platforms[i].type == 0)
            SDL_SetRenderDrawColor(renderer, 0,255,0,255);
        else if(g->platforms[i].type == 1)
            SDL_SetRenderDrawColor(renderer, 0,0,255,255);
        else
            SDL_SetRenderDrawColor(renderer, 255,0,0,255);

        SDL_RenderFillRect(renderer, &p);
    }
}void handleScrolling(Game *g, SDL_Event e)
{
    if(e.type == SDL_KEYDOWN)
    {
        switch(e.key.keysym.sym)
        {
            case SDLK_RIGHT: g->camera.x += 10; break;
            case SDLK_LEFT:  g->camera.x -= 10; break;
            case SDLK_UP:    g->camera.y -= 10; break;
            case SDLK_DOWN:  g->camera.y += 10; break;
        }
    }
}

void renderTimer(Game *g, SDL_Renderer *renderer)
{
    Uint32 time = (SDL_GetTicks() - g->startTime) / 1000;

    char txt[50];
    sprintf(txt, "Temps: %d s", time);

    SDL_Color color = {255,255,255};
    SDL_Surface *surf = TTF_RenderText_Solid(g->font, txt, color);
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);

    SDL_Rect pos = {10,10,surf->w,surf->h};
    SDL_RenderCopy(renderer, tex, NULL, &pos);

    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

void renderGuide(SDL_Renderer *renderer, TTF_Font *font)
{
    SDL_Color c = {255,255,0};
    SDL_Surface *surf = TTF_RenderText_Solid(font, "Arrows = Scroll", c);
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);

    SDL_Rect pos = {500,10,surf->w,surf->h};
    SDL_RenderCopy(renderer, tex, NULL, &pos);

    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

void freeGame(Game *g)
{
    SDL_DestroyTexture(g->background);
    TTF_CloseFont(g->font);
}
