/*
 * optionsmenu.c  -  Options-menu implementation.
 */

#include "optionsmenu.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

typedef enum {
    OPT_BTN_NONE,
    OPT_BTN_VOL_UP,
    OPT_BTN_VOL_DOWN,
    OPT_BTN_WINDOW,
    OPT_BTN_FULLSCREEN,
    OPT_BTN_RETURN
} OptBtnID;

#define OPT_NBUTTONS 5

typedef struct {
    SDL_Texture *texNormal;
    SDL_Texture *texHover;
    SDL_Texture *texLabel;
    SDL_Rect     rect;
    SDL_Rect     labelRect;
    OptBtnID     id;
    bool         hover;
} OptButton;

static SDL_Texture *s_bgTexture       = NULL;
static OptButton    s_buttons[OPT_NBUTTONS];

static const char *s_labels[OPT_NBUTTONS] = {
    "Fenetre", "Diminuer", "Augmenter", "Plein ecran", "Retour"
};
static const OptBtnID s_ids[OPT_NBUTTONS] = {
    OPT_BTN_WINDOW, OPT_BTN_VOL_DOWN, OPT_BTN_VOL_UP,
    OPT_BTN_FULLSCREEN, OPT_BTN_RETURN
};
static const SDL_Rect s_positions[OPT_NBUTTONS] = {
    {100, 350, 200, 60},
    {100, 250, 200, 60},
    {500, 250, 200, 60},
    {500, 350, 200, 60},
    { 20, 500, 120, 50}
};

static SDL_Texture *opt_loadTex(const char *path, SDL_Renderer *r)
{
    if (!path) return NULL;
    SDL_Texture *t = IMG_LoadTexture(r, path);
    if (!t) fprintf(stderr, "[Options] IMG_LoadTexture(%s): %s\n", path, IMG_GetError());
    return t;
}

static void rebuildVolText(OptionsMenu *o, SDL_Renderer *renderer)
{
    if (!o->font) return;
    if (o->volText) { SDL_DestroyTexture(o->volText); o->volText = NULL; }
    char buf[32];
    snprintf(buf, sizeof(buf), "Volume : %d%%", o->volumePercent);
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface *surf = TTF_RenderText_Blended(o->font, buf, white);
    if (!surf) return;
    o->volText = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_QueryTexture(o->volText, NULL, NULL, &o->volRect.w, &o->volRect.h);
    o->volRect.x = (800 - o->volRect.w) / 2;
    o->volRect.y = 50;
    SDL_FreeSurface(surf);
}

bool OptionsMenu_Init(OptionsMenu *o, SDL_Renderer *renderer)
{
    s_bgTexture = opt_loadTex(o->pathBackground, renderer);
    o->font = o->pathFont ? TTF_OpenFont(o->pathFont, 24) : NULL;
    if (!o->font) fprintf(stderr, "[Options] No font: %s\n", TTF_GetError());

    for (int i = 0; i < OPT_NBUTTONS; i++) {
        OptButton *b = &s_buttons[i];
        b->id        = s_ids[i];
        b->hover     = false;
        b->rect      = s_positions[i];
        b->texNormal = opt_loadTex(o->pathBtnNormal, renderer);
        b->texHover  = opt_loadTex(o->pathBtnHover,  renderer);
        b->texLabel  = NULL;
        if (o->font) {
            SDL_Color white = {255, 255, 255, 255};
            SDL_Surface *surf = TTF_RenderText_Blended(o->font, s_labels[i], white);
            if (surf) {
                b->texLabel = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_QueryTexture(b->texLabel, NULL, NULL, &b->labelRect.w, &b->labelRect.h);
                b->labelRect.x = b->rect.x + (b->rect.w - b->labelRect.w) / 2;
                b->labelRect.y = b->rect.y + (b->rect.h - b->labelRect.h) / 2;
                SDL_FreeSurface(surf);
            }
        }
    }
    o->volumePercent = Mix_VolumeMusic(-1) * 100 / MIX_MAX_VOLUME;
    o->volText = NULL;
    rebuildVolText(o, renderer);
    return true;
}

