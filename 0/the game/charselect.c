/*
 * charselect.c  –  Character-selection screen.
 *
 * Layout (800 × 600 window):
 *
 *   ┌──────────────────────────────────────────────────────┐
 *   │          CHOOSE YOUR CHARACTER                       │
 *   │                                                      │
 *   │  ┌──────────────┐        ┌──────────────┐           │
 *   │  │              │        │              │           │
 *   │  │  [sprite 1]  │        │  [sprite 2]  │           │
 *   │  │              │        │              │           │
 *   │  │  SOLDIER     │        │  WARRIOR     │           │
 *   │  │  ← → ↑ ↓    │        │  Z Q S D     │           │
 *   │  │  Shoot: K    │        │  Shoot: L    │           │
 *   │  │  Dash:  M    │        │  Dash:  C    │           │
 *   │  │  Jump:  N    │        │  Jump: SPC   │           │
 *   │  └──────────────┘        └──────────────┘           │
 *   │                                                      │
 *   │              [ CONFIRM ]                             │
 *   └──────────────────────────────────────────────────────┘
 */

#include "charselect.h"
#include <stdio.h>
#include <string.h>

/* ── geometry ─────────────────────────────────────────────────────────── */
#define CARD_W       280
#define CARD_H       320
#define CARD1_X       60
#define CARD2_X      460
#define CARD_Y       130
#define PREVIEW_W    128
#define PREVIEW_H    128
/* sprite sheet: we show frame 0 of ROW_IDLE (y=0) */
#define PREVIEW_SRC_X  0
#define PREVIEW_SRC_Y  0

/* ── colours ──────────────────────────────────────────────────────────── */
static const SDL_Color COL_TITLE   = {255, 215,   0, 255};
static const SDL_Color COL_WHITE   = {255, 255, 255, 255};
static const SDL_Color COL_GREY    = {180, 180, 180, 200};
static const SDL_Color COL_CONFIRM = {200, 255, 200, 255};
static const SDL_Color COL_HINT    = {160, 210, 255, 200};

/* ── helpers ──────────────────────────────────────────────────────────── */

static SDL_Texture *cs_loadTex(const char *path, SDL_Renderer *r)
{
    if (!path) return NULL;
    SDL_Texture *t = IMG_LoadTexture(r, path);
    if (!t)
        fprintf(stderr, "[CharSelect] IMG_LoadTexture(%s): %s\n",
                path, IMG_GetError());
    return t;
}

static void cs_renderText(SDL_Renderer *r, TTF_Font *f,
                          const char *txt, SDL_Color col,
                          int cx, int y)
{
    if (!f) return;
    SDL_Surface *s = TTF_RenderText_Blended(f, txt, col);
    if (!s) return;
    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
    SDL_Rect dst = { cx - s->w / 2, y, s->w, s->h };
    SDL_RenderCopy(r, t, NULL, &dst);
    SDL_FreeSurface(s);
    SDL_DestroyTexture(t);
}

static bool pt_in(SDL_Rect rect, int x, int y)
{
    return x >= rect.x && x < rect.x + rect.w &&
           y >= rect.y && y < rect.y + rect.h;
}

/* ── public API ───────────────────────────────────────────────────────── */

bool CharSelect_Init(CharSelect *cs, SDL_Renderer *renderer)
{
    cs->chosenId  = 1;   /* default selection */
    cs->selected  = 1;
    cs->hovered   = 0;

    /* Card rects */
    cs->cardRect[0] = (SDL_Rect){ CARD1_X, CARD_Y, CARD_W, CARD_H };
    cs->cardRect[1] = (SDL_Rect){ CARD2_X, CARD_Y, CARD_W, CARD_H };

    /* Confirm button */
    cs->confirmRect = (SDL_Rect){ 800/2 - 100, CARD_Y + CARD_H + 24, 200, 52 };

    /* Sprite previews (first idle frame of the sprite sheet) */
    cs->preview[0] = cs_loadTex(cs->pathPreview1, renderer);
    cs->preview[1] = cs_loadTex(cs->pathPreview2, renderer);

    /* Fonts */
    cs->font    = cs->pathFont    ? TTF_OpenFont(cs->pathFont,    22) : NULL;
    cs->fontBig = cs->pathFontBig ? TTF_OpenFont(cs->pathFontBig, 38) : NULL;

    if (!cs->font)
        fprintf(stderr, "[CharSelect] font missing: %s\n", TTF_GetError());

    return true;
}

