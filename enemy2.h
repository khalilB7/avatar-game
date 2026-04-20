#ifndef ENEMY2_H
#define ENEMY2_H

#include <SDL2/SDL.h>
#include <stdbool.h>

typedef struct {
    SDL_Rect rect;
    int speed;
} Projectile;

typedef struct {
    SDL_Rect rect;
    SDL_Texture* texture;
} Enemy;

void enemy_init(Enemy* enemy, SDL_Texture* texture, int x, int y);
void enemy_update(Enemy* enemy, Projectile** projectiles, int* proj_count, int* proj_capacity);
void enemy_render(Enemy* enemy, SDL_Renderer* renderer);
bool check_collision(const SDL_Rect* a, const SDL_Rect* b);
void projectiles_update(Projectile* projectiles, int* proj_count);
void projectiles_render(Projectile* projectiles, int proj_count, SDL_Renderer* renderer, SDL_Texture* texture);

#endif
