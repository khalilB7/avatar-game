#include "partage.h"
#include <stdio.h>

/* ══════════════════════════════════════════════════════════════════════
 * Init / free
 * ══════════════════════════════════════════════════════════════════════ */

int initSplitGame(SplitGame *g, SDL_Renderer *renderer)
{
    g->background = IMG_LoadTexture(renderer, "/home/khalil/op/avatar-game-khalilB7-version2/strengh and wisdom/avatar-game-khalilB7-version2/the game/images/background.png");
    if (g->background) {
        SDL_QueryTexture(g->background, NULL, NULL, &g->bg_w, &g->bg_h);
    } else {
        g->bg_w = 1600;
        g->bg_h =  600;
    }

    for (int i = 0; i < 2; i++)
        g->camera[i] = (SDL_Rect){0, 0, SPLIT_VIEW_W, SPLIT_VIEW_H};

    g->startTime = SDL_GetTicks();
    g->font      = TTF_OpenFont("/home/khalil/op/avatar-game-khalilB7-version2/strengh and wisdom/avatar-game-khalilB7-version2/the game/images/arial.ttf", 18);

    return 1;
}

void freeSplitGame(SplitGame *g)
{
    if (g->background) SDL_DestroyTexture(g->background);
    if (g->font)       TTF_CloseFont(g->font);
}

/* ══════════════════════════════════════════════════════════════════════
 * Camera update
 * ══════════════════════════════════════════════════════════════════════ */

static void clampCamera(SDL_Rect *cam, int worldW, int worldH)
{
    if (cam->x < 0)                  cam->x = 0;
    if (cam->y < 0)                  cam->y = 0;
    if (cam->x > worldW - cam->w)   cam->x = worldW - cam->w;
    if (cam->y > worldH - cam->h)   cam->y = worldH - cam->h;
}

void updateSplitGame(SplitGame *g, Hero *players, int numPlayers)
{
    for (int i = 0; i < numPlayers && i < 2; i++) {
        SDL_Rect *cam  = &g->camera[i];
        SDL_Rect *hero = &players[i].posHero;
        cam->x = hero->x + hero->w / 2 - cam->w / 2;
        cam->y = hero->y + hero->h / 2 - cam->h / 2;
        clampCamera(cam, g->bg_w, g->bg_h);
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * Render helpers
 * ══════════════════════════════════════════════════════════════════════ */

static void drawDivider(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 200);
    SDL_RenderDrawLine(renderer, SPLIT_VIEW_W,     0, SPLIT_VIEW_W,     SPLIT_VIEW_H);
    SDL_RenderDrawLine(renderer, SPLIT_VIEW_W - 1, 0, SPLIT_VIEW_W - 1, SPLIT_VIEW_H);
}

static void drawLifeBar(SDL_Renderer *renderer, TTF_Font *font,
                        Hero *hero, int vpX)
{
    /* Dark background panel — tall enough for name + lives + score */
    SDL_Rect bar = {vpX + 4, 4, 180, 52};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
    SDL_RenderFillRect(renderer, &bar);

    if (!font) return;

    /* Player label */
    char label[4];
    sprintf(label, "P%d", hero->id);
    SDL_Color nameColor = (hero->id == 1)
        ? (SDL_Color){100, 220, 255, 255}
        : (SDL_Color){255, 160,  80, 255};

    SDL_Surface *ns = TTF_RenderText_Solid(font, label, nameColor);
    if (ns) {
        SDL_Texture *nt = SDL_CreateTextureFromSurface(renderer, ns);
        SDL_Rect np = {vpX + 8, 6, ns->w, ns->h};
        SDL_RenderCopy(renderer, nt, NULL, &np);
        SDL_FreeSurface(ns);
        SDL_DestroyTexture(nt);
    }

    /* Life icons */
    int iconSize = 20;
    int startX   = vpX + 44;
    for (int i = 0; i < hero->lives; i++) {
        SDL_Rect pos = {startX + i * (iconSize + 3), 6, iconSize, iconSize};
        if (hero->healthIcon) {
            SDL_RenderCopy(renderer, hero->healthIcon, NULL, &pos);
        } else {
            SDL_SetRenderDrawColor(renderer, 220, 30, 30, 255);
            SDL_RenderFillRect(renderer, &pos);
        }
    }

    /* Score line */
    char scoreTxt[32];
    if (hero->scoreActive)
        sprintf(scoreTxt, "Score: %d", hero->score);
    else
        sprintf(scoreTxt, "Score: --");

    SDL_Color scoreColor = (hero->id == 1)
        ? (SDL_Color){ 80, 220, 120, 255}
        : (SDL_Color){220, 200,  60, 255};

    SDL_Surface *ss = TTF_RenderText_Solid(font, scoreTxt, scoreColor);
    if (ss) {
        SDL_Texture *st = SDL_CreateTextureFromSurface(renderer, ss);
        SDL_Rect sp = {vpX + 8, 31, ss->w, ss->h};
        SDL_RenderCopy(renderer, st, NULL, &sp);
        SDL_FreeSurface(ss);
        SDL_DestroyTexture(st);
    }
}

static void drawGameOverOverlay(SDL_Renderer *renderer, TTF_Font *font)
{
    if (!font) return;
    SDL_Color red = {255, 50, 50, 255};
    SDL_Surface *s = TTF_RenderText_Solid(font, "GAME OVER", red);
    if (!s) return;
    SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);
    /* Centred in the current viewport */
    SDL_Rect pos = {(SPLIT_VIEW_W - s->w) / 2, SPLIT_VIEW_H / 2 - s->h,
                    s->w, s->h};
    SDL_RenderCopy(renderer, t, NULL, &pos);
    SDL_FreeSurface(s);
    SDL_DestroyTexture(t);
}

