/*
 * savemenu.c  -  Save/Load menu with per-slot rich display.
 *
 * Each slot shows:
 *   • Slot number + character name (e.g. "Slot 1 – Soldier")
 *   • Survival time  (MM:SS)
 *   • Score
 * Layout uses proportional positioning so it looks fine in both
 * 800×600 windowed mode and SDL_WINDOW_FULLSCREEN_DESKTOP.
 */

#include "savemenu.h"
#include <stdio.h>
#include <string.h>

/* ── layout constants (fractions of 800×600 logical resolution) ─────── */
#define LOG_W   800
#define LOG_H   600

#define SLOT_X_FRAC   0.18f   /* left edge  = 18 % of width  */
#define SLOT_W_FRAC   0.64f   /* slot width = 64 % of width  */
#define SLOT_H_FRAC   0.11f   /* slot height= 11 % of height */
#define SLOT_GAP_FRAC 0.14f   /* gap between slots           */
#define SLOT_Y0_FRAC  0.24f   /* first slot top              */

/* ── helpers ─────────────────────────────────────────────────────────── */

/* Render text centred on (cx, y); returns rendered width */
static int renderTextCX(SDL_Renderer *r, TTF_Font *f,
                         const char *txt, SDL_Color col,
                         int cx, int y)
{
    if (!f || !txt || !txt[0]) return 0;
    SDL_Surface *s = TTF_RenderText_Blended(f, txt, col);
    if (!s) return 0;
    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
    SDL_Rect dst = { cx - s->w / 2, y, s->w, s->h };
    SDL_RenderCopy(r, t, NULL, &dst);
    int w = s->w;
    SDL_FreeSurface(s);
    SDL_DestroyTexture(t);
    return w;
}

/* Render text left-aligned at (x, y) */
static void renderTextL(SDL_Renderer *r, TTF_Font *f,
                         const char *txt, SDL_Color col, int x, int y)
{
    if (!f || !txt || !txt[0]) return;
    SDL_Surface *s = TTF_RenderText_Blended(f, txt, col);
    if (!s) return;
    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
    SDL_Rect dst = { x, y, s->w, s->h };
    SDL_RenderCopy(r, t, NULL, &dst);
    SDL_FreeSurface(s);
    SDL_DestroyTexture(t);
}

/* Render text right-aligned so it ends at (rx, y) */
static void renderTextR(SDL_Renderer *r, TTF_Font *f,
                         const char *txt, SDL_Color col, int rx, int y)
{
    if (!f || !txt || !txt[0]) return;
    SDL_Surface *s = TTF_RenderText_Blended(f, txt, col);
    if (!s) return;
    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
    SDL_Rect dst = { rx - s->w, y, s->w, s->h };
    SDL_RenderCopy(r, t, NULL, &dst);
    SDL_FreeSurface(s);
    SDL_DestroyTexture(t);
}

/* ── layout helper ───────────────────────────────────────────────────── */
static void buildLayout(SaveMenu *s, int W, int H)
{
    int slotX = (int)(W * SLOT_X_FRAC);
    int slotW = (int)(W * SLOT_W_FRAC);
    int slotH = (int)(H * SLOT_H_FRAC);
    int gapH  = (int)(H * SLOT_GAP_FRAC);
    int y0    = (int)(H * SLOT_Y0_FRAC);

    for (int i = 0; i < SAVE_SLOT_COUNT; i++)
        s->slotRects[i] = (SDL_Rect){ slotX, y0 + i * gapH, slotW, slotH };

    /* Back button – top-left corner, small */
    s->btnBack = (SDL_Rect){ (int)(W*0.02f), (int)(H*0.03f),
                              (int)(W*0.12f), (int)(H*0.06f) };
}

/* ── public API ──────────────────────────────────────────────────────── */

bool SaveMenu_Init(SaveMenu *s, SDL_Renderer *renderer)
{
    s->pendingSlot   = -1;
    s->hoveredSlot   = -1;
    s->texBg         = NULL;

    if (s->pathTexBg)
        s->texBg = IMG_LoadTexture(renderer, s->pathTexBg);

    s->font    = NULL;
    s->fontBig = NULL;
    if (s->pathFont) {
        s->font    = TTF_OpenFont(s->pathFont, 20);
        s->fontBig = TTF_OpenFont(s->pathFont, 28);
    }
    if (!s->font)
        fprintf(stderr, "[SaveMenu] font missing: %s\n", TTF_GetError());

    buildLayout(s, LOG_W, LOG_H);
    SaveSystem_QuerySlots(s->slots);
    return true;
}

