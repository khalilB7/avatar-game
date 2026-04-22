/*
 * main.c — Unified entry point
 *
 * Controls
 * --------
 *  P1  Move  : Arrow keys       P2  Move  : W A S D
 *      Shoot : K                    Shoot : L
 *      Dash  : M                    Dash  : C
 *      Jump  : N                    Jump  : Space
 *
 *  P   : Add second player (switches to split-screen)
 *  ESC : Quit
 *
 * Assets  (place in  images/  next to the binary)
 * ------
 *  images/background.png
 *  images/arial.ttf
 *  images/soldierR.png   images/soldierL.png   (P1 sprite sheet)
 *  images/C1R.png        images/C1L.png        (P2 sprite sheet)
 *  images/Icon12.png                           (health icon)
 *  images/MiniMap.jpeg
 *  images/enemy_walk0.png  enemy_walk1.png  enemy_walk2.png
 *  images/enemy_attack.png
 *  All images are optional – coloured rectangles are used as fallbacks.
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "hero.h"
#include "enemy.h"
#include "game.h"
#include "partage.h"
#include "minimap.h"

#define MAX_PLAYERS  2
#define NUM_ENEMIES  4
#define TARGET_FPS  60

/* ── Nearest hero position (for enemy AI) ─────────────────────────── */
static void nearestHeroPos(Hero *players, int numPlayers,
                            int ex, int ey, int *outX, int *outY)
{
    *outX = players[0].posHero.x + players[0].frameWidth  / 2;
    *outY = players[0].posHero.y + players[0].frameHeight / 2;
    if (numPlayers < 2) return;

    int d0  = abs(ex - *outX) + abs(ey - *outY);
    int cx1 = players[1].posHero.x + players[1].frameWidth  / 2;
    int cy1 = players[1].posHero.y + players[1].frameHeight / 2;
    int d1  = abs(ex - cx1) + abs(ey - cy1);
    if (d1 < d0) { *outX = cx1; *outY = cy1; }
}

/* ── Bullet → enemy collision ─────────────────────────────────────── */
static void checkBulletEnemyHit(Hero *shooter, Enemy *enemies, int count)
{
    for (int b = 0; b < MAX_BULLETS; b++) {
        if (!shooter->bullets[b].active) continue;
        SDL_Rect br = {
            (int)shooter->bullets[b].x,
            (int)shooter->bullets[b].y,
            12, 5
        };
        for (int e = 0; e < count; e++) {
            if (!enemies[e].active || enemies[e].health <= 0) continue;
            SDL_Rect er = {
                enemies[e].x + 8, enemies[e].y + 8,
                enemies[e].w - 16, enemies[e].h - 16
            };
            if (checkCollision(br, er)) {
                shooter->bullets[b].active = 0;
                enemies[e].health--;
                if (enemies[e].health <= 0) {
                    enemies[e].active = 0;
                    addKillScore(shooter);
                }
                break;
            }
        }
    }
}

/* ── Enemy body → hero damage ────────────────────────────────────── */
static void checkEnemyHeroContact(Enemy *enemies, int eCount,
                                   Hero *players, int pCount)
{
    for (int e = 0; e < eCount; e++) {
        if (!enemies[e].active || enemies[e].health <= 0) continue;
        SDL_Rect er = {enemies[e].x, enemies[e].y, enemies[e].w, enemies[e].h};
        for (int p = 0; p < pCount; p++) {
            if (players[p].dead || players[p].invincible) continue;
            SDL_Rect hr = {
                players[p].posHero.x + 16,
                players[p].posHero.y + 16,
                players[p].frameWidth  - 32,
                players[p].frameHeight - 24
            };
            if (checkCollision(er, hr))
                takeDamage(&players[p]);
        }
    }
}

/* ── Draw enemies into the current viewport ───────────────────────── */
static void drawEnemies(Enemy *enemies, int count,
                        SDL_Renderer *renderer, int camX, int camY)
{
    for (int i = 0; i < count; i++)
        drawEnemy(&enemies[i], renderer, camX, camY);
}

/* ══════════════════════════════════════════════════════════════════════
 * main()
 * ══════════════════════════════════════════════════════════════════════ */
