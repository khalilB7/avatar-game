/*
 * main.c  –  Integrated entry point.
 *
 * States
 * ──────
 *   STATE_MAIN_MENU  → mainmenu.c
 *   STATE_OPTIONS    → optionsmenu.c
 *   STATE_SAVE_LOAD  → savemenu.c
 *   STATE_HIGHSCORE  → highscore.c
 *   STATE_PLAYING    → game logic  (with in-game pause overlay)
 *
 * NEW in this version
 * ───────────────────
 *   • Press H during gameplay → pause menu overlay (pausemenu.c)
 *       Resume  → 3-2-1 countdown, then gameplay continues
 *       Save    → writes savegame.bin + appends score to players.txt
 *       Exit    → back to main menu
 *   • Main menu gains "Continue" option when a save file exists
 *       (detected at startup; uses MM_BTN_COUNT slot via keyboard hint)
 *   • Save/load system (savesystem.c) persists positions, health,
 *     scores, elapsed time, enemy states
 *   • Every save automatically updates the high-score leaderboard
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "gamestate.h"
#include "mainmenu.h"
#include "optionsmenu.h"
#include "savemenu.h"
#include "highscore.h"

#include "hero.h"
#include "enemy.h"
#include "game.h"
#include "partage.h"
#include "minimap.h"

/* NEW modules */
#include "pausemenu.h"
#include "savesystem.h"
#include "charselect.h"

/* ═══════════════════════════════════════════════════════════════════════
 * A S S E T   P A T H S
 * ═══════════════════════════════════════════════════════════════════════ */

#define BASE  "/home/khalil/op/avatar-game-khalilB7-version2/strengh and wisdom/avatar-game-khalilB7-version2/the game/images/"
#define OPT   "/home/khalil/option/"

/* Main menu */
#define PATH_MM_BG        BASE "avatar.jpg"
#define PATH_MM_BTN       BASE "button.png"
#define PATH_MM_CORNER    BASE "cc.jpg"
#define PATH_MM_MUSIC     BASE "/home/khalil/0/the game/images/ost.mp3"
#define PATH_MM_HOVER_SFX BASE "/home/khalil/0/the game/images/ost.mp3"
#define PATH_FONT         BASE "arial.ttf"

/* Options menu */
#define PATH_OPT_BG       OPT  "official_avatar_movie_poster.png"
#define PATH_OPT_BTN_N    OPT  "grayb.png"
#define PATH_OPT_BTN_H    OPT  "redbut.jpg"

/* Save/load menu */
#define PATH_SAVE_TEX1    BASE "photo1.png"
#define PATH_SAVE_TEX2    BASE "photo2.png"

/* High-score menu */
#define PATH_HS_BG_INPUT  BASE "playername.jpg"
#define PATH_HS_BG_SCORE  BASE "bestscore.jpg"
#define PATH_SCORE_FILE        "players.txt"

/* Character select */
#define PATH_CS_PREVIEW1  BASE "soldierR.png"
#define PATH_CS_PREVIEW2  BASE "C1R.png"

/* ═══════════════════════════════════════════════════════════════════════
 * G A M E   C O N S T A N T S
 * ═══════════════════════════════════════════════════════════════════════ */

#define MAX_PLAYERS   2
#define NUM_ENEMIES  16   /* 4 initial; extra slots for wave spawns */
#define TARGET_FPS   60

/* ═══════════════════════════════════════════════════════════════════════
 * G A M E   C O N T E X T
 * ═══════════════════════════════════════════════════════════════════════ */

typedef struct {
    Game      world;
    SplitGame split;
    Minimap   minimap;
    Hero      players[MAX_PLAYERS];
    Enemy     enemies[NUM_ENEMIES];
    int       numPlayers;
    int       worldW, worldH, groundY;
    int       initialised;
    Uint32    pausedElapsed;   /* seconds elapsed before last pause */
    Uint32    sessionStart;    /* SDL_GetTicks() when this session began */
    int       chosenPlayerId;  /* 1 or 2 – set by character select screen */
} GameContext;

/* ── helpers ─────────────────────────────────────────────────────────── */

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

