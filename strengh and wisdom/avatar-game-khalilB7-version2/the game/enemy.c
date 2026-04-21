#include "enemy.h"
#include "minimap.h"   /* checkCollision */
#include <stdlib.h>
#include <stdio.h>



void initEnemy(Enemy *e, int x, int y, SDL_Renderer *renderer)
{
    e->x         = x;
    e->y         = y;
    e->w         = 72;
    e->h         = 72;
    e->dx        = 2;
    e->speed     = 2;
    e->frame     = 0;
    e->health    = 3;
    e->active    = 1;
    e->moveTimer = 0;
    e->animTimer = 0;

   
    SDL_Surface *surf;

    surf = IMG_Load("/home/mohamed/strengh and wisdom/avatar-game-khalilB7-version2/the game/images/stop.png");
    e->framesRight[0] = surf ? SDL_CreateTextureFromSurface(renderer, surf) : NULL;
    if (surf) SDL_FreeSurface(surf);

    surf = IMG_Load("/home/mohamed/strengh and wisdom/avatar-game-khalilB7-version2/the game/images/left.png");
    e->framesRight[1] = surf ? SDL_CreateTextureFromSurface(renderer, surf) : NULL;
    if (surf) SDL_FreeSurface(surf);

    surf = IMG_Load("/home/mohamed/strengh and wisdom/avatar-game-khalilB7-version2/the game/images/right.png");
    e->framesRight[2] = surf ? SDL_CreateTextureFromSurface(renderer, surf) : NULL;
    if (surf) SDL_FreeSurface(surf);

    surf = IMG_Load("/home/mohamed/strengh and wisdom/avatar-game-khalilB7-version2/the game/images/attack.png");
    e->attackTex = surf ? SDL_CreateTextureFromSurface(renderer, surf) : NULL;
    if (surf) SDL_FreeSurface(surf);
}

void freeEnemy(Enemy *e)
{
    for (int i = 0; i < ENEMY_FRAME_COUNT; i++) {
        if (e->framesRight[i]) {
            SDL_DestroyTexture(e->framesRight[i]);
            e->framesRight[i] = NULL;
        }
    }
    if (e->attackTex) { SDL_DestroyTexture(e->attackTex); e->attackTex = NULL; }
    e->active = 0;
}


void moveEnemy(Enemy *e, int targetX, int targetY, int worldW)
{
    if (!e->active || e->health <= 0) { e->active = 0; return; }

   
    e->moveTimer++;
    if (e->moveTimer >= 3) {
        e->moveTimer = 0;

        int dx = targetX - e->x;
        int dy = targetY - e->y;

        if (abs(dx) < ENEMY_DETECTION && abs(dy) < ENEMY_DETECTION) {
            
            e->dx = (dx < 0) ? -e->speed : e->speed;
        } else {
           
            if (rand() % 100 < 5) e->dx = -e->dx;
        }

        e->x += e->dx;

        
        if (e->x < 0)              { e->x = 0;             e->dx =  e->speed; }
        if (e->x + e->w > worldW)  { e->x = worldW - e->w; e->dx = -e->speed; }
    }

    
    e->animTimer++;
    if (e->animTimer >= 6) {
        e->frame     = (e->frame + 1) % ENEMY_FRAME_COUNT;
        e->animTimer = 0;
    }
}



void drawEnemy(Enemy *e, SDL_Renderer *renderer, int camX, int camY)
{
    if (!e->active || e->health <= 0) return;

    SDL_Rect dest = { e->x - camX, e->y - camY, e->w, e->h };

    SDL_Texture *tex = e->framesRight[e->frame];
    if (tex) {
        SDL_RenderCopy(renderer, tex, NULL, &dest);
    } else {
        
        SDL_SetRenderDrawColor(renderer, 160, 32, 240, 255);
        SDL_RenderFillRect(renderer, &dest);
    }

    
    SDL_Rect hbBg = { dest.x,           dest.y - 8, e->w,              6 };
    SDL_Rect hbFg = { dest.x,           dest.y - 8, e->health * 20,    6 };
    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 200);
    SDL_RenderFillRect(renderer, &hbBg);
    SDL_SetRenderDrawColor(renderer, 0, 210, 60, 255);
    SDL_RenderFillRect(renderer, &hbFg);
}
