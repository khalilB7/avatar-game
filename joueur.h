#ifndef JOUEUR_H
#define JOUEUR_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>  


typedef struct
{
    SDL_Texture *sprite;   
    SDL_Rect pos;          
    int score;            
    int hearts;            
    SDL_RendererFlip flip; 
    Uint32 lastScoreTime;  
} Joueur;


void initJoueur(Joueur *j, SDL_Renderer *renderer);
void deplacerJoueur(Joueur *j, SDL_Event event);
void afficherJoueur(Joueur j, SDL_Renderer *renderer);
void displayHearts(Joueur j, SDL_Renderer *renderer);
void updateAnimation(Joueur *j);
void updateScore(Joueur *j, Uint32 currentTime);
void renderScore(Joueur j, SDL_Renderer *renderer, TTF_Font *font);

#endif
