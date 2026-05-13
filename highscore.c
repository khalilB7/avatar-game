/*
 * highscore.c  –  High-score / name-input menu.
 *
 * players.txt format: "name score survivalSeconds\n"
 * Old 2-column entries are accepted (survivalSeconds → 0).
 *
 * Leaderboard columns: rank | name | score | time (mm:ss)
 */

#include "highscore.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── file helpers ─────────────────────────────────────────────────────── */

static void hs_save(HighScore *h, const char *name, int score, Uint32 secs)
{
    const char *path = h->pathScoreFile ? h->pathScoreFile : "players.txt";
    FILE *f = fopen(path, "a");
    if (f) {
        fprintf(f, "%s %d %u\n", name, score, (unsigned)secs);
        fclose(f);
    }
}

static int hs_load(HighScore *h)
{
    const char *path = h->pathScoreFile ? h->pathScoreFile : "players.txt";
    FILE *f = fopen(path, "r");
    int n = 0;
    if (f) {
        char line[128];
        while (n < HS_MAX_PLAYERS && fgets(line, sizeof(line), f)) {
            char   name[HS_NAME_MAX] = {0};
            int    score = 0;
            unsigned secs = 0;
            /* Try 3-column format first, fall back to 2-column */
            int got = sscanf(line, "%49s %d %u", name, &score, &secs);
            if (got >= 2) {
                strncpy(h->players[n].name, name, HS_NAME_MAX - 1);
                h->players[n].score           = score;
                h->players[n].survivalSeconds = (got >= 3) ? (Uint32)secs : 0;
                n++;
            }
        }
        fclose(f);
    }
    return n;
}

static void hs_sort(HighScore *h)
{
    for (int i = 0; i < h->playerCount - 1; i++) {
        for (int j = i + 1; j < h->playerCount; j++) {
            if (h->players[j].score > h->players[i].score) {
                HS_Player tmp  = h->players[i];
                h->players[i] = h->players[j];
                h->players[j] = tmp;
            }
        }
    }
}

/* ── misc ─────────────────────────────────────────────────────────────── */

static int mouse_in(SDL_Rect r, int x, int y)
{
    return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
}

static SDL_Texture *load_tex(const char *path, SDL_Renderer *r)
{
    if (!path) return NULL;
    SDL_Texture *t = IMG_LoadTexture(r, path);
    if (!t) fprintf(stderr, "[HighScore] IMG_LoadTexture(%s): %s\n",
                    path, IMG_GetError());
    return t;
}

/* ── public API ───────────────────────────────────────────────────────── */

bool HighScore_Init(HighScore *h, SDL_Renderer *renderer, int pendingScore)
{
    h->screen           = (pendingScore > 0) ? HS_SCREEN_INPUT : HS_SCREEN_BOARD;
    h->pendingScore     = pendingScore;
    h->pendingSurvival  = 0;
    h->inputText[0]     = '\0';

    h->bgInput = load_tex(h->pathBgInput, renderer);
    h->bgScore = load_tex(h->pathBgScore, renderer);

    h->font    = h->pathFont    ? TTF_OpenFont(h->pathFont,    28) : NULL;
    h->fontBig = h->pathFontBig ? TTF_OpenFont(h->pathFontBig, 42) : NULL;
    if (!h->font)
        fprintf(stderr, "[HighScore] font missing: %s\n", TTF_GetError());

    h->btnValider = (SDL_Rect){ 950, 620, 200, 70 };
    h->btnRetour  = (SDL_Rect){ 100, 620, 200, 70 };

    h->playerCount = hs_load(h);
    hs_sort(h);

    SDL_StartTextInput();
    return true;
}

void HighScore_SetScore(HighScore *h, int score)
{
    h->pendingScore    = score;
    h->pendingSurvival = 0;
    h->screen          = (score > 0) ? HS_SCREEN_INPUT : HS_SCREEN_BOARD;
    h->inputText[0]    = '\0';
}

void HighScore_SetScoreAndTime(HighScore *h, int score, Uint32 survivalSeconds)
{
    h->pendingScore    = score;
    h->pendingSurvival = survivalSeconds;
    h->screen          = (score > 0) ? HS_SCREEN_INPUT : HS_SCREEN_BOARD;
    h->inputText[0]    = '\0';
}

