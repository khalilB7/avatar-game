#ifndef PAUSEMENU_H
#define PAUSEMENU_H

/*
 * pausemenu.h  –  In-game pause overlay.
 *
 * Triggered by pressing H during STATE_PLAYING.
 * Three options: Resume (with 3-2-1 countdown), Save Game, Exit to Main Menu.
 *
 * The pause menu does NOT own a separate game state in AppState – it is a
 * sub-mode of STATE_PLAYING, controlled by a paused flag in main.c.
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

/* ── Menu items ─────────────────────────────────────────────────────────── */
typedef enum {
    PAUSE_ITEM_RESUME   = 0,
    PAUSE_ITEM_SAVE     = 1,
    PAUSE_ITEM_EXIT     = 2,
    PAUSE_ITEM_COUNT    = 3
} PauseItem;

/* ── Countdown state ────────────────────────────────────────────────────── */
typedef enum {
    COUNTDOWN_NONE   = 0,   /* not counting down                  */
    COUNTDOWN_ACTIVE = 1,   /* showing 3 / 2 / 1                  */
    COUNTDOWN_DONE   = 2    /* finished – main loop resumes game  */
} CountdownState;

/* ── Module struct ──────────────────────────────────────────────────────── */
typedef struct {
    bool           visible;          /* is the pause overlay open?     */

    int            selectedItem;     /* keyboard cursor                */

    CountdownState countdown;
    int            countdownValue;   /* 3, 2, 1                        */
    Uint32         countdownStart;   /* SDL_GetTicks() when digit began */

    TTF_Font      *font;             /* borrowed – not owned           */
    TTF_Font      *fontBig;          /* borrowed – not owned           */
} PauseMenu;

/* ── API ──────────────────────────────────────────────────────────────────
 *
 * PauseMenu_Open        – call when H is pressed
 * PauseMenu_HandleEvent – feed SDL events while visible or counting down
 *                         returns PAUSE_ITEM_RESUME / SAVE / EXIT,
 *                         or -1 if nothing actionable yet.
 * PauseMenu_Update      – call once per frame; advances countdown
 *                         returns true when gameplay may resume
 * PauseMenu_Render      – draw overlay (pause UI or countdown)
 */

void PauseMenu_Open        (PauseMenu *p);
int  PauseMenu_HandleEvent (PauseMenu *p, const SDL_Event *e);
bool PauseMenu_Update      (PauseMenu *p);   /* true = resume gameplay now */
void PauseMenu_Render      (PauseMenu *p, SDL_Renderer *renderer);

#endif /* PAUSEMENU_H */
