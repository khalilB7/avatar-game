#include "game.h"
#include <stdio.h>

int initGame(Game *g, SDL_Renderer *renderer)
{
    g->background = IMG_LoadTexture(renderer, "/home/khalil/op/avatar-game-khalilB7-version2/strengh and wisdom/avatar-game-khalilB7-version2/the game/images/background.png");
    if (g->background) {
        SDL_QueryTexture(g->background, NULL, NULL, &g->bg_w, &g->bg_h);
    } else {
        g->bg_w = 1600;
        g->bg_h =  600;
    }

    g->camera = (SDL_Rect){0, 0, 800, 600};

    g->nbPlatforms           = 1;
    g->platforms[0].rect     = (SDL_Rect){400, 400, 100, 20};
    g->platforms[0].type     = 0;
    g->platforms[0].texture  = NULL;

    g->startTime = SDL_GetTicks();
    g->font      = TTF_OpenFont("images/arial.ttf", 20);

    return 1;
}

void updateGame(Game *g, int fx, int fy, int fw, int fh)
{
    g->camera.x = fx + fw / 2 - g->camera.w / 2;
    g->camera.y = fy + fh / 2 - g->camera.h / 2;

    if (g->camera.x < 0)                        g->camera.x = 0;
    if (g->camera.y < 0)                        g->camera.y = 0;
    if (g->camera.x > g->bg_w - g->camera.w)   g->camera.x = g->bg_w - g->camera.w;
    if (g->camera.y > g->bg_h - g->camera.h)   g->camera.y = g->bg_h - g->camera.h;
}

void renderGame(Game *g, SDL_Renderer *renderer)
{
    if (g->background)
        SDL_RenderCopy(renderer, g->background, &g->camera, NULL);
    else {
        SDL_SetRenderDrawColor(renderer, 30, 30, 60, 255);
        SDL_RenderClear(renderer);
    }

    for (int i = 0; i < g->nbPlatforms; i++) {
        GamePlatform *p = &g->platforms[i];
        SDL_Rect dest = {
            p->rect.x - g->camera.x,
            p->rect.y - g->camera.y,
            p->rect.w,
            p->rect.h
        };
        if (p->texture)
            SDL_RenderCopy(renderer, p->texture, NULL, &dest);
        else {
            SDL_SetRenderDrawColor(renderer, 80, 180, 80, 255);
            SDL_RenderFillRect(renderer, &dest);
        }
    }
}

void renderTimer(Game *g, SDL_Renderer *renderer)
{
    if (!g->font) return;

    Uint32 secs = (SDL_GetTicks() - g->startTime) / 1000;
    char txt[32];
    sprintf(txt, "Time: %us", secs);

    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface *surf = TTF_RenderText_Solid(g->font, txt, white);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_Rect pos = {800 / 2 - surf->w / 2, 8, surf->w, surf->h};
    SDL_RenderCopy(renderer, tex, NULL, &pos);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

void freeGame(Game *g)
{
    if (g->background) SDL_DestroyTexture(g->background);
    for (int i = 0; i < g->nbPlatforms; i++)
        if (g->platforms[i].texture)
            SDL_DestroyTexture(g->platforms[i].texture);
    if (g->font) TTF_CloseFont(g->font);
}
