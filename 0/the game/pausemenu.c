/*
 * pausemenu.c  –  In-game pause overlay with 3-2-1 countdown.
 */

#include "pausemenu.h"
#include <stdio.h>
#include <string.h>

static const char *s_labels[PAUSE_ITEM_COUNT] = {
    "Resume",
    "Save Game",
    "Exit to Main Menu"
};

void PauseMenu_Open(PauseMenu *p)
{
    p->visible       = true;
    p->selectedItem  = PAUSE_ITEM_RESUME;
    p->countdown     = COUNTDOWN_NONE;
    p->countdownValue = 3;
}

/* Returns the chosen PauseItem (0/1/2) or -1 if no action yet. */
int PauseMenu_HandleEvent(PauseMenu *p, const SDL_Event *e)
{
    /* Ignore input during countdown */
    if (p->countdown == COUNTDOWN_ACTIVE) return -1;

    if (e->type == SDL_KEYDOWN) {
        SDL_Keycode k = e->key.keysym.sym;

        if (k == SDLK_UP || k == SDLK_w) {
            p->selectedItem--;
            if (p->selectedItem < 0)
                p->selectedItem = PAUSE_ITEM_COUNT - 1;
            return -1;
        }
        if (k == SDLK_DOWN || k == SDLK_s) {
            p->selectedItem = (p->selectedItem + 1) % PAUSE_ITEM_COUNT;
            return -1;
        }
        if (k == SDLK_RETURN || k == SDLK_KP_ENTER) {
            return p->selectedItem;
        }
        /* H again or ESC also resume */
        if (k == SDLK_h || k == SDLK_ESCAPE) {
            return PAUSE_ITEM_RESUME;
        }
    }

    /* Mouse click */
    if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT) {
        int mx = e->button.x;
        int my = e->button.y;

        /* Button rects mirror PauseMenu_Render layout */
        int bx = 800 / 2 - 150;
        int bw = 300;
        int bh = 50;
        int startY = 600 / 2 - 30;
        int gapY   = 70;

        for (int i = 0; i < PAUSE_ITEM_COUNT; i++) {
            int by = startY + i * gapY;
            if (mx >= bx && mx < bx + bw && my >= by && my < by + bh)
                return i;
        }
    }

    return -1;
}

/*
 * Call once per frame.
 * Returns true when the countdown has finished and gameplay should resume.
 */
bool PauseMenu_Update(PauseMenu *p)
{
    if (p->countdown != COUNTDOWN_ACTIVE) return false;

    Uint32 now     = SDL_GetTicks();
    Uint32 elapsed = now - p->countdownStart;

    /* Advance digit every 1000 ms */
    int digitIndex = (int)(elapsed / 1000);   /* 0→"3", 1→"2", 2→"1" */
    p->countdownValue = 3 - digitIndex;

    if (p->countdownValue < 1) {
        p->countdown  = COUNTDOWN_DONE;
        p->visible    = false;
        return true;   /* caller resumes gameplay */
    }
    return false;
}

void PauseMenu_Render(PauseMenu *p, SDL_Renderer *renderer)
{
    if (!p->visible && p->countdown != COUNTDOWN_ACTIVE) return;

    /* ── semi-transparent dark overlay ───────────────────────────────────── */
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
    SDL_Rect full = {0, 0, 800, 600};
    SDL_RenderFillRect(renderer, &full);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    TTF_Font *f    = p->font;
    TTF_Font *fBig = p->fontBig ? p->fontBig : f;

    /* ── COUNTDOWN ────────────────────────────────────────────────────────── */
    if (p->countdown == COUNTDOWN_ACTIVE) {
        if (!fBig) return;
        char buf[4];
        snprintf(buf, sizeof(buf), "%d", p->countdownValue);
        SDL_Color red = {255, 50, 50, 255};
        SDL_Surface *s = TTF_RenderText_Blended(fBig, buf, red);
        if (s) {
            SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);
            SDL_Rect r = {800 / 2 - s->w / 2, 600 / 2 - s->h / 2, s->w, s->h};
            SDL_RenderCopy(renderer, t, NULL, &r);
            SDL_FreeSurface(s);
            SDL_DestroyTexture(t);
        }
        return;
    }

    /* ── PAUSE MENU ───────────────────────────────────────────────────────── */
    if (!f) {
        /* No font: just draw coloured rectangles */
        int bx = 800 / 2 - 150;
        int startY = 600 / 2 - 30;
        for (int i = 0; i < PAUSE_ITEM_COUNT; i++) {
            SDL_Rect r = {bx, startY + i * 70, 300, 50};
            SDL_SetRenderDrawColor(renderer,
                i == p->selectedItem ? 180 : 60,
                i == p->selectedItem ?  60 : 60,
                i == p->selectedItem ?  60 : 120, 220);
            SDL_RenderFillRect(renderer, &r);
        }
        return;
    }

    /* Title */
    SDL_Color white  = {255, 255, 255, 255};
    SDL_Color yellow = {255, 230,   0, 255};
    SDL_Surface *ts = TTF_RenderText_Blended(f, "– PAUSED –", yellow);
    if (ts) {
        SDL_Texture *tt = SDL_CreateTextureFromSurface(renderer, ts);
        SDL_Rect tr = {800 / 2 - ts->w / 2, 600 / 2 - 140, ts->w, ts->h};
        SDL_RenderCopy(renderer, tt, NULL, &tr);
        SDL_FreeSurface(ts); SDL_DestroyTexture(tt);
    }

    /* Menu items */
    int bx     = 800 / 2 - 150;
    int bw     = 300;
    int bh     = 50;
    int startY = 600 / 2 - 30;
    int gapY   = 70;

    for (int i = 0; i < PAUSE_ITEM_COUNT; i++) {
        int by = startY + i * gapY;
        SDL_Rect rect = {bx, by, bw, bh};

        /* Button background */
        if (i == p->selectedItem)
            SDL_SetRenderDrawColor(renderer, 180, 50, 50, 220);
        else
            SDL_SetRenderDrawColor(renderer, 50, 50, 100, 200);
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 200, 200, 255, 255);
        SDL_RenderDrawRect(renderer, &rect);

        /* Label */
        SDL_Color col = (i == p->selectedItem) ? yellow : white;
        SDL_Surface *ls = TTF_RenderText_Blended(f, s_labels[i], col);
        if (ls) {
            SDL_Texture *lt = SDL_CreateTextureFromSurface(renderer, ls);
            SDL_Rect lr = {bx + (bw - ls->w) / 2,
                           by  + (bh - ls->h) / 2,
                           ls->w, ls->h};
            SDL_RenderCopy(renderer, lt, NULL, &lr);
            SDL_FreeSurface(ls); SDL_DestroyTexture(lt);
        }
    }

    /* Navigation hint */
    SDL_Color grey = {180, 180, 180, 200};
    SDL_Surface *hs = TTF_RenderText_Blended(f, "W/S or Arrow keys  |  Enter to select", grey);
    if (hs) {
        SDL_Texture *ht = SDL_CreateTextureFromSurface(renderer, hs);
        SDL_Rect hr = {800 / 2 - hs->w / 2, startY + PAUSE_ITEM_COUNT * gapY + 10,
                       hs->w, hs->h};
        SDL_RenderCopy(renderer, ht, NULL, &hr);
        SDL_FreeSurface(hs); SDL_DestroyTexture(ht);
    }
}
