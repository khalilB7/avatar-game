#include "ennemie.h"
#include <stdio.h>
#include <math.h>

// Initialize enemy
void initialiserennemie(ennemie *e, SDL_Renderer *renderer)
{
    e->posmax = 700;
    e->posmin = 300;
    e->nbretat = 4;
    e->nbrpic = 5;

    SDL_Surface *surface1 = IMG_Load("move1.png");
    if (!surface1) { printf("Failed to load enemy sprite 1\n"); return; }

    e->sprite = SDL_CreateTextureFromSurface(renderer, surface1);
    e->possprite.w = surface1->w / e->nbrpic;
    e->possprite.h = surface1->h / e->nbretat;
    e->longueur = surface1->h;
    e->largeur = surface1->w;
    SDL_FreeSurface(surface1);

    SDL_Surface *surface2 = IMG_Load("move2.png");
    e->sprite2 = SDL_CreateTextureFromSurface(renderer, surface2);
    SDL_FreeSurface(surface2);

    SDL_Surface *surface3 = IMG_Load("move3.png");
    e->sprite3 = SDL_CreateTextureFromSurface(renderer, surface3);
    SDL_FreeSurface(surface3);

    SDL_Surface *surface4 = IMG_Load("move4.png");
    e->sprite4 = SDL_CreateTextureFromSurface(renderer, surface4);
    SDL_FreeSurface(surface4);

    e->pos.x = 50;
    e->pos.y = 50;
    e->pos.w = 100;
    e->pos.h = 100;
    e->possprite.x = 50;
    e->possprite.y = 50;

    e->direction = 1;
    e->health = 30;
    e->saut = 0;
    e->agauche = 1;
    e->vitesse = 0.5;
    e->ysprite = 0;
    e->modif = 3;
    e->auSol = 1;
    e->entraindetomber = 0;
    e->gravity = 2;
    e->force_saut = -20;
    e->level = 0;
    e->velociteX = 0;
    e->velociteY = 0;
    e->posYinitiale = e->pos.y;
}

// Jump towards X
void sauterVersPosition(ennemie *en, int cibleX) {
    if (en->auSol) {
        en->velociteY = en->force_saut;
        en->auSol = 0;
        en->posYinitiale = en->pos.y;

        int distance = cibleX - en->pos.x;
        en->velociteX = distance / 18.0f;
    }
}

// Gravity
void appliquerGravite(ennemie *en) {
    if (!en->auSol) {
        en->velociteY += en->gravity;
        en->pos.y += (int)en->velociteY;
        en->pos.x += (int)en->velociteX;

        if (en->pos.y >= en->posYinitiale) {
            en->pos.y = en->posYinitiale;
            en->velociteY = 0;
            en->velociteX = 0;
            en->auSol = 1;
        }

        if(en->velociteY >= 0) en->entraindetomber = 1;
    } else en->entraindetomber = 0;
}

// Move enemy
void deplacerEnnemi(ennemie *e)
{
    if(e->pos.x >= e->posmax) { e->direction = 1; e->agauche = 1; }
    if(e->pos.x <= e->posmin) { e->direction = 0; e->agauche = 0; }
    if(e->direction == 0) e->pos.x++;
    else e->pos.x--;
}

// Simple AI to follow player
void moveAI(SDL_Rect perso, ennemie *en){
    float dx = perso.x - en->pos.x;
    float dy = perso.y - en->pos.y;
    float distance = sqrt(dx * dx + dy * dy);

    if (distance < 450) {
        float nx = dx / distance;
        //en->pos.x += (int)(nx * en->vitesse);

        en->agauche = (dx < 0) ? 1 : 0;
        en->ysprite = 2;
    } else {
        en->ysprite = 0;
    }
}

// Animate enemy
void animer(ennemie *e)
{
     static int timer = 0;

    timer++;

    if(timer % 8 == 0)
    {
        e->possprite.y = e->ysprite * e->possprite.h;

        if(e->possprite.x >= (e->nbrpic - 1) * e->possprite.w)
            e->possprite.x = 0;
        else
            e->possprite.x += e->possprite.w;
    }}

// Collision bounding box
int collisisonBB(ennemie e, SDL_Rect pos)
{
    if(pos.x + pos.w < e.pos.x || pos.x > e.pos.x + e.pos.w ||
       pos.y + pos.h < e.pos.y || pos.y > e.pos.y + e.pos.h) return 0;
    return 1;
}

// Display enemy
void affichere(ennemie e, SDL_Renderer *renderer)
{
    if(e.level == 0){
        if(!e.agauche) SDL_RenderCopy(renderer, e.sprite, &e.possprite , &e.pos);
        else SDL_RenderCopy(renderer, e.sprite2, &e.possprite, &e.pos);
    } else {
        if(!e.agauche) SDL_RenderCopy(renderer, e.sprite3, &e.possprite , &e.pos);
        else SDL_RenderCopy(renderer, e.sprite4, &e.possprite, &e.pos);
    }
}
