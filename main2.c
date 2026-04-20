#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "enemy2.h"

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        fprintf(stderr, "IMG_Init Error: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    srand((unsigned int)time(NULL)); // seed RNG once

    SDL_Window* window = SDL_CreateWindow("Enemy Attack Example",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          800, 600,
                                          SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Texture* enemyTex = IMG_LoadTexture(renderer, "enemy.png");
    SDL_Texture* elementTex = IMG_LoadTexture(renderer, "element.png");

    if (!enemyTex || !elementTex) {
        fprintf(stderr, "Failed to load textures: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Rect player = {100, 100, 50, 50};
    int playerHealth = 100;

    Enemy enemy;
    enemy_init(&enemy, enemyTex, 400, 150); // center enemy on window

    int proj_capacity = 10;
    int proj_count = 0;
    Projectile* projectiles = malloc(proj_capacity * sizeof(Projectile));

    SDL_Event e;
    bool running = true;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
        }

        const Uint8* state = SDL_GetKeyboardState(NULL);
        if (state[SDL_SCANCODE_W]) player.y -= 5;
        if (state[SDL_SCANCODE_S]) player.y += 5;
        if (state[SDL_SCANCODE_A]) player.x -= 5;
        if (state[SDL_SCANCODE_D]) player.x += 5;

        // Clamp player inside window
        if (player.x < 0) player.x = 0;
        if (player.y < 0) player.y = 0;
        if (player.x + player.w > 800) player.x = 800 - player.w;
        if (player.y + player.h > 600) player.y = 600 - player.h;

        enemy_update(&enemy, &projectiles, &proj_count, &proj_capacity);
        projectiles_update(projectiles, &proj_count);

        // Check collisions
        if (check_collision(&player, &enemy.rect)) playerHealth -= 1;
        for (int i = 0; i < proj_count;) {
            if (check_collision(&player, &projectiles[i].rect)) {
                playerHealth -= 10;
                for (int j = i; j < proj_count - 1; j++) projectiles[j] = projectiles[j + 1];
                proj_count--;
            } else i++;
        }

        // Render
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &player);

        enemy_render(&enemy, renderer);
        projectiles_render(projectiles, proj_count, renderer, elementTex);

        // Draw health bar
        SDL_Rect healthBg = {20, 20, 200, 20};
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderFillRect(renderer, &healthBg);

        SDL_Rect healthBar = {20, 20, playerHealth * 2, 20};
        SDL_SetRenderDrawColor(renderer, playerHealth > 30 ? 0 : 255, playerHealth > 30 ? 255 : 0, 0, 255);
        SDL_RenderFillRect(renderer, &healthBar);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    free(projectiles);
    SDL_DestroyTexture(enemyTex);
    SDL_DestroyTexture(elementTex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