AppState HighScore_HandleEvent(HighScore *h, const SDL_Event *e,
                                SDL_Renderer *renderer)
{
    (void)renderer;

    if (e->type == SDL_KEYDOWN &&
        e->key.keysym.sym == SDLK_ESCAPE)
        return STATE_MAIN_MENU;

    /* ── name-input sub-screen ─────────────────────────────────────────── */
    if (h->screen == HS_SCREEN_INPUT) {

        if (e->type == SDL_TEXTINPUT) {
            if (strlen(h->inputText) < HS_NAME_MAX - 1)
                strcat(h->inputText, e->text.text);
        }

        if (e->type == SDL_KEYDOWN &&
            e->key.keysym.sym == SDLK_BACKSPACE &&
            strlen(h->inputText) > 0)
            h->inputText[strlen(h->inputText) - 1] = '\0';

        /* Enter or Valider button */
        bool confirm = false;
        if (e->type == SDL_KEYDOWN &&
            e->key.keysym.sym == SDLK_RETURN &&
            strlen(h->inputText) > 0)
            confirm = true;

        if (e->type == SDL_MOUSEBUTTONDOWN &&
            e->button.button == SDL_BUTTON_LEFT) {
            int mx = e->button.x, my = e->button.y;
            if (mouse_in(h->btnValider, mx, my) && strlen(h->inputText) > 0)
                confirm = true;
        }

        if (confirm) {
            hs_save(h, h->inputText, h->pendingScore, h->pendingSurvival);
            h->playerCount = hs_load(h);
            hs_sort(h);
            h->screen = HS_SCREEN_BOARD;
        }
    }

    /* ── leaderboard sub-screen ────────────────────────────────────────── */
    else {
        if (e->type == SDL_MOUSEBUTTONDOWN &&
            e->button.button == SDL_BUTTON_LEFT) {
            int mx = e->button.x, my = e->button.y;
            if (mouse_in(h->btnRetour, mx, my))
                return STATE_MAIN_MENU;
        }
    }

    return STATE_HIGHSCORE;
}

