/*
 * mainmenu.c  -  Main-menu implementation.
 */

#include "mainmenu.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <string.h>

static SDL_Texture *loadTex(const char *path, SDL_Renderer *r)
{
    if (!path) return NULL;
    SDL_Surface *s = IMG_Load(path);
    if (!s) { fprintf(stderr, "[MainMenu] IMG_Load(%s): %s\n", path, IMG_GetError()); return NULL; }
    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
    SDL_FreeSurface(s);
    return t;
}

bool MainMenu_Init(MainMenu *m, SDL_Renderer *renderer)
{
    m->bgTexture     = loadTex(m->pathBg,    renderer);
    m->btnTexture    = loadTex(m->pathBtn,   renderer);
    m->cornerTexture = loadTex(m->pathCorner,renderer);

    m->bgMusic    = m->pathMusic    ? Mix_LoadMUS(m->pathMusic)    : NULL;
    m->hoverSound = m->pathHoverSfx ? Mix_LoadWAV(m->pathHoverSfx) : NULL;

    m->font = m->pathFont ? TTF_OpenFont(m->pathFont, 24) : NULL;
    if (!m->font) fprintf(stderr, "[MainMenu] No font: %s\n", TTF_GetError());

    if (m->bgMusic) Mix_PlayMusic(m->bgMusic, -1);

    static const char *labels[MM_BTN_COUNT] = {
        "Play", "Options", "Save / Load", "High Scores", "Exit"
    };
    int leftX  = 100;
    int startY = 180;
    int gapY   = 70;
    for (int i = 0; i < MM_BTN_COUNT; i++) {
        m->buttons[i].rect    = (SDL_Rect){ leftX, startY + i * gapY, 220, 55 };
        m->buttons[i].text    = labels[i];
        m->buttons[i].hovered = false;
    }
    return true;
}

AppState MainMenu_HandleEvent(MainMenu *m, const SDL_Event *e)
{
    if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT) {
        int x = e->button.x, y = e->button.y;
        for (int i = 0; i < MM_BTN_COUNT; i++) {
            SDL_Rect r = m->buttons[i].rect;
            if (x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h) {
                switch (i) {
                    case MM_BTN_PLAY:      return STATE_PLAYING;
                    case MM_BTN_OPTIONS:   return STATE_OPTIONS;
                    case MM_BTN_SAVE:      return STATE_SAVE_LOAD;
                    case MM_BTN_HIGHSCORE: return STATE_HIGHSCORE;
                    case MM_BTN_EXIT:      return STATE_EXIT;
                }
            }
        }
    }
    if (e->type == SDL_KEYDOWN) {
        switch (e->key.keysym.sym) {
            case SDLK_RETURN: return STATE_PLAYING;
            case SDLK_ESCAPE: return STATE_EXIT;
        }
    }
    return STATE_MAIN_MENU;
}

void MainMenu_Update(MainMenu *m)
{
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    for (int i = 0; i < MM_BTN_COUNT; i++) {
        SDL_Rect r   = m->buttons[i].rect;
        bool entered = (mx >= r.x && mx < r.x + r.w && my >= r.y && my < r.y + r.h);
        if (entered && !m->buttons[i].hovered && m->hoverSound)
            Mix_PlayChannel(-1, m->hoverSound, 0);
        m->buttons[i].hovered = entered;
    }
}

void MainMenu_Render(MainMenu *m, SDL_Renderer *renderer)
{
    if (!m->skipBackground) {
        if (m->bgTexture) {
            SDL_RenderCopy(renderer, m->bgTexture, NULL, NULL);
        } else {
            SDL_SetRenderDrawColor(renderer, 15, 15, 40, 255);
            SDL_RenderClear(renderer);
        }
    }

    if (m->cornerTexture) {
        SDL_Rect dst = { 800 - 150 - 20, 20, 150, 150 };
        SDL_RenderCopy(renderer, m->cornerTexture, NULL, &dst);
    }

    for (int i = 0; i < MM_BTN_COUNT; i++) {
        SDL_Rect dst = m->buttons[i].rect;
        if (m->buttons[i].hovered) { dst.x -= 4; dst.y -= 4; dst.w += 8; dst.h += 8; }

        if (m->btnTexture) {
            SDL_RenderCopy(renderer, m->btnTexture, NULL, &dst);
        } else {
            if (m->buttons[i].hovered)
                SDL_SetRenderDrawColor(renderer, 180, 60, 60, 230);
            else
                SDL_SetRenderDrawColor(renderer, 60, 60, 120, 200);
            SDL_RenderFillRect(renderer, &dst);
            SDL_SetRenderDrawColor(renderer, 200, 200, 255, 255);
            SDL_RenderDrawRect(renderer, &dst);
        }

        if (m->font) {
            SDL_Color white = {255, 255, 255, 255};
            SDL_Surface *surf = TTF_RenderText_Blended(m->font, m->buttons[i].text, white);
            if (surf) {
                SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_Rect tr = { dst.x + 16, dst.y + (dst.h - surf->h) / 2, surf->w, surf->h };
                SDL_RenderCopy(renderer, tex, NULL, &tr);
                SDL_FreeSurface(surf);
                SDL_DestroyTexture(tex);
            }
        }
    }
}

void MainMenu_Free(MainMenu *m)
{
    if (m->bgTexture)     SDL_DestroyTexture(m->bgTexture);
    if (m->btnTexture)    SDL_DestroyTexture(m->btnTexture);
    if (m->cornerTexture) SDL_DestroyTexture(m->cornerTexture);
    if (m->bgMusic)       Mix_FreeMusic(m->bgMusic);
    if (m->hoverSound)    Mix_FreeChunk(m->hoverSound);
    if (m->font)          TTF_CloseFont(m->font);
    memset(m, 0, sizeof(*m));
}