AppState OptionsMenu_HandleEvent(OptionsMenu *o, SDL_Window *window,
                                  const SDL_Event *e, SDL_Renderer *renderer)
{
    if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_ESCAPE)
        return STATE_MAIN_MENU;

    if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT) {
        SDL_Point p = {e->button.x, e->button.y};
        for (int i = 0; i < OPT_NBUTTONS; i++) {
            if (!SDL_PointInRect(&p, &s_buttons[i].rect)) continue;
            switch (s_buttons[i].id) {
                case OPT_BTN_VOL_UP: {
                    int vol = Mix_VolumeMusic(-1) + 8;
                    if (vol > MIX_MAX_VOLUME) vol = MIX_MAX_VOLUME;
                    Mix_VolumeMusic(vol);
                    o->volumePercent = vol * 100 / MIX_MAX_VOLUME;
                    rebuildVolText(o, renderer);
                    break;
                }
                case OPT_BTN_VOL_DOWN: {
                    int vol = Mix_VolumeMusic(-1) - 8;
                    if (vol < 0) vol = 0;
                    Mix_VolumeMusic(vol);
                    o->volumePercent = vol * 100 / MIX_MAX_VOLUME;
                    rebuildVolText(o, renderer);
                    break;
                }
                case OPT_BTN_WINDOW:
                    SDL_SetWindowFullscreen(window, 0);
                    break;
                case OPT_BTN_FULLSCREEN:
                    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                    break;
                case OPT_BTN_RETURN:
                    return STATE_MAIN_MENU;
                default:
                    break;
            }
        }
    }
    return STATE_OPTIONS;
}

void OptionsMenu_Update(OptionsMenu *o)
{
    (void)o;
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    SDL_Point p = {mx, my};
    for (int i = 0; i < OPT_NBUTTONS; i++)
        s_buttons[i].hover = SDL_PointInRect(&p, &s_buttons[i].rect);
}

void OptionsMenu_Render(OptionsMenu *o, SDL_Renderer *renderer)
{
    if (!o->skipBackground) {
        if (s_bgTexture) {
            SDL_RenderCopy(renderer, s_bgTexture, NULL, NULL);
        } else {
            SDL_SetRenderDrawColor(renderer, 10, 20, 40, 255);
            SDL_RenderClear(renderer);
        }
    }

    for (int i = 0; i < OPT_NBUTTONS; i++) {
        OptButton *b   = &s_buttons[i];
        SDL_Texture *t = b->hover ? b->texHover : b->texNormal;
        if (t) {
            SDL_RenderCopy(renderer, t, NULL, &b->rect);
        } else {
            if (b->hover)
                SDL_SetRenderDrawColor(renderer, 180, 50, 50, 230);
            else
                SDL_SetRenderDrawColor(renderer, 60, 60, 100, 200);
            SDL_RenderFillRect(renderer, &b->rect);
            SDL_SetRenderDrawColor(renderer, 200, 200, 255, 255);
            SDL_RenderDrawRect(renderer, &b->rect);
        }
        if (b->texLabel)
            SDL_RenderCopy(renderer, b->texLabel, NULL, &b->labelRect);
    }
    if (o->volText)
        SDL_RenderCopy(renderer, o->volText, NULL, &o->volRect);
}

void OptionsMenu_Free(OptionsMenu *o)
{
    if (s_bgTexture) { SDL_DestroyTexture(s_bgTexture); s_bgTexture = NULL; }
    for (int i = 0; i < OPT_NBUTTONS; i++) {
        if (s_buttons[i].texNormal) SDL_DestroyTexture(s_buttons[i].texNormal);
        if (s_buttons[i].texHover)  SDL_DestroyTexture(s_buttons[i].texHover);
        if (s_buttons[i].texLabel)  SDL_DestroyTexture(s_buttons[i].texLabel);
        memset(&s_buttons[i], 0, sizeof(OptButton));
    }
    if (o->volText) { SDL_DestroyTexture(o->volText); o->volText = NULL; }
    if (o->font)    { TTF_CloseFont(o->font);         o->font    = NULL; }
}