void SaveMenu_Refresh(SaveMenu *s)
{
    SaveSystem_QuerySlots(s->slots);
}

AppState SaveMenu_HandleEvent(SaveMenu *s, const SDL_Event *e)
{
    if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_ESCAPE)
        return STATE_MAIN_MENU;

    if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT) {
        SDL_Point p = { e->button.x, e->button.y };
        if (SDL_PointInRect(&p, &s->btnBack))
            return STATE_MAIN_MENU;

        for (int i = 0; i < SAVE_SLOT_COUNT; i++) {
            if (SDL_PointInRect(&p, &s->slotRects[i])) {
                if (s->slots[i].exists) {
                    s->pendingSlot = i;
                    return STATE_PLAYING;
                }
                /* clicking empty slot does nothing */
            }
        }
    }

    /* Track hover for highlight */
    if (e->type == SDL_MOUSEMOTION) {
        SDL_Point p = { e->motion.x, e->motion.y };
        s->hoveredSlot = -1;
        for (int i = 0; i < SAVE_SLOT_COUNT; i++)
            if (SDL_PointInRect(&p, &s->slotRects[i]))
                s->hoveredSlot = i;
    }

    return STATE_SAVE_LOAD;
}

void SaveMenu_Render(SaveMenu *s, SDL_Renderer *renderer)
{
    /* Query current output size so layout scales with fullscreen */
    int W, H;
    SDL_GetRendererOutputSize(renderer, &W, &H);
    buildLayout(s, W, H);   /* recalculate every frame – cheap */

    /* ── Background ─────────────────────────────────────────────── */
    if (!s->skipBackground) {
        if (s->texBg) {
            SDL_RenderCopy(renderer, s->texBg, NULL, NULL);
        } else {
            SDL_SetRenderDrawColor(renderer, 10, 18, 42, 255);
            SDL_RenderClear(renderer);
            /* subtle grid */
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 8);
            for (int x = 0; x < W; x += 40) SDL_RenderDrawLine(renderer,x,0,x,H);
            for (int y = 0; y < H; y += 40) SDL_RenderDrawLine(renderer,0,y,W,y);
        }
    }

    TTF_Font *f    = s->font;
    TTF_Font *fBig = s->fontBig ? s->fontBig : f;

    /* ── Title ───────────────────────────────────────────────────── */
    {
        SDL_Color gold = {255, 215, 0, 255};
        renderTextCX(renderer, fBig, "SAVE / LOAD",
                     gold, W / 2, (int)(H * 0.06f));

        SDL_Color hint = {160, 200, 255, 180};
        renderTextCX(renderer, f,
                     "Click a slot to continue that saved game",
                     hint, W / 2, (int)(H * 0.14f));
    }

    /* ── Slots ───────────────────────────────────────────────────── */
    int fontH = f ? TTF_FontHeight(f) : 18;

    for (int i = 0; i < SAVE_SLOT_COUNT; i++) {
        SDL_Rect  r        = s->slotRects[i];
        bool      occupied = s->slots[i].exists;
        bool      hovered  = (s->hoveredSlot == i);

        /* Slot background */
        if (occupied && hovered)
            SDL_SetRenderDrawColor(renderer, 40, 110, 190, 240);
        else if (occupied)
            SDL_SetRenderDrawColor(renderer, 25,  70, 140, 220);
        else
            SDL_SetRenderDrawColor(renderer, 20,  20,  40, 160);
        SDL_RenderFillRect(renderer, &r);

        /* Slot border */
        if (occupied && hovered)
            SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
        else if (occupied)
            SDL_SetRenderDrawColor(renderer, 80, 160, 255, 255);
        else
            SDL_SetRenderDrawColor(renderer, 50,  50,  80, 200);
        SDL_RenderDrawRect(renderer, &r);

        /* Inner highlight on hover */
        if (hovered && occupied) {
            SDL_Rect inner = {r.x+2, r.y+2, r.w-4, r.h-4};
            SDL_SetRenderDrawColor(renderer, 255, 215, 0, 60);
            SDL_RenderDrawRect(renderer, &inner);
        }

        int pad   = r.w / 40;   /* ~2.5% of slot width */
        int vMid  = r.y + r.h / 2;

        if (occupied) {
            /* ── Slot number badge (left) ── */
            char badge[8];
            snprintf(badge, sizeof(badge), "%d", i + 1);
            SDL_Color badgeCol = hovered
                ? (SDL_Color){255, 230, 60, 255}
                : (SDL_Color){200, 200, 255, 255};

            /* Draw a small circle/square badge */
            int bx = r.x + pad;
            int bs = r.h - 2 * pad;
            SDL_Rect badgeRect = {bx, r.y + pad, bs, bs};
            SDL_SetRenderDrawColor(renderer, 20, 40, 90, 200);
            SDL_RenderFillRect(renderer, &badgeRect);
            SDL_SetRenderDrawColor(renderer, badgeCol.r, badgeCol.g,
                                   badgeCol.b, 200);
            SDL_RenderDrawRect(renderer, &badgeRect);
            if (f) {
                SDL_Surface *bs2 = TTF_RenderText_Blended(f, badge, badgeCol);
                if (bs2) {
                    SDL_Texture *bt = SDL_CreateTextureFromSurface(renderer, bs2);
                    SDL_Rect br = {
                        bx + (bs - bs2->w) / 2,
                        r.y + pad + (bs - bs2->h) / 2,
                        bs2->w, bs2->h
                    };
                    SDL_RenderCopy(renderer, bt, NULL, &br);
                    SDL_FreeSurface(bs2);
                    SDL_DestroyTexture(bt);
                }
            }

            int textX  = r.x + bs + 3 * pad;   /* after badge */
            int rightX = r.x + r.w - pad;

            /* ── Character name (top-left of text area) ── */
            char header[80];
            snprintf(header, sizeof(header), "%s",
                     s->slots[i].playerName[0]
                         ? s->slots[i].playerName : "Player");
            if (s->slots[i].numPlayers == 2) {
                char tmp[80];
                snprintf(tmp, sizeof(tmp), "%s  +  Player 2", header);
                strncpy(header, tmp, sizeof(header)-1);
            }
            SDL_Color nameCol = hovered
                ? (SDL_Color){255, 230,  80, 255}
                : (SDL_Color){255, 255, 255, 255};
            renderTextL(renderer, fBig ? fBig : f, header,
                        nameCol, textX, vMid - fontH);

            /* ── Survival time (bottom-left) ── */
            unsigned secs = (unsigned)s->slots[i].elapsedSeconds;
            char timeTxt[32];
            snprintf(timeTxt, sizeof(timeTxt),
                     "Time:  %u:%02u", secs / 60, secs % 60);
            SDL_Color timeCol = {120, 210, 255, 230};
            renderTextL(renderer, f, timeTxt, timeCol,
                        textX, vMid + 2);

            /* ── Score (bottom-right) ── */
            char scoreTxt[32];
            snprintf(scoreTxt, sizeof(scoreTxt),
                     "Score:  %d", s->slots[i].score);
            SDL_Color scoreCol = {80, 230, 120, 230};
            renderTextR(renderer, f, scoreTxt, scoreCol,
                        rightX, vMid + 2);

            /* ── "Click to load" hint on hover ── */
            if (hovered) {
                SDL_Color loadHint = {255, 215, 0, 200};
                renderTextR(renderer, f, "▶  Click to load",
                            loadHint, rightX, vMid - fontH);
            }

        } else {
            /* Empty slot */
            char label[32];
            snprintf(label, sizeof(label), "Slot %d   [ empty ]", i + 1);
            SDL_Color grey = {100, 100, 120, 200};
            renderTextCX(renderer, f, label, grey,
                         r.x + r.w / 2, vMid - fontH / 2);
        }
    }

    /* ── Back button ─────────────────────────────────────────────── */
    {
        SDL_Rect b = s->btnBack;
        SDL_SetRenderDrawColor(renderer, 55, 30, 30, 210);
        SDL_RenderFillRect(renderer, &b);
        SDL_SetRenderDrawColor(renderer, 200, 160, 160, 255);
        SDL_RenderDrawRect(renderer, &b);
        SDL_Color pink = {255, 180, 180, 255};
        renderTextCX(renderer, f, "< Back",
                     pink, b.x + b.w / 2, b.y + (b.h - fontH) / 2);
    }

    /* ── ESC hint (bottom) ──────────────────────────────────────── */
    {
        SDL_Color hint2 = {130, 130, 130, 160};
        renderTextCX(renderer, f, "ESC = Back to Main Menu",
                     hint2, W / 2, (int)(H * 0.94f));
    }
}

void SaveMenu_Free(SaveMenu *s)
{
    if (s->texBg)    { SDL_DestroyTexture(s->texBg); s->texBg  = NULL; }
    if (s->font)     { TTF_CloseFont(s->font);        s->font   = NULL; }
    if (s->fontBig)  { TTF_CloseFont(s->fontBig);     s->fontBig = NULL; }
    memset(s, 0, sizeof(*s));
}
