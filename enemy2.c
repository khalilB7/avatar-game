#include "enemy2.h"
#include <stdlib.h>
#include <stdio.h>

void enemy_init(Enemy* enemy, SDL_Texture* texture, int x, int y) {
    enemy->rect.x = x;
    enemy->rect.y = y;
    enemy->rect.w = 300; // bigger enemy
    enemy->rect.h = 300;
    enemy->texture = texture;
}

bool check_collision(const SDL_Rect* a, const SDL_Rect* b) {
    return (a->x + a->w > b->x &&
            a->x < b->x + b->w &&
            a->y + a->h > b->y &&
            a->y < b->y + b->h);
}

void enemy_update(Enemy* enemy, Projectile** projectiles, int* proj_count, int* proj_capacity) {
    if (rand() % 100 < 2) { // 2% chance per frame
        if (*proj_count >= *proj_capacity) {
            *proj_capacity *= 2;
            Projectile* temp = realloc(*projectiles, (*proj_capacity) * sizeof(Projectile));
            if (!temp) {
                fprintf(stderr, "Memory allocation failed!\n");
                exit(1);
            }
            *projectiles = temp;
        }
        (*projectiles)[*proj_count].rect.x = enemy->rect.x;
        (*projectiles)[*proj_count].rect.y = enemy->rect.y + enemy->rect.h / 2 - 16;
        (*projectiles)[*proj_count].rect.w = 32;
        (*projectiles)[*proj_count].rect.h = 32;
        (*projectiles)[*proj_count].speed = 5;
        (*proj_count)++;
    }
}

void projectiles_update(Projectile* projectiles, int* proj_count) {
    for (int i = 0; i < *proj_count;) {
        projectiles[i].rect.x -= projectiles[i].speed;
        if (projectiles[i].rect.x + projectiles[i].rect.w < 0) {
            for (int j = i; j < *proj_count - 1; j++)
                projectiles[j] = projectiles[j + 1];
            (*proj_count)--;
        } else {
            i++;
        }
    }
}

void enemy_render(Enemy* enemy, SDL_Renderer* renderer) {
    SDL_RenderCopy(renderer, enemy->texture, NULL, &enemy->rect);
}

void projectiles_render(Projectile* projectiles, int proj_count, SDL_Renderer* renderer, SDL_Texture* texture) {
    for (int i = 0; i < proj_count; i++) {
        SDL_RenderCopy(renderer, texture, NULL, &projectiles[i].rect);
    }
}