void HighScore_Render(HighScore *h, SDL_Renderer *renderer)
{
    TTF_Font *f = h->font;

    /* ── background ────────────────────────────────────────────────────── */
    if (!h->skipBackground) {
        SDL_Texture *bg = (h->screen == HS_SCREEN_INPUT) ? h->bgInput : h->bgScore;
        if (bg) {
            SDL_RenderCopy(renderer, bg, NULL, NULL);
        } else {
            SDL_SetRenderDrawColor(renderer,
                h->screen == HS_SCREEN_INPUT ? 20 : 10,
                h->screen == HS_SCREEN_INPUT ? 20 : 30,
                40, 255);
            SDL_RenderClear(renderer);
        }
    }

    if (!f) return;

    /* ── name-input sub-screen ─────────────────────────────────────────── */
    if (h->screen == HS_SCREEN_INPUT) {

        SDL_Color yellow = {255, 230, 0, 255};
        SDL_Surface *ps = TTF_RenderText_Solid(f, "Enter your name:", yellow);
        if (ps) {
            SDL_Texture *pt = SDL_CreateTextureFromSurface(renderer, ps);
            SDL_Rect pr = { 640 - ps->w / 2, 240, ps->w, ps->h };
            SDL_RenderCopy(renderer, pt, NULL, &pr);
            SDL_FreeSurface(ps); SDL_DestroyTexture(pt);
        }

        /* Show pending score + time */
        {
            unsigned secs = (unsigned)h->pendingSurvival;
            unsigned mm   = secs / 60;
            unsigned ss2  = secs % 60;
            char info[64];
            snprintf(info, sizeof(info),
                     "Score: %d   Survival: %u:%02u",
                     h->pendingScore, mm, ss2);
            SDL_Color cyan = {100, 220, 255, 255};
            SDL_Surface *is = TTF_RenderText_Solid(f, info, cyan);
            if (is) {
                SDL_Texture *it = SDL_CreateTextureFromSurface(renderer, is);
                SDL_Rect ir = { 640 - is->w / 2, 280, is->w, is->h };
                SDL_RenderCopy(renderer, it, NULL, &ir);
                SDL_FreeSurface(is); SDL_DestroyTexture(it);
            }
        }

        /* Input box */
        SDL_Rect box = { 390, 330, 500, 50 };
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 60);
        SDL_RenderFillRect(renderer, &box);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 200);
        SDL_RenderDrawRect(renderer, &box);

        if (strlen(h->inputText) > 0) {
            SDL_Color black = {0, 0, 0, 255};
            SDL_Surface *ts = TTF_RenderText_Solid(f, h->inputText, black);
            if (ts) {
                SDL_Texture *tt = SDL_CreateTextureFromSurface(renderer, ts);
                SDL_Rect tr = { box.x + 10, box.y + (box.h - ts->h) / 2,
                                ts->w, ts->h };
                SDL_RenderCopy(renderer, tt, NULL, &tr);
                SDL_FreeSurface(ts); SDL_DestroyTexture(tt);
            }
        }

        /* Valider button */
        SDL_SetRenderDrawColor(renderer, 60, 160, 60, 220);
        SDL_RenderFillRect(renderer, &h->btnValider);
        SDL_SetRenderDrawColor(renderer, 200, 255, 200, 255);
        SDL_RenderDrawRect(renderer, &h->btnValider);
        SDL_Color green = {200, 255, 200, 255};
        SDL_Surface *vs = TTF_RenderText_Solid(f, "Valider", green);
        if (vs) {
            SDL_Texture *vt = SDL_CreateTextureFromSurface(renderer, vs);
            SDL_Rect vr = {
                h->btnValider.x + (h->btnValider.w - vs->w) / 2,
                h->btnValider.y + (h->btnValider.h - vs->h) / 2,
                vs->w, vs->h };
            SDL_RenderCopy(renderer, vt, NULL, &vr);
            SDL_FreeSurface(vs); SDL_DestroyTexture(vt);
        }
    }

    /* ── leaderboard sub-screen ────────────────────────────────────────── */
    else {

        TTF_Font *fb = h->fontBig ? h->fontBig : f;
        SDL_Color gold = {255, 215, 0, 255};
        SDL_Surface *ts = TTF_RenderText_Solid(fb, "TOP 5 HIGH SCORES", gold);
        if (ts) {
            SDL_Texture *tt = SDL_CreateTextureFromSurface(renderer, ts);
            SDL_Rect tr = { 640 - ts->w / 2, 60, ts->w, ts->h };
            SDL_RenderCopy(renderer, tt, NULL, &tr);
            SDL_FreeSurface(ts); SDL_DestroyTexture(tt);
        }

        /* Column headers */
        SDL_Color hdrCol = {200, 200, 200, 200};
        SDL_Surface *hs2 = TTF_RenderText_Solid(f,
            "#    Name                   Score    Time", hdrCol);
        if (hs2) {
            SDL_Texture *ht = SDL_CreateTextureFromSurface(renderer, hs2);
            SDL_Rect hr = { 640 - hs2->w / 2, 130, hs2->w, hs2->h };
            SDL_RenderCopy(renderer, ht, NULL, &hr);
            SDL_FreeSurface(hs2); SDL_DestroyTexture(ht);
        }

        /* Divider line under header */
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 150);
        SDL_RenderDrawLine(renderer, 200, 158, 1080, 158);

        static const SDL_Color rankColors[HS_TOP_COUNT] = {
            {255, 215,   0, 255},
            {192, 192, 192, 255},
            {205, 127,  50, 255},
            {255, 255, 255, 255},
            {200, 200, 200, 255},
        };

        int top = h->playerCount < HS_TOP_COUNT ? h->playerCount : HS_TOP_COUNT;
        for (int i = 0; i < top; i++) {
            unsigned secs = (unsigned)h->players[i].survivalSeconds;
            unsigned mm   = secs / 60;
            unsigned ss2  = secs % 60;

            char buf[128];
            snprintf(buf, sizeof(buf),
                     "#%d  %-20s  %6d pts   %u:%02u",
                     i + 1,
                     h->players[i].name,
                     h->players[i].score,
                     mm, ss2);

            SDL_Color col = rankColors[i];
            SDL_Surface *rs = TTF_RenderText_Solid(f, buf, col);
            if (rs) {
                SDL_Texture *rt = SDL_CreateTextureFromSurface(renderer, rs);
                SDL_Rect rr = { 640 - rs->w / 2, 170 + i * 80, rs->w, rs->h };
                SDL_RenderCopy(renderer, rt, NULL, &rr);
                SDL_FreeSurface(rs); SDL_DestroyTexture(rt);
            }
        }

        if (top == 0) {
            SDL_Color grey = {180, 180, 180, 255};
            SDL_Surface *es = TTF_RenderText_Solid(f, "No scores yet!", grey);
            if (es) {
                SDL_Texture *et = SDL_CreateTextureFromSurface(renderer, es);
                SDL_Rect er = { 640 - es->w / 2, 300, es->w, es->h };
                SDL_RenderCopy(renderer, et, NULL, &er);
                SDL_FreeSurface(es); SDL_DestroyTexture(et);
            }
        }

        /* Retour button */
        SDL_SetRenderDrawColor(renderer, 100, 40, 40, 220);
        SDL_RenderFillRect(renderer, &h->btnRetour);
        SDL_SetRenderDrawColor(renderer, 255, 180, 180, 255);
        SDL_RenderDrawRect(renderer, &h->btnRetour);
        SDL_Color pink = {255, 180, 180, 255};
        SDL_Surface *bs = TTF_RenderText_Solid(f, "< Back", pink);
        if (bs) {
            SDL_Texture *bt = SDL_CreateTextureFromSurface(renderer, bs);
            SDL_Rect br = {
                h->btnRetour.x + (h->btnRetour.w - bs->w) / 2,
                h->btnRetour.y + (h->btnRetour.h - bs->h) / 2,
                bs->w, bs->h };
            SDL_RenderCopy(renderer, bt, NULL, &br);
            SDL_FreeSurface(bs); SDL_DestroyTexture(bt);
        }
    }
}

void HighScore_Free(HighScore *h)
{
    SDL_StopTextInput();
    if (h->bgInput) { SDL_DestroyTexture(h->bgInput); h->bgInput = NULL; }
    if (h->bgScore) { SDL_DestroyTexture(h->bgScore); h->bgScore = NULL; }
    if (h->font)    { TTF_CloseFont(h->font);          h->font    = NULL; }
    if (h->fontBig) { TTF_CloseFont(h->fontBig);       h->fontBig = NULL; }
}