/* ══════════════════════════════════════════════════════════════════════
 * Main render entry point
 * ══════════════════════════════════════════════════════════════════════ */

void renderSplitGame(SplitGame *g, SDL_Renderer *renderer,
                     Hero *players, int numPlayers, TTF_Font *font)
{
    TTF_Font *f = font ? font : g->font;

    /* ── Single-player: full screen ── */
    if (numPlayers == 1) {
        SDL_RenderSetViewport(renderer, NULL);

        if (g->background)
            SDL_RenderCopy(renderer, g->background, &g->camera[0], NULL);
        else {
            SDL_SetRenderDrawColor(renderer, 20, 20, 50, 255);
            SDL_RenderClear(renderer);
        }

        showPlayer(renderer, &players[0], g->camera[0].x, g->camera[0].y);
        drawBullets(&players[0], renderer, g->camera[0].x, g->camera[0].y);

        drawLifeBar(renderer, f, &players[0], 0);
        if (players[0].dead) drawGameOverOverlay(renderer, f);

        SDL_RenderSetViewport(renderer, NULL);
        return;
    }

    /* ── Two-player split-screen ── */
    for (int p = 0; p < 2; p++) {
        SDL_Rect vp = {p * SPLIT_VIEW_W, 0, SPLIT_VIEW_W, SPLIT_VIEW_H};
        SDL_RenderSetViewport(renderer, &vp);

        if (g->background)
            SDL_RenderCopy(renderer, g->background, &g->camera[p], NULL);
        else {
            SDL_SetRenderDrawColor(renderer, 20, 20, 50, 255);
            SDL_RenderClear(renderer);
        }

        /* Both heroes are visible in both viewports */
        for (int i = 0; i < 2; i++) {
            showPlayer(renderer, &players[i], g->camera[p].x, g->camera[p].y);
            drawBullets(&players[i], renderer, g->camera[p].x, g->camera[p].y);
        }

        /* HUD uses viewport-local coords (vpX = 0 inside the viewport) */
        drawLifeBar(renderer, f, &players[p], 0);
        if (players[p].dead) drawGameOverOverlay(renderer, f);
    }

    /* Dividing line in full-screen space */
    SDL_RenderSetViewport(renderer, NULL);
    drawDivider(renderer);
}