int main(void)
{
    srand((unsigned int)time(NULL));

    /* ── SDL init ─────────────────────────────────────────────────── */
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
    if (!window) { fprintf(stderr, "Window: %s\n", SDL_GetError()); return 1; }

    SDL_Renderer *renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) { fprintf(stderr, "Renderer: %s\n", SDL_GetError()); return 1; }

    TTF_Font *font = TTF_OpenFont("/home/khalil/op/avatar-game-khalilB7-version2/strengh and wisdom/avatar-game-khalilB7-version2/the game/images/arial.ttf", 22);
    if (!font)
        fprintf(stderr, "Warning – font not loaded: %s\n", TTF_GetError());

    /* ── Subsystems ───────────────────────────────────────────────── */
    Game      world;
    SplitGame split;
    Minimap   minimap;

    initGame(&world, renderer);
    initSplitGame(&split, renderer);
    initMinimap(&minimap, renderer);

    /* ── Players ──────────────────────────────────────────────────── */
    Hero players[MAX_PLAYERS];
    int  numPlayers = 1;
    memset(players, 0, sizeof(players));
    initializePlayer(&players[0], 1, renderer);
    players[0].scoreActive = 1;   /* P1 scores from the start */
    /* bounds & ground position set below after worldH is known */

    /* ── Enemies ──────────────────────────────────────────────────── */
    Enemy enemies[NUM_ENEMIES];
    memset(enemies, 0, sizeof(enemies));

    int worldW  = (split.bg_w > 0) ? split.bg_w : SCREEN_WIDTH;
    int worldH  = (split.bg_h > 0) ? split.bg_h : SCREEN_HEIGHT;
    int groundY = worldH - 128 - 10; /* same height for players and enemies */

    /* Bind players to the real world size so they stand on the background */
    setHeroBounds(&players[0], worldW, worldH);
    players[0].posHero.y = groundY;

    for (int i = 0; i < NUM_ENEMIES; i++) {
        int spawnX = (worldW / (NUM_ENEMIES + 1)) * (i + 1);
        initEnemy(&enemies[i], spawnX, groundY, renderer);
    }

    /* ── Frame timer ──────────────────────────────────────────────── */
    const int frameDelay = 1000 / TARGET_FPS;

    /* ══════════════════════════════════════════════════════════════
     * Main loop
     * ══════════════════════════════════════════════════════════════ */
    SDL_Event ev;
    int running = 1;

    while (running)
    {
        Uint32 frameStart = SDL_GetTicks();

        /* ── Events ───────────────────────────────────────────────── */
        while (SDL_PollEvent(&ev))
        {
            if (ev.type == SDL_QUIT) { running = 0; break; }

            if (ev.type == SDL_KEYDOWN)
            {
                SDL_Keycode k = ev.key.keysym.sym;

                if (k == SDLK_ESCAPE) { running = 0; break; }

                /* Add P2 */
                if (k == SDLK_p && numPlayers < 2) {
                    initializePlayer(&players[1], 2, renderer);
                    players[1].scoreActive = 1;   /* score starts now, not before */
                    numPlayers = 2;
                    setHeroBounds(&players[0], worldW, worldH);
                    setHeroBounds(&players[1], worldW, worldH);
                    players[1].posHero.y = groundY;
                }

                /* P1 actions */
                if (!players[0].dead) {
                    if (k == SDLK_k) {
                        shoot(&players[0]);
                    } else if (k == SDLK_m && players[0].state != STATE_DASH &&
                               players[0].state != STATE_DEAD) {
                        setState_public(&players[0], STATE_DASH);
                        players[0].dashTime = 10;
                    } else if (k == SDLK_n &&
                               players[0].state != STATE_JUMP &&
                               players[0].state != STATE_DEAD) {
                        setState_public(&players[0], STATE_JUMP);
                        players[0].jumpTime = 30;
                        players[0].jumpVy   = -10.0f;
                    }
                }

                /* P2 actions */
                if (numPlayers > 1 && !players[1].dead) {
                    if (k == SDLK_l) {
                        shoot(&players[1]);
                    } else if (k == SDLK_c && players[1].state != STATE_DASH &&
                               players[1].state != STATE_DEAD) {
                        setState_public(&players[1], STATE_DASH);
                        players[1].dashTime = 10;
                    } else if (k == SDLK_SPACE &&
                               players[1].state != STATE_JUMP &&
                               players[1].state != STATE_DEAD) {
                        setState_public(&players[1], STATE_JUMP);
                        players[1].jumpTime = 30;
                        players[1].jumpVy   = -10.0f;
                    }
                }
            }
        }

        /* ── Held keys ────────────────────────────────────────────── */
        const Uint8 *keys = SDL_GetKeyboardState(NULL);

        if (!players[0].dead) {
            players[0].moveLeft  = keys[SDL_SCANCODE_LEFT];
            players[0].moveRight = keys[SDL_SCANCODE_RIGHT];
            players[0].moveUp    = keys[SDL_SCANCODE_UP];
            players[0].moveDown  = keys[SDL_SCANCODE_DOWN];
        } else {
            players[0].moveLeft = players[0].moveRight =
            players[0].moveUp   = players[0].moveDown  = 0;
        }

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

        /* ── Update: heroes ───────────────────────────────────────── */
        for (int i = 0; i < numPlayers; i++) {
            moveHero(&players[i]);
            updateBullets(&players[i]);
            updateScore(&players[i]);
        }

        /* Hero ↔ hero bullets — friendly fire disabled */
        /* checkBulletHit(&players[0], &players[1]); */
        /* checkBulletHit(&players[1], &players[0]); */

        /* ── Update: enemies ──────────────────────────────────────── */
        for (int i = 0; i < NUM_ENEMIES; i++) {
            if (!enemies[i].active) continue;
            int tx, ty;
            nearestHeroPos(players, numPlayers,
                           enemies[i].x, enemies[i].y, &tx, &ty);
            moveEnemy(&enemies[i], tx, ty, worldW);
        }

        /* Bullets → enemies */
        for (int p = 0; p < numPlayers; p++)
            checkBulletEnemyHit(&players[p], enemies, NUM_ENEMIES);

        /* Enemy bodies → heroes */
        checkEnemyHeroContact(enemies, NUM_ENEMIES, players, numPlayers);

        /* ── Camera ───────────────────────────────────────────────── */
        updateSplitGame(&split, players, numPlayers);

        /* ── Minimap dots ─────────────────────────────────────────── */
        minimapClear(&minimap);
        SDL_Color p1col  = { 80, 180, 255, 255};
        SDL_Color p2col  = {255, 100,  50, 255};
        SDL_Color encol  = {255,  40,  40, 255};

        minimapAddDot(&minimap,
                      players[0].posHero.x, players[0].posHero.y,
                      worldW, SCREEN_HEIGHT, p1col);
        if (numPlayers > 1)
            minimapAddDot(&minimap,
                          players[1].posHero.x, players[1].posHero.y,
                          worldW, SCREEN_HEIGHT, p2col);
        for (int i = 0; i < NUM_ENEMIES && minimap.numDots < 4; i++)
            if (enemies[i].active)
                minimapAddDot(&minimap,
                              enemies[i].x, enemies[i].y,
                              worldW, SCREEN_HEIGHT, encol);

        /* ════════════════════════════════════════════════════════════
         * Draw
         * ════════════════════════════════════════════════════════════ */
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        /* Background + heroes + bullets + per-viewport HUD */
        renderSplitGame(&split, renderer, players, numPlayers, font);

        /* Enemies overlaid in the correct viewport(s) */
        if (numPlayers == 1) {
            drawEnemies(enemies, NUM_ENEMIES, renderer,
                        split.camera[0].x, split.camera[0].y);
        } else {
            for (int p = 0; p < 2; p++) {
                SDL_Rect vp = {p * SPLIT_VIEW_W, 0, SPLIT_VIEW_W, SPLIT_VIEW_H};
                SDL_RenderSetViewport(renderer, &vp);
                drawEnemies(enemies, NUM_ENEMIES, renderer,
                            split.camera[p].x, split.camera[p].y);
            }
            SDL_RenderSetViewport(renderer, NULL);
        }

        /* Minimap overlay (top-right, full-screen space) */
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

        /* "Press P" hint */
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

    /* ── Cleanup ──────────────────────────────────────────────────── */
    for (int i = 0; i < numPlayers;  i++) freePlayer(&players[i]);
    for (int i = 0; i < NUM_ENEMIES; i++) freeEnemy(&enemies[i]);

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