static void checkBulletEnemyHit(Hero *shooter, Enemy *enemies, int count)
{
    for (int b = 0; b < MAX_BULLETS; b++) {
        if (!shooter->bullets[b].active) continue;
        SDL_Rect br = { (int)shooter->bullets[b].x,
                        (int)shooter->bullets[b].y, 12, 5 };
        for (int e = 0; e < count; e++) {
            if (!enemies[e].active || enemies[e].health <= 0) continue;
            SDL_Rect er = { enemies[e].x + 8, enemies[e].y + 8,
                            enemies[e].w - 16, enemies[e].h - 16 };
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

static void checkEnemyHeroContact(Enemy *enemies, int eCount,
                                   Hero *players, int pCount)
{
    for (int e = 0; e < eCount; e++) {
        if (!enemies[e].active || enemies[e].health <= 0) continue;
        SDL_Rect er = { enemies[e].x, enemies[e].y,
                        enemies[e].w, enemies[e].h };
        for (int p = 0; p < pCount; p++) {
            if (players[p].dead || players[p].invincible) continue;
            SDL_Rect hr = { players[p].posHero.x + 16,
                            players[p].posHero.y + 16,
                            players[p].frameWidth  - 32,
                            players[p].frameHeight - 24 };
            if (checkCollision(er, hr))
                takeDamage(&players[p]);
        }
    }
}

static void drawEnemies(Enemy *enemies, int count,
                        SDL_Renderer *renderer, int camX, int camY)
{
    for (int i = 0; i < count; i++)
        drawEnemy(&enemies[i], renderer, camX, camY);
}

static int allPlayersDead(Hero *players, int numPlayers)
{
    for (int i = 0; i < numPlayers; i++)
        if (!players[i].dead) return 0;
    return 1;
}

static int bestScore(Hero *players, int numPlayers)
{
    int best = 0;
    for (int i = 0; i < numPlayers; i++)
        if (players[i].score > best) best = players[i].score;
    return best;
}

/* Returns total elapsed game seconds accounting for pause time */
static Uint32 totalElapsed(const GameContext *gc)
{
    return gc->pausedElapsed + (SDL_GetTicks() - gc->sessionStart) / 1000;
}

/* ── GameContext lifecycle ────────────────────────────────────────────── */

static void GameContext_Init(GameContext *gc, SDL_Renderer *renderer)
{
    if (gc->initialised) return;

    int savedChosenId = gc->chosenPlayerId;   /* preserve selection */
    memset(gc, 0, sizeof(*gc));
    gc->numPlayers    = 1;
    gc->chosenPlayerId = (savedChosenId == 2) ? 2 : 1;

    initGame(&gc->world, renderer);
    initSplitGame(&gc->split, renderer);
    initMinimap(&gc->minimap, renderer);

    initializePlayer(&gc->players[0], gc->chosenPlayerId, renderer);
    gc->players[0].scoreActive = 1;

    gc->worldW  = (gc->split.bg_w > 0) ? gc->split.bg_w : SCREEN_WIDTH;
    gc->worldH  = (gc->split.bg_h > 0) ? gc->split.bg_h : SCREEN_HEIGHT;
    gc->groundY = gc->worldH - 128 - 10;

    setHeroBounds(&gc->players[0], gc->worldW, gc->worldH);
    gc->players[0].posHero.y = gc->groundY;

    /* Spawn only the first 4 enemies at start; the rest are wave-reserved */
    #define INITIAL_ENEMIES 4
    for (int i = 0; i < NUM_ENEMIES; i++) {
        if (i < INITIAL_ENEMIES) {
            int spawnX = (gc->worldW / (INITIAL_ENEMIES + 1)) * (i + 1);
            initEnemy(&gc->enemies[i], spawnX, gc->groundY, renderer);
        } else {
            memset(&gc->enemies[i], 0, sizeof(Enemy));  /* inactive slot */
        }
    }
    #undef INITIAL_ENEMIES

    gc->pausedElapsed = 0;
    gc->sessionStart  = SDL_GetTicks();
    gc->initialised   = 1;
}

/*
 * Restore from a save file.  Textures are already allocated by GameContext_Init;
 * SaveSystem_Load only patches numeric fields.
 */
static void GameContext_LoadSave(GameContext *gc, SDL_Renderer *renderer)
{
    GameContext_Init(gc, renderer);   /* allocate textures first */

    Uint32 saved_elapsed = 0;
    int    loaded_players = gc->numPlayers;

    if (SaveSystem_Load(gc->players, &loaded_players,
                        gc->enemies,  NUM_ENEMIES,
                        &saved_elapsed, SAVE_FILE)) {
        gc->numPlayers    = loaded_players;
        gc->pausedElapsed = saved_elapsed;
        gc->sessionStart  = SDL_GetTicks();

        /* Make sure P2 textures exist if save had 2 players */
        if (loaded_players > 1 && !gc->players[1].spriteSheetRight) {
            initializePlayer(&gc->players[1], 2, renderer);
            /* Re-patch stats that initializePlayer reset */
            SaveRecord dummy; /* just re-load cleanly */
            Uint32 dummy_elapsed = 0;
            int dummy_np = 2;
            SaveSystem_Load(gc->players, &dummy_np, gc->enemies,
                            NUM_ENEMIES, &dummy_elapsed, SAVE_FILE);
            gc->numPlayers    = dummy_np;
            gc->pausedElapsed = dummy_elapsed;
        }

        /* Re-apply bounds after position restore */
        for (int i = 0; i < gc->numPlayers; i++) {
            gc->players[i].scoreActive = 1;
            setHeroBounds(&gc->players[i], gc->worldW, gc->worldH);
        }
    }
}

static void GameContext_Free(GameContext *gc)
{
    if (!gc->initialised) return;
    for (int i = 0; i < gc->numPlayers;  i++) freePlayer(&gc->players[i]);
    for (int i = 0; i < NUM_ENEMIES; i++) freeEnemy(&gc->enemies[i]);
    freeGame(&gc->world);
    freeSplitGame(&gc->split);
    freeMinimap(&gc->minimap);
    gc->initialised = 0;
}

/* ═══════════════════════════════════════════════════════════════════════
 * M A I N
 * ═══════════════════════════════════════════════════════════════════════ */

int main(void)
{
    srand((unsigned int)time(NULL));

    /* ── SDL init ────────────────────────────────────────────────────── */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    TTF_Init();
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

    /* Mix_Init MUST be called before Mix_LoadMUS can decode MP3/OGG */
    int mix_flags = MIX_INIT_MP3 | MIX_INIT_OGG;
    if ((Mix_Init(mix_flags) & mix_flags) != mix_flags)
        fprintf(stderr, "Mix_Init: %s (some formats may be unavailable)\n",
                Mix_GetError());

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
        fprintf(stderr, "Mix_OpenAudio: %s (continuing without audio)\n",
                Mix_GetError());
    Mix_VolumeMusic(MIX_MAX_VOLUME);   /* ensure full volume at start */

    SDL_Window *window = SDL_CreateWindow(
        "2-Player Arena",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!window) { fprintf(stderr, "Window: %s\n", SDL_GetError()); return 1; }

    SDL_Renderer *renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) { fprintf(stderr, "Renderer: %s\n", SDL_GetError()); return 1; }

    TTF_Font *gameFont    = TTF_OpenFont(PATH_FONT, 22);
    TTF_Font *gameFontBig = TTF_OpenFont(PATH_FONT, 96);   /* countdown digits */
    if (!gameFont)
        fprintf(stderr, "Warning – game font not loaded: %s\n", TTF_GetError());

    /* ── Main menu ───────────────────────────────────────────────────── */
    MainMenu mm;
    memset(&mm, 0, sizeof(mm));
    mm.pathBg       = PATH_MM_BG;
    mm.pathBtn      = PATH_MM_BTN;
    mm.pathCorner   = PATH_MM_CORNER;
    mm.pathMusic    = PATH_MM_MUSIC;
    mm.pathHoverSfx = PATH_MM_HOVER_SFX;
    mm.pathFont     = PATH_FONT;
    MainMenu_Init(&mm, renderer);

    /* ── Options menu ────────────────────────────────────────────────── */
    OptionsMenu om;
    memset(&om, 0, sizeof(om));
    om.pathBackground = PATH_OPT_BG;
    om.pathFont       = PATH_FONT;
    om.pathBtnNormal  = PATH_OPT_BTN_N;
    om.pathBtnHover   = PATH_OPT_BTN_H;
    OptionsMenu_Init(&om, renderer);

    /* ── Save/load menu ──────────────────────────────────────────────── */
    SaveMenu sm;
    memset(&sm, 0, sizeof(sm));
    sm.pathTexBg = PATH_SAVE_TEX1;
    sm.pathFont  = PATH_FONT;
    SaveMenu_Init(&sm, renderer);

    /* ── High-score menu ─────────────────────────────────────────────── */
    HighScore hs;
    memset(&hs, 0, sizeof(hs));
    hs.pathFont      = PATH_FONT;
    hs.pathFontBig   = PATH_FONT;
    hs.pathBgInput   = PATH_HS_BG_INPUT;
    hs.pathBgScore   = PATH_HS_BG_SCORE;
    hs.pathScoreFile = PATH_SCORE_FILE;
    HighScore_Init(&hs, renderer, 0);

    /* ── Pause menu ──────────────────────────────────────────────────── */
    PauseMenu pm;
    memset(&pm, 0, sizeof(pm));
    pm.font    = gameFont;
    pm.fontBig = gameFontBig;

    /* ── Character select ────────────────────────────────────────────── */
    CharSelect cs;
    memset(&cs, 0, sizeof(cs));
    cs.pathFont     = PATH_FONT;
    cs.pathFontBig  = PATH_FONT;
    cs.pathPreview1 = PATH_CS_PREVIEW1;
    cs.pathPreview2 = PATH_CS_PREVIEW2;
    CharSelect_Init(&cs, renderer);

    /* ── Game context ────────────────────────────────────────────────── */
    GameContext gc;
    memset(&gc, 0, sizeof(gc));

    /* ── State machine ───────────────────────────────────────────────── */
    AppState  state     = STATE_MAIN_MENU;
    AppState  prevState = STATE_MAIN_MENU;
    const int frameDelay = 1000 / TARGET_FPS;

    /*
     * paused  – gameplay is frozen (pause menu open OR countdown running)
     * The gameplay update block is simply skipped while paused.
     */
    int paused = 0;

    int running = 1;
    while (running)
    {
        Uint32 frameStart = SDL_GetTicks();

        /* Music management – same track loops across all states */
        if (!Mix_PlayingMusic() && mm.bgMusic)
            Mix_PlayMusic(mm.bgMusic, -1);

        prevState = state;

        /* ══════════════════════════════════════════════════════════════
         * E V E N T   H A N D L I N G
         * ══════════════════════════════════════════════════════════════ */
        SDL_Event ev;
        while (SDL_PollEvent(&ev))
        {
            if (ev.type == SDL_QUIT) { running = 0; break; }

            switch (state)
            {
                /* ── Main menu ──────────────────────────────────────── */
                case STATE_MAIN_MENU:
                {
                    AppState next = MainMenu_HandleEvent(&mm, &ev);

                    /* "C" key = Continue saved game */
                    if (ev.type == SDL_KEYDOWN &&
                        ev.key.keysym.sym == SDLK_c &&
                        SaveSystem_Exists(SAVE_FILE))
                    {
                        GameContext_LoadSave(&gc, renderer);
                        paused = 0;
                        state  = STATE_PLAYING;
                        break;
                    }

                    if (next != STATE_MAIN_MENU) {
                        if (next == STATE_EXIT) {
                            running = 0;
                        } else if (next == STATE_HIGHSCORE) {
                            HighScore_SetScore(&hs, 0);
                            state = STATE_HIGHSCORE;
                        } else if (next == STATE_PLAYING) {
                            /* Go to character selection first */
                            cs.chosenId = 1;
                            cs.selected = 1;
                            state = STATE_CHAR_SELECT;
                        } else {
                            state = next;
                        }
                    }
                    break;
                }

                /* ── Character select ───────────────────────────────── */
                case STATE_CHAR_SELECT:
                {
                    AppState next = CharSelect_HandleEvent(&cs, &ev);
                    if (next == STATE_PLAYING) {
                        /* Launch fresh game with the chosen character */
                        if (gc.initialised) GameContext_Free(&gc);
                        gc.chosenPlayerId = cs.chosenId;
                        GameContext_Init(&gc, renderer);
                        paused = 0;
                        state  = STATE_PLAYING;
                    } else if (next == STATE_MAIN_MENU) {
                        state = STATE_MAIN_MENU;
                    }
                    break;
                }

                /* ── Options menu ───────────────────────────────────── */
                case STATE_OPTIONS:                {
                    AppState next = OptionsMenu_HandleEvent(&om, window, &ev, renderer);
                    if (next != STATE_OPTIONS) state = next;
                    break;
                }

                /* ── Save/load menu ─────────────────────────────────── */
                case STATE_SAVE_LOAD:
                {
                    AppState next = SaveMenu_HandleEvent(&sm, &ev);
                    if (next == STATE_PLAYING && sm.pendingSlot >= 0) {
                        /* Load the chosen slot */
                        static const char *slotFiles[5] = {
                            "savegame.bin",  "savegame1.bin", "savegame2.bin",
                            "savegame3.bin", "savegame4.bin"
                        };
                        const char *slotFile = slotFiles[sm.pendingSlot];
                        if (gc.initialised) GameContext_Free(&gc);
                        /* Use chosenPlayerId from save (default 1) */
                        gc.chosenPlayerId = 1;
                        GameContext_Init(&gc, renderer);
                        Uint32 saved_elapsed = 0;
                        int loaded_np = gc.numPlayers;
                        if (SaveSystem_Load(gc.players, &loaded_np,
                                            gc.enemies, NUM_ENEMIES,
                                            &saved_elapsed, slotFile)) {
                            gc.numPlayers    = loaded_np;
                            gc.pausedElapsed = saved_elapsed;
                            gc.sessionStart  = SDL_GetTicks();
                            /* Ensure P2 textures if save had 2 players */
                            if (loaded_np > 1 && !gc.players[1].spriteSheetRight) {
                                initializePlayer(&gc.players[1], 2, renderer);
                                Uint32 de2 = 0; int dnp = 2;
                                SaveSystem_Load(gc.players, &dnp, gc.enemies,
                                                NUM_ENEMIES, &de2, slotFile);
                                gc.numPlayers    = dnp;
                                gc.pausedElapsed = de2;
                            }
                            for (int _i = 0; _i < gc.numPlayers; _i++) {
                                gc.players[_i].scoreActive = 1;
                                setHeroBounds(&gc.players[_i],
                                              gc.worldW, gc.worldH);
                            }
                        }
                        sm.pendingSlot = -1;
                        paused = 0;
                        state  = STATE_PLAYING;
                    } else if (next != STATE_SAVE_LOAD) {
                        state = next;
                    }
                    break;
                }

                /* ── High-score menu ────────────────────────────────── */
                case STATE_HIGHSCORE:
                {
                    AppState next = HighScore_HandleEvent(&hs, &ev, renderer);
                    if (next != STATE_HIGHSCORE) {
                        if (gc.initialised) GameContext_Free(&gc);
                        state = next;
                    }
                    break;
                }

                /* ── Gameplay ───────────────────────────────────────── */
                case STATE_PLAYING:
                {
                    if (!gc.initialised)
                        GameContext_Init(&gc, renderer);

                    /* ── Pause menu open: route events to it ────────── */
                    if (pm.visible) {
                        int action = PauseMenu_HandleEvent(&pm, &ev);
                        if (action == PAUSE_ITEM_RESUME) {
                            /* Start 3-2-1 countdown */
                            pm.visible        = false;
                            pm.countdown      = COUNTDOWN_ACTIVE;
                            pm.countdownValue = 3;
                            pm.countdownStart = SDL_GetTicks();
                            /* paused stays true until countdown finishes */
                        } else if (action == PAUSE_ITEM_SAVE) {
                            SaveSystem_Save(
                                gc.players, gc.numPlayers,
                                gc.enemies, NUM_ENEMIES,
                                totalElapsed(&gc),
                                SAVE_FILE, PATH_SCORE_FILE);
                            /* Reload high-score board silently */
                            HighScore_SetScore(&hs, 0);
                        } else if (action == PAUSE_ITEM_EXIT) {
                            pm.visible   = false;
                            pm.countdown = COUNTDOWN_NONE;
                            paused       = 0;
                            GameContext_Free(&gc);
                            state = STATE_MAIN_MENU;
                        }
                        break;   /* don't process gameplay keys while paused */
                    }

                    /* ── Normal (unpaused) key handling ─────────────── */
                    if (!paused && ev.type == SDL_KEYDOWN)
                    {
                        SDL_Keycode k = ev.key.keysym.sym;

                        /* H → open pause menu */
                        if (k == SDLK_h) {
                            PauseMenu_Open(&pm);
                            paused = 1;
                            break;
                        }

                        if (k == SDLK_ESCAPE) {
                            GameContext_Free(&gc);
                            state = STATE_MAIN_MENU;
                            break;
                        }

                        /* Add P2 */
                        if (k == SDLK_p && gc.numPlayers < 2) {
                            initializePlayer(&gc.players[1], 2, renderer);
                            gc.players[1].scoreActive = 1;
                            gc.numPlayers = 2;
                            setHeroBounds(&gc.players[0], gc.worldW, gc.worldH);
                            setHeroBounds(&gc.players[1], gc.worldW, gc.worldH);
                            gc.players[1].posHero.y = gc.groundY;
                        }

                        /* Solo player actions – keybinds depend on chosen skin */
                        if (!gc.players[0].dead) {
                            if (gc.chosenPlayerId == 2) {
                                /* Warrior (P2 skin): Shoot=L, Dash=C, Jump=Space */
                                if (k == SDLK_l) {
                                    shoot(&gc.players[0]);
                                } else if (k == SDLK_c &&
                                           gc.players[0].state != STATE_DASH &&
                                           gc.players[0].state != STATE_DEAD) {
                                    setState_public(&gc.players[0], STATE_DASH);
                                    gc.players[0].dashTime = 10;
                                } else if (k == SDLK_SPACE &&
                                           gc.players[0].state != STATE_JUMP &&
                                           gc.players[0].state != STATE_DEAD) {
                                    setState_public(&gc.players[0], STATE_JUMP);
                                    gc.players[0].jumpTime = 30;
                                    gc.players[0].jumpVy   = -10.0f;
                                }
                            } else {
                                /* Soldier (P1 skin): Shoot=K, Dash=M, Jump=N */
                                if (k == SDLK_k) {
                                    shoot(&gc.players[0]);
                                } else if (k == SDLK_m &&
                                           gc.players[0].state != STATE_DASH &&
                                           gc.players[0].state != STATE_DEAD) {
                                    setState_public(&gc.players[0], STATE_DASH);
                                    gc.players[0].dashTime = 10;
                                } else if (k == SDLK_n &&
                                           gc.players[0].state != STATE_JUMP &&
                                           gc.players[0].state != STATE_DEAD) {
                                    setState_public(&gc.players[0], STATE_JUMP);
                                    gc.players[0].jumpTime = 30;
                                    gc.players[0].jumpVy   = -10.0f;
                                }
                            }
                        }
                    }
                    break;
                }

                default: break;
            }
        }

        if (!running) break;

        /* ══════════════════════════════════════════════════════════════
         * U P D A T E
         * ══════════════════════════════════════════════════════════════ */
        switch (state)
        {
            case STATE_MAIN_MENU:
                MainMenu_Update(&mm);
                break;

            case STATE_OPTIONS:
                OptionsMenu_Update(&om);
                break;

            case STATE_CHAR_SELECT:
                CharSelect_Update(&cs);
                break;

            case STATE_SAVE_LOAD:
                /* no per-frame update needed */
                break;

            case STATE_HIGHSCORE:
                /* no per-frame update needed */
                break;

            case STATE_PLAYING:
            {
                if (!gc.initialised) break;

                /* ── Advance countdown (runs even while paused=1) ──── */
                if (pm.countdown == COUNTDOWN_ACTIVE) {
                    if (PauseMenu_Update(&pm)) {
                        /* countdown finished */
                        paused = 0;
                        gc.sessionStart = SDL_GetTicks();  /* restart timer */
                    }
                    break;   /* skip gameplay update during countdown */
                }

                /* ── Skip gameplay update while pause menu is open ─── */
                if (paused) break;

                /* ── Normal gameplay update ─────────────────────────── */
                const Uint8 *keys = SDL_GetKeyboardState(NULL);

                if (!gc.players[0].dead) {
                    if (gc.chosenPlayerId == 2) {
                        /* Warrior (Player 2 skin) – ZQSD */
                        gc.players[0].moveLeft  = keys[SDL_SCANCODE_Q];
                        gc.players[0].moveRight = keys[SDL_SCANCODE_D];
                        gc.players[0].moveUp    = keys[SDL_SCANCODE_Z];
                        gc.players[0].moveDown  = keys[SDL_SCANCODE_S];
                    } else {
                        /* Soldier (Player 1 skin) – Arrow keys */
                        gc.players[0].moveLeft  = keys[SDL_SCANCODE_LEFT];
                        gc.players[0].moveRight = keys[SDL_SCANCODE_RIGHT];
                        gc.players[0].moveUp    = keys[SDL_SCANCODE_UP];
                        gc.players[0].moveDown  = keys[SDL_SCANCODE_DOWN];
                    }
                } else {
                    gc.players[0].moveLeft = gc.players[0].moveRight =
                    gc.players[0].moveUp   = gc.players[0].moveDown  = 0;
                }

                if (gc.numPlayers > 1) {
                    if (!gc.players[1].dead) {
                        gc.players[1].moveLeft  = keys[SDL_SCANCODE_A];
                        gc.players[1].moveRight = keys[SDL_SCANCODE_D];
                        gc.players[1].moveUp    = keys[SDL_SCANCODE_W];
                        gc.players[1].moveDown  = keys[SDL_SCANCODE_S];
                    } else {
                        gc.players[1].moveLeft = gc.players[1].moveRight =
                        gc.players[1].moveUp   = gc.players[1].moveDown  = 0;
                    }
                }

                for (int i = 0; i < gc.numPlayers; i++) {
                    moveHero(&gc.players[i]);
                    updateBullets(&gc.players[i]);
                    updateScore(&gc.players[i]);
                }

                for (int i = 0; i < NUM_ENEMIES; i++) {
                    if (!gc.enemies[i].active) continue;
                    int tx, ty;
                    nearestHeroPos(gc.players, gc.numPlayers,
                                   gc.enemies[i].x, gc.enemies[i].y,
                                   &tx, &ty);
                    moveEnemy(&gc.enemies[i], tx, ty, gc.worldW);
                }

                /* ── Wave spawn: if active enemies <= 2, spawn 4 more ── */
                {
                    int activeCount = 0;
                    for (int i = 0; i < NUM_ENEMIES; i++)
                        if (gc.enemies[i].active && gc.enemies[i].health > 0)
                            activeCount++;

                    if (activeCount <= 2) {
                        int spawned = 0;
                        for (int i = 0; i < NUM_ENEMIES && spawned < 4; i++) {
                            if (!gc.enemies[i].active || gc.enemies[i].health <= 0) {
                                /* Spread spawn points evenly, avoid player pos */
                                int spawnX = (gc.worldW / 5) * (spawned + 1);
                                /* Alternate sides to keep it interesting */
                                if (spawned % 2 == 1)
                                    spawnX = gc.worldW - spawnX;
                                initEnemy(&gc.enemies[i], spawnX,
                                          gc.groundY, renderer);
                                spawned++;
                            }
                        }
                    }
                }

                for (int p = 0; p < gc.numPlayers; p++)
                    checkBulletEnemyHit(&gc.players[p], gc.enemies, NUM_ENEMIES);

                checkEnemyHeroContact(gc.enemies, NUM_ENEMIES,
                                      gc.players, gc.numPlayers);

                updateSplitGame(&gc.split, gc.players, gc.numPlayers);

                /* ── Minimap dots ──────────────────────────────────── */
                minimapClear(&gc.minimap);
                SDL_Color p1col = {  80, 180, 255, 255 };
                SDL_Color p2col = { 255, 100,  50, 255 };
                SDL_Color encol = { 255,  40,  40, 255 };
                minimapAddDot(&gc.minimap,
                              gc.players[0].posHero.x,
                              gc.players[0].posHero.y,
                              gc.worldW, SCREEN_HEIGHT, p1col);
                if (gc.numPlayers > 1)
                    minimapAddDot(&gc.minimap,
                                  gc.players[1].posHero.x,
                                  gc.players[1].posHero.y,
                                  gc.worldW, SCREEN_HEIGHT, p2col);
                for (int i = 0; i < NUM_ENEMIES && gc.minimap.numDots < 4; i++)
                    if (gc.enemies[i].active)
                        minimapAddDot(&gc.minimap,
                                      gc.enemies[i].x, gc.enemies[i].y,
                                      gc.worldW, SCREEN_HEIGHT, encol);

                /* ── Game-over → high-score transition ─────────────── */
                if (allPlayersDead(gc.players, gc.numPlayers)) {
                    int score = bestScore(gc.players, gc.numPlayers);
                    HighScore_SetScore(&hs, score);
                    state = STATE_HIGHSCORE;
                }
                break;
            }

            default: break;
        }

        /* ══════════════════════════════════════════════════════════════
         * R E N D E R
         * ══════════════════════════════════════════════════════════════ */
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        switch (state)
        {
            case STATE_MAIN_MENU:
                MainMenu_Render(&mm, renderer);

                /* "Continue" hint when a save exists */
                if (SaveSystem_Exists(SAVE_FILE) && gameFont) {
                    SDL_Color cyan = {0, 220, 220, 255};
                    SDL_Surface *cs = TTF_RenderText_Solid(
                        gameFont, "Press C to Continue saved game", cyan);
                    if (cs) {
                        SDL_Texture *ct = SDL_CreateTextureFromSurface(renderer, cs);
                        SDL_Rect cr = { SCREEN_WIDTH / 2 - cs->w / 2,
                                        SCREEN_HEIGHT - cs->h - 8,
                                        cs->w, cs->h };
                        SDL_RenderCopy(renderer, ct, NULL, &cr);
                        SDL_FreeSurface(cs);
                        SDL_DestroyTexture(ct);
                    }
                }
                break;

            case STATE_OPTIONS:
                OptionsMenu_Render(&om, renderer);
                break;

            case STATE_CHAR_SELECT:
                CharSelect_Render(&cs, renderer);
                break;

            case STATE_SAVE_LOAD:
                SaveMenu_Render(&sm, renderer);
                break;

            case STATE_HIGHSCORE:
                HighScore_Render(&hs, renderer);
                break;

            case STATE_PLAYING:
            {
                if (!gc.initialised) break;

                /* ── Draw game world ──────────────────────────────── */
                renderSplitGame(&gc.split, renderer, gc.players,
                                gc.numPlayers, gameFont);

                if (gc.numPlayers == 1) {
                    drawEnemies(gc.enemies, NUM_ENEMIES, renderer,
                                gc.split.camera[0].x,
                                gc.split.camera[0].y);
                } else {
                    for (int p = 0; p < 2; p++) {
                        SDL_Rect vp = { p * SPLIT_VIEW_W, 0,
                                        SPLIT_VIEW_W, SPLIT_VIEW_H };
                        SDL_RenderSetViewport(renderer, &vp);
                        drawEnemies(gc.enemies, NUM_ENEMIES, renderer,
                                    gc.split.camera[p].x,
                                    gc.split.camera[p].y);
                    }
                    SDL_RenderSetViewport(renderer, NULL);
                }

                renderMinimap(&gc.minimap, renderer);

                /* Timer (shows real elapsed seconds, pauses excluded) */
                if (gameFont) {
                    Uint32 secs = totalElapsed(&gc);
                    char txt[32];
                    sprintf(txt, "%u s", secs);
                    SDL_Color white = {255, 255, 255, 255};
                    SDL_Surface *ts = TTF_RenderText_Solid(gameFont, txt, white);
                    if (ts) {
                        SDL_Texture *tt = SDL_CreateTextureFromSurface(renderer, ts);
                        SDL_Rect tp = { SCREEN_WIDTH / 2 - ts->w / 2, 6,
                                        ts->w, ts->h };
                        SDL_RenderCopy(renderer, tt, NULL, &tp);
                        SDL_FreeSurface(ts);
                        SDL_DestroyTexture(tt);
                    }
                }

                /* Hint overlays */
                if (gc.numPlayers == 1 && gameFont) {
                    SDL_Color yellow = {255, 230, 0, 200};
                    SDL_Surface *ph = TTF_RenderText_Solid(
                        gameFont, "P = add Player 2  |  H = Pause", yellow);
                    if (ph) {
                        SDL_Texture *ht = SDL_CreateTextureFromSurface(renderer, ph);
                        SDL_Rect hp = { SCREEN_WIDTH / 2 - ph->w / 2,
                                        SCREEN_HEIGHT - ph->h - 8,
                                        ph->w, ph->h };
                        SDL_RenderCopy(renderer, ht, NULL, &hp);
                        SDL_FreeSurface(ph);
                        SDL_DestroyTexture(ht);
                    }
                }

                if (gameFont) {
                    SDL_Color grey = {200, 200, 200, 160};
                    SDL_Surface *es = TTF_RenderText_Solid(
                        gameFont, "ESC = Main Menu", grey);
                    if (es) {
                        SDL_Texture *et = SDL_CreateTextureFromSurface(renderer, es);
                        SDL_Rect ep = { SCREEN_WIDTH - es->w - 8,
                                        SCREEN_HEIGHT - es->h - 8,
                                        es->w, es->h };
                        SDL_RenderCopy(renderer, et, NULL, &ep);
                        SDL_FreeSurface(es);
                        SDL_DestroyTexture(et);
                    }
                }

                /* ── Draw pause overlay ON TOP of everything ─────── */
                PauseMenu_Render(&pm, renderer);

                break;
            }

            default: break;
        }

        SDL_RenderPresent(renderer);

        int elapsed = (int)(SDL_GetTicks() - frameStart);
        if (frameDelay > elapsed)
            SDL_Delay(frameDelay - elapsed);

    } /* main loop */

    /* ── Cleanup ──────────────────────────────────────────────────────── */
    GameContext_Free(&gc);

    MainMenu_Free(&mm);
    OptionsMenu_Free(&om);
    SaveMenu_Free(&sm);
    HighScore_Free(&hs);
    CharSelect_Free(&cs);

    if (gameFont)    TTF_CloseFont(gameFont);
    if (gameFontBig) TTF_CloseFont(gameFontBig);

    Mix_HaltMusic();
    Mix_CloseAudio();
    Mix_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    return 0;
}