AppState CharSelect_HandleEvent(CharSelect *cs, const SDL_Event *e)
{
    /* Keyboard: left/right to switch, Enter to confirm, Esc to go back */
    if (e->type == SDL_KEYDOWN) {
        SDL_Keycode k = e->key.keysym.sym;
        if (k == SDLK_ESCAPE)  return STATE_MAIN_MENU;
        if (k == SDLK_LEFT  || k == SDLK_q) { cs->selected = 1; cs->chosenId = 1; }
        if (k == SDLK_RIGHT || k == SDLK_d) { cs->selected = 2; cs->chosenId = 2; }
        if (k == SDLK_RETURN || k == SDLK_KP_ENTER) {
            return STATE_PLAYING;
        }
    }

    /* Mouse */
    if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT) {
        int mx = e->button.x, my = e->button.y;

        if (pt_in(cs->cardRect[0], mx, my)) {
            cs->selected = 1;
            cs->chosenId = 1;
        } else if (pt_in(cs->cardRect[1], mx, my)) {
            cs->selected = 2;
            cs->chosenId = 2;
        }

        if (pt_in(cs->confirmRect, mx, my) && cs->selected > 0)
            return STATE_PLAYING;
    }

    return STATE_CHAR_SELECT;
}

void CharSelect_Update(CharSelect *cs)
{
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    cs->hovered = 0;
    if (pt_in(cs->cardRect[0], mx, my)) cs->hovered = 1;
    if (pt_in(cs->cardRect[1], mx, my)) cs->hovered = 2;
}

/* Draw one character card */
static void drawCard(CharSelect *cs, SDL_Renderer *r,
                     int idx,            /* 0 or 1 */
                     const char *name,
                     const char *line1,  /* movement keys */
                     const char *line2,
                     const char *line3,
                     const char *line4)
{
    SDL_Rect card  = cs->cardRect[idx];
    bool selected  = (cs->selected  == idx + 1);
    bool hovered   = (cs->hovered   == idx + 1);

    /* Card background */
    if (selected)
        SDL_SetRenderDrawColor(r, 30, 80, 160, 230);
    else if (hovered)
        SDL_SetRenderDrawColor(r, 50, 50, 100, 200);
    else
        SDL_SetRenderDrawColor(r, 25, 25,  55, 190);
    SDL_RenderFillRect(r, &card);

    /* Card border – gold if selected, white if hovered, grey otherwise */
    if (selected)
        SDL_SetRenderDrawColor(r, 255, 215, 0, 255);
    else if (hovered)
        SDL_SetRenderDrawColor(r, 200, 200, 255, 255);
    else
        SDL_SetRenderDrawColor(r, 100, 100, 140, 200);
    SDL_RenderDrawRect(r, &card);

    /* Inner border highlight when selected */
    if (selected) {
        SDL_Rect inner = { card.x+2, card.y+2, card.w-4, card.h-4 };
        SDL_SetRenderDrawColor(r, 255, 215, 0, 80);
        SDL_RenderDrawRect(r, &inner);
    }

    /* Sprite preview */
    int previewX = card.x + (card.w - PREVIEW_W) / 2;
    int previewY = card.y + 12;
    SDL_Rect destPreview = { previewX, previewY, PREVIEW_W, PREVIEW_H };

    if (cs->preview[idx]) {
        /* Crop first idle frame (128×128 at top-left of sheet) */
        SDL_Rect srcRect = { PREVIEW_SRC_X, PREVIEW_SRC_Y, 128, 128 };
        SDL_RenderCopy(r, cs->preview[idx], &srcRect, &destPreview);
    } else {
        /* Fallback coloured rectangle */
        SDL_SetRenderDrawColor(r,
            idx == 0 ?  80 : 220,
            idx == 0 ? 140 :  80,
            idx == 0 ? 240 :  80, 200);
        SDL_RenderFillRect(r, &destPreview);
    }

    /* "SELECTED" badge */
    if (selected && cs->fontBig) {
        SDL_Color gold = {255, 215, 0, 255};
        cs_renderText(r, cs->fontBig, "SELECTED", gold,
                      card.x + card.w / 2, previewY + PREVIEW_H + 4);
    }

    /* Character name */
    int textY = previewY + PREVIEW_H + (selected && cs->fontBig ? 44 : 8);
    int cx    = card.x + card.w / 2;
    SDL_Color nameCol = selected
        ? (SDL_Color){255, 230, 80, 255}
        : COL_WHITE;
    cs_renderText(r, cs->fontBig ? cs->fontBig : cs->font,
                  name, nameCol, cx, textY);
    textY += (cs->fontBig ? 42 : 28);

    /* Control lines */
    cs_renderText(r, cs->font, line1, COL_GREY, cx, textY);  textY += 22;
    cs_renderText(r, cs->font, line2, COL_GREY, cx, textY);  textY += 22;
    cs_renderText(r, cs->font, line3, COL_GREY, cx, textY);  textY += 22;
    cs_renderText(r, cs->font, line4, COL_GREY, cx, textY);
}

