#ifndef ENNEMIE_H
#define ENNEMIE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>

// Struct for enemy
typedef struct {
    SDL_Texture *sprite;
    SDL_Texture *sprite2;
    SDL_Texture *sprite3;
    SDL_Texture *sprite4;

    SDL_Rect pos;       // Position on screen
    SDL_Rect possprite; // Sprite frame

    int posmax;
    int posmin;

    int nbretat;
    int nbrpic;

    int longueur;
    int largeur;

    int direction; // 0 = right, 1 = left
    int health;

    int saut;
    int agauche;
    int vitesse;

    int ysprite;
    int modif;

    int auSol;
    int entraindetomber;
    int gravity;
    int force_saut;
    int level;

    int posYinitiale;
    float velociteX;
    float velociteY;

} ennemie;

// Function declarations
void initialiserennemie(ennemie *e, SDL_Renderer *renderer);
void sauterVersPosition(ennemie *en, int cibleX);
void appliquerGravite(ennemie *en);
void deplacerEnnemi(ennemie *e);
void moveAI(SDL_Rect perso, ennemie *en);
void animer(ennemie *e);
int collisisonBB(ennemie e, SDL_Rect pos);
void affichere(ennemie e, SDL_Renderer *renderer);

#endif
