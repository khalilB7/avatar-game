#ifndef INTEGRATED_H
#define INTEGRATED_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

#include "fonction_p.h"
#include "perso.h"
#include "joueur.h"
#include "enemie.h"

// Game states
typedef enum {
    STATE_MENU,
    STATE_GAME,
    STATE_MINIGAMES,
    STATE_PUZZLE,
    STATE_LEVEL2,
    STATE_HIGHSCORE,
    STATE_EXIT
} GameState;

// Function prototypes
GameState run_menu();
GameState run_game();
GameState run_minigames();
GameState run_puzzle();
GameState run_level2();
GameState run_highscore(int score);

// Shared global variables
extern int global_score;
extern SDL_Window* window;
extern SDL_Renderer* renderer;

#endif