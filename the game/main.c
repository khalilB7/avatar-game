/*
 * main.c — Entry point
 *
 * Architecture
 * ============
 *  hero.c/h     : Animated sprites, movement, jumping, dashing,
 *                 shooting, health, score.
 *  game.c/h     : Background texture, single-camera world.
 *  partage.c/h  : Split-screen subsystem (1 or 2 viewports).
 *  minimap.c/h  : Minimap overlay, AABB collision helper.
 *
 * Controls
 * --------
 *  P1  Move  : Arrow keys
 *      Shoot : K
 *      Dash  : M
 *      Jump  : N
 *
 *  P2  Move  : W A S D
 *      Shoot : L
 *      Dash  : C
 *      Jump  : Space
 *
 *  P   : Add second player
 *  ESC : Quit
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string.h>

#include "hero.h"
#include "game.h"
#include "partage.h"
#include "minimap.h"

#define MAX_PLAYERS 2
#define TARGET_FPS  60

/* ========================================================================= */
/* Main                                                                       */
/* ========================================================================= */
int main(void)
{
    /* ----- SDL init ----------------------------------------------------- */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    TTF_Init();
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

    SDL_Window *window = SDL_CreateWindow(
        "2-Player Arena",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, 0);

    SDL_Renderer *renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    TTF_Font *font = TTF_OpenFont(
        "/home/khalil/the game/images/arial.ttf", 22);
    if (!font)
        fprintf(stderr, "Warning: could not load font: %s\n", TTF_GetError());

    /* ----- Subsystems --------------------------------------------------- */
    Game      world;
    SplitGame split;
    Minimap   minimap;

    initGame(&world, renderer);
    initSplitGame(&split, renderer);
    initMinimap(&minimap, renderer);

    /* ----- Players ------------------------------------------------------ */
    Hero players[MAX_PLAYERS];
    int  numPlayers = 1;
    memset(players, 0, sizeof(players));
    initializePlayer(&players[0], 1, renderer);
    /* P1 starts alone → full window */
    setHeroBounds(&players[0], SCREEN_WIDTH, SCREEN_HEIGHT);

    /* ----- Frame timer -------------------------------------------------- */
    const int frameDelay = 1000 / TARGET_FPS;

    /* ===================================================================== */
    /* Main loop                                                              */
    /* ===================================================================== */
    SDL_Event e;
    int running = 1;

    while (running)
    {
        Uint32 frameStart = SDL_GetTicks();

        /* ---- Events ------------------------------------------------------ */
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT) { running = 0; break; }

            if (e.type == SDL_KEYDOWN)
            {
                SDL_Keycode k = e.key.keysym.sym;

                /* Global */
                if (k == SDLK_ESCAPE) { running = 0; break; }

                /* Add second player */
                if (k == SDLK_p && numPlayers < 2) {
                    initializePlayer(&players[1], 2, renderer);
                    numPlayers = 2;
                    /* Switch to split-screen: each player confined to their half */
                    setHeroBounds(&players[0], SPLIT_VIEW_W, SCREEN_HEIGHT);
                    setHeroBounds(&players[1], SPLIT_VIEW_W, SCREEN_HEIGHT);
                }

                /* ---- P1 actions ----------------------------------------- */
                if (!players[0].dead) {
                    if (k == SDLK_k) {
                        shoot(&players[0]);
                    }
                    else if (k == SDLK_m &&
                             players[0].state != STATE_DEAD) {
                        players[0].state     = STATE_DASH;
                        players[0].maxFrames = FRAMES_DASH;
                        players[0].dashTime  = 10;
                    }
                    else if (k == SDLK_n &&
                             players[0].state != STATE_JUMP &&
                             players[0].state != STATE_DEAD) {
                        players[0].state     = STATE_JUMP;
                        players[0].maxFrames = FRAMES_JUMP;
                        players[0].jumpTime  = 30;
                        players[0].jumpVy    = -10.0f;
                    }
                }

                /* ---- P2 actions ----------------------------------------- */
                if (numPlayers > 1 && !players[1].dead) {
                    if (k == SDLK_l) {
                        shoot(&players[1]);
                    }
                    else if (k == SDLK_c &&
                             players[1].state != STATE_DEAD) {
                        players[1].state     = STATE_DASH;
                        players[1].maxFrames = FRAMES_DASH;
                        players[1].dashTime  = 10;
                    }
                    else if (k == SDLK_SPACE &&
                             players[1].state != STATE_JUMP &&
                             players[1].state != STATE_DEAD) {
                        players[1].state     = STATE_JUMP;
                        players[1].maxFrames = FRAMES_JUMP;
                        players[1].jumpTime  = 30;
                        players[1].jumpVy    = -10.0f;
                    }
                }
            }
        }

        /* ---- Keyboard state (held keys) ---------------------------------- */
        const Uint8 *keys = SDL_GetKeyboardState(NULL);

        /* P1 — arrow keys */
        if (!players[0].dead) {
            players[0].moveLeft  = keys[SDL_SCANCODE_LEFT];
            players[0].moveRight = keys[SDL_SCANCODE_RIGHT];
            players[0].moveUp    = keys[SDL_SCANCODE_UP];
            players[0].moveDown  = keys[SDL_SCANCODE_DOWN];
        } else {
            players[0].moveLeft = players[0].moveRight =
            players[0].moveUp   = players[0].moveDown  = 0;
        }

        /* P2 — WASD */
        if (numPlayers > 1) {
            if (!players[1].dead) {
                players[1].moveLeft  = keys[SDL_SCANCODE_A];
                players[1].moveRight = keys[SDL_SCANCODE_D];
                players[1].moveUp    = keys[SDL_SCANCODE_W];
                players[1].moveDown  = keys[SDL_SCANCODE_S];
            } else {
                players[1].moveLeft = players[1].moveRight =
                players[1].moveUp   = players[1].moveDown  = 0;
            }
        }

        /* ---- Update ------------------------------------------------------- */

        for (int i = 0; i < numPlayers; i++) {
            moveHero(&players[i]);   /* clamps internally via hero->worldW/H */
            updateBullets(&players[i]);
            updateScore(&players[i]);
        }

        /* Mutual bullet collision */
        if (numPlayers == 2) {
            checkBulletHit(&players[0], &players[1]);
            checkBulletHit(&players[1], &players[0]);
        }

        /* Split-screen camera update */
        updateSplitGame(&split, players, numPlayers);

        /* Minimap dots */
        minimapClear(&minimap);
        SDL_Color p1color = {80,  180, 255, 255};
        SDL_Color p2color = {255, 100,  50, 255};
        minimapAddDot(&minimap,
                      players[0].posHero.x, players[0].posHero.y,
                      SCREEN_WIDTH, SCREEN_HEIGHT, p1color);
        if (numPlayers > 1)
            minimapAddDot(&minimap,
                          players[1].posHero.x, players[1].posHero.y,
                          SCREEN_WIDTH, SCREEN_HEIGHT, p2color);

        /* ---- Draw --------------------------------------------------------- */
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        /*
         * We rely entirely on the split-screen subsystem for rendering
         * backgrounds, heroes, bullets, and per-viewport HUDs.
         * The single-player path renders full-screen; two-player renders
         * two side-by-side viewports.
         */
        renderSplitGame(&split, renderer, players, numPlayers, font);

        /* Make sure viewport is reset after split rendering */
        SDL_RenderSetViewport(renderer, NULL);

        /* Minimap (top-right overlay) */
        renderMinimap(&minimap, renderer);

        /* Global timer (centred, top) */
        if (font) {
            Uint32 secs = (SDL_GetTicks() - split.startTime) / 1000;
            char txt[32];
            sprintf(txt, "%u s", secs);
            SDL_Color white = {255, 255, 255, 255};
            SDL_Surface *ts = TTF_RenderText_Solid(font, txt, white);
            if (ts) {
                SDL_Texture *tt = SDL_CreateTextureFromSurface(renderer, ts);
                SDL_Rect tp = {SCREEN_WIDTH / 2 - ts->w / 2, 6, ts->w, ts->h};
                SDL_RenderCopy(renderer, tt, NULL, &tp);
                SDL_FreeSurface(ts);
                SDL_DestroyTexture(tt);
            }
        }

        /* Controls hint when only 1 player */
        if (numPlayers == 1 && font) {
            SDL_Color yellow = {255, 230, 0, 200};
            SDL_Surface *hs = TTF_RenderText_Solid(font,
                "Press P to add Player 2", yellow);
            if (hs) {
                SDL_Texture *ht = SDL_CreateTextureFromSurface(renderer, hs);
                SDL_Rect hp = {SCREEN_WIDTH / 2 - hs->w / 2,
                               SCREEN_HEIGHT - hs->h - 8, hs->w, hs->h};
                SDL_RenderCopy(renderer, ht, NULL, &hp);
                SDL_FreeSurface(hs);
                SDL_DestroyTexture(ht);
            }
        }

        SDL_RenderPresent(renderer);

        /* Frame cap */
        int elapsed = (int)(SDL_GetTicks() - frameStart);
        if (frameDelay > elapsed)
            SDL_Delay(frameDelay - elapsed);
    }

    /* ---- Cleanup -------------------------------------------------------- */
    for (int i = 0; i < numPlayers; i++)
        freePlayer(&players[i]);

    freeGame(&world);
    freeSplitGame(&split);
    freeMinimap(&minimap);

    if (font) TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    return 0;
}