void CharSelect_Render(CharSelect *cs, SDL_Renderer *r)
{
    /* Background */
    if (!cs->skipBackground) {
        SDL_SetRenderDrawColor(r, 10, 12, 35, 255);
        SDL_RenderClear(r);

        /* Subtle grid overlay */
        SDL_SetRenderDrawColor(r, 255, 255, 255, 12);
        for (int x = 0; x < 800; x += 40)
            SDL_RenderDrawLine(r, x, 0, x, 600);
        for (int y = 0; y < 600; y += 40)
            SDL_RenderDrawLine(r, 0, y, 800, y);
    }

    /* Title */
    cs_renderText(r, cs->fontBig ? cs->fontBig : cs->font,
                  "CHOOSE YOUR CHARACTER",
                  COL_TITLE, 400, 30);

    /* Sub-hint */
    cs_renderText(r, cs->font,
                  "Click a card or use  ←/→  to select,  then press  Enter  or  Confirm",
                  COL_HINT, 400, 82);

    /* Player 1 card */
    drawCard(cs, r, 0,
             "SOLDIER",
             "Move:  Arrow keys",
             "Shoot: K",
             "Dash:  M",
             "Jump:  N");

    /* Player 2 card */
    drawCard(cs, r, 1,
             "WARRIOR",
             "Move:  Z Q S D",
             "Shoot: L",
             "Dash:  C",
             "Jump:  Space");

    /* VS separator */
    cs_renderText(r, cs->fontBig ? cs->fontBig : cs->font,
                  "VS",
                  (SDL_Color){255, 80, 80, 200},
                  400, CARD_Y + CARD_H / 2 - 20);

    /* Confirm button */
    bool confirmHov = ({ int mx, my; SDL_GetMouseState(&mx,&my);
                         pt_in(cs->confirmRect, mx, my); });
    bool confirmOk  = (cs->selected > 0);

    SDL_SetRenderDrawColor(r,
        confirmOk ? (confirmHov ? 80 : 40) : 25,
        confirmOk ? (confirmHov ? 180 : 130) : 40,
        confirmOk ? (confirmHov ? 80 : 40) : 25,
        confirmOk ? 230 : 120);
    SDL_RenderFillRect(r, &cs->confirmRect);

    SDL_SetRenderDrawColor(r,
        confirmOk ? 180 : 80,
        confirmOk ? 255 : 100,
        confirmOk ? 180 : 80, 255);
    SDL_RenderDrawRect(r, &cs->confirmRect);

    cs_renderText(r, cs->font,
                  confirmOk ? "CONFIRM  ( Enter )" : "Select a character first",
                  confirmOk ? COL_CONFIRM : COL_GREY,
                  cs->confirmRect.x + cs->confirmRect.w / 2,
                  cs->confirmRect.y + (cs->confirmRect.h -
                      (cs->font ? TTF_FontHeight(cs->font) : 20)) / 2);

    /* ESC hint */
    cs_renderText(r, cs->font,
                  "ESC = Back to Main Menu",
                  (SDL_Color){140, 140, 140, 180},
                  400, 570);
}

void CharSelect_Free(CharSelect *cs)
{
    for (int i = 0; i < 2; i++) {
        if (cs->preview[i]) {
            SDL_DestroyTexture(cs->preview[i]);
            cs->preview[i] = NULL;
        }
    }
    if (cs->font)    { TTF_CloseFont(cs->font);    cs->font    = NULL; }
    if (cs->fontBig) { TTF_CloseFont(cs->fontBig); cs->fontBig = NULL; }
}
