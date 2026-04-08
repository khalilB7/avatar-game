#ifndef GAME_H
#define GAME_H

#include "joueur.h"


void initGame(SDL_Window **window, SDL_Renderer **renderer, TTF_Font **font);
void gameLoop(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font);
void handleEvents(SDL_Event *event, Joueur *j);
void render(SDL_Renderer *renderer, Joueur *j, TTF_Font *font);

#endif
