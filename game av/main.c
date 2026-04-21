#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include "hero.h"

#define MAX_PLAYERS 2

int main(void) {

    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    IMG_Init(IMG_INIT_PNG);

    SDL_Window *window = SDL_CreateWindow(
        "Hero Game",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, 0);

    SDL_Surface *screen = SDL_GetWindowSurface(window);

    TTF_Font *font = TTF_OpenFont(
        "/usr/share/fonts/truetype/malayalam/Rachana-Bold.ttf", 24);

    Hero players[MAX_PLAYERS];
    int  numPlayers = 1;

    initializePlayer(&players[0], 1);

    SDL_Event e;
    int running = 1;

    while (running) {

        // ---- Events --------------------------------------------------------
        while (SDL_PollEvent(&e)) {

            if (e.type == SDL_QUIT)
                running = 0;

            if (e.type == SDL_KEYDOWN) {

                switch (e.key.keysym.sym) {

                    // Add player 2
                    case SDLK_p:
                        if (numPlayers < 2) {
                            initializePlayer(&players[1], 2);
                            numPlayers = 2;
                        }
                        break;

                    // Player 1 actions
                    case SDLK_k:
                        shoot(&players[0]);
                        break;
                    case SDLK_m:
                        if (!players[0].dead) {
                            players[0].state     = STATE_DASH;
                            players[0].maxFrames = FRAMES_DASH;
                            players[0].dashTime  = 10;
                        }
                        break;
                    case SDLK_n:
                        if (!players[0].dead) {
                            players[0].state     = STATE_JUMP;
                            players[0].maxFrames = FRAMES_JUMP;
                            players[0].jumpTime  = 20;
                        }
                        break;

                    // Player 2 actions
                    case SDLK_l:
                        if (numPlayers > 1) shoot(&players[1]);
                        break;
                    case SDLK_c:
                        if (numPlayers > 1 && !players[1].dead) {
                            players[1].state     = STATE_DASH;
                            players[1].maxFrames = FRAMES_DASH;
                            players[1].dashTime  = 10;
                        }
                        break;
                    case SDLK_SPACE:
                        if (numPlayers > 1 && !players[1].dead) {
                            players[1].state     = STATE_JUMP;
                            players[1].maxFrames = FRAMES_JUMP;
                            players[1].jumpTime  = 20;
                        }
                        break;

                    default: break;
                }
            }
        }

        // ---- Keyboard state (held keys) ------------------------------------
        const Uint8 *keys = SDL_GetKeyboardState(NULL);

        // Player 1 — arrow keys; dead players ignore input
        if (!players[0].dead) {
            players[0].moveLeft  = keys[SDL_SCANCODE_LEFT];
            players[0].moveRight = keys[SDL_SCANCODE_RIGHT];
            players[0].moveUp    = keys[SDL_SCANCODE_UP];
            players[0].moveDown  = keys[SDL_SCANCODE_DOWN];
        } else {
            players[0].moveLeft = players[0].moveRight =
            players[0].moveUp   = players[0].moveDown  = 0;
        }

        // Player 2 — WASD; dead players ignore input
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

        // ---- Update --------------------------------------------------------
        for (int i = 0; i < numPlayers; i++) {
            moveHero(&players[i]);
            updateBullets(&players[i]);
            updateScore(&players[i]);
        }

        // NOTE: no friendly fire — players do NOT damage each other.
        // checkBulletHit is reserved for player vs enemy (added by colleague).

        // ---- Draw ----------------------------------------------------------
        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

        for (int i = 0; i < numPlayers; i++) {
            showPlayer(screen, &players[i]);
            drawBullets(&players[i], screen);
            drawHearts(&players[i], screen, font);
            drawScore(&players[i], screen, font);
        }

        // Game-over overlay
        int allDead = 1;
        for (int i = 0; i < numPlayers; i++)
            if (!players[i].dead) { allDead = 0; break; }

        if ((numPlayers == 1 && players[0].dead) ||
            (numPlayers == 2 && allDead)) {

            SDL_Color red = {255, 0, 0, 255};
            SDL_Surface *msg = TTF_RenderText_Solid(font, "GAME OVER", red);
            SDL_Rect pos = {(SCREEN_WIDTH - msg->w) / 2,
                            (SCREEN_HEIGHT - msg->h) / 2, 0, 0};
            SDL_BlitSurface(msg, NULL, screen, &pos);
            SDL_FreeSurface(msg);
        }

        SDL_UpdateWindowSurface(window);
        SDL_Delay(16);   // ~60 fps
    }

    TTF_CloseFont(font);
    IMG_Quit();
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
