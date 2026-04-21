#include "enemy.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

int main() {

    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window *win = SDL_CreateWindow(
        "strength and wisdom",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640, 480, 0
    );

    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    srand(time(NULL));

    Enemy enemy = {
        300, 200, 72, 72,
        0, 0, 4, 0,
        10,
        0, 0, 0, 0,
        0,
        0
    };

    Player player = {100, 100, 64, 64, 5};
    Item bonus = {400, 300, 32, 32};

    SDL_Surface *surf;

    // =========================
    // LOAD ENEMY TEXTURES
    // =========================

    surf = IMG_Load("left.png");
    if (!surf) { printf("Error: %s\n", IMG_GetError()); return 1; }
    enemy.frames[0] = SDL_CreateTextureFromSurface(ren, surf);
    SDL_FreeSurface(surf);

    surf = IMG_Load("right.png");
    if (!surf) { printf("Error: %s\n", IMG_GetError()); return 1; }
    enemy.frames[1] = SDL_CreateTextureFromSurface(ren, surf);
    SDL_FreeSurface(surf);

    surf = IMG_Load("up.png");
    if (!surf) { printf("Error: %s\n", IMG_GetError()); return 1; }
    enemy.frames[2] = SDL_CreateTextureFromSurface(ren, surf);
    SDL_FreeSurface(surf);

    // NORMAL ATTACK (SAFE - no damage)
    surf = IMG_Load("attack.png");
    if (!surf) { printf("Error: %s\n", IMG_GetError()); return 1; }
    enemy.attack = SDL_CreateTextureFromSurface(ren, surf);
    SDL_FreeSurface(surf);

    // SPECIAL ATTACK (DAMAGING)
    surf = IMG_Load("attack.png");
    if (!surf) { printf("Error: %s\n", IMG_GetError()); return 1; }
    enemy.attack2 = SDL_CreateTextureFromSurface(ren, surf);
    SDL_FreeSurface(surf);

    for (int i = 0; i < FRAME_COUNT; i++) {
        surf = IMG_Load("dead.png");
        if (!surf) { printf("Error: %s\n", IMG_GetError()); return 1; }
        enemy.deathFrames[i] = SDL_CreateTextureFromSurface(ren, surf);
        SDL_FreeSurface(surf);
    }

    int running = 1;
    SDL_Event e;

    // =========================
    // GAME LOOP
    // =========================
    while (running) {

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = 0;
        }

        updateEnemy(&enemy, &player);

        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        // =========================
        // PLAYER
        // =========================
        SDL_Rect playerRect = {player.x, player.y, player.w, player.h};
        SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
        SDL_RenderFillRect(ren, &playerRect);

        // =========================
        // ITEM
        // =========================
        SDL_Rect itemRect = {bonus.x, bonus.y, bonus.w, bonus.h};
        SDL_SetRenderDrawColor(ren, 0, 255, 0, 255);
        SDL_RenderFillRect(ren, &itemRect);

        if (checkCollision(playerRect, itemRect)) {
            if (player.health < 5)
                player.health++;
            bonus.x = -100;
        }

        // =========================
        // ENEMY
        // =========================
        SDL_Rect enemyRect = {enemy.x, enemy.y, enemy.w, enemy.h};

        if (enemy.isDead) {

            SDL_RenderCopy(ren, enemy.deathFrames[enemy.deathFrame], NULL, &enemyRect);

        }
        else if (enemy.isAttacking) {

            // SAFE OR DAMAGING ATTACK
            if (enemy.attackType == 1)
                SDL_RenderCopy(ren, enemy.attack2, NULL, &enemyRect); // DAMAGE
            else
                SDL_RenderCopy(ren, enemy.attack, NULL, &enemyRect);  // SAFE

            enemyTryAttackHit(&enemy, &player, enemyRect, playerRect);
        }
        else {

            SDL_RenderCopy(ren, enemy.frames[enemy.frame], NULL, &enemyRect);
        }

        // =========================
        // PLAYER HP
        // =========================
        SDL_Rect playerHP = {20, 20, player.health * 40, 10};
        SDL_SetRenderDrawColor(ren, 0, 255, 0, 255);
        SDL_RenderFillRect(ren, &playerHP);

        // =========================
        // ENEMY HP (SMALL BAR)
        // =========================
        int maxHP = 10;
        int barMax = 40;

        int bar = (enemy.health * barMax) / maxHP;
        if (bar < 0) bar = 0;

        SDL_Rect bg = {enemy.x, enemy.y - 8, barMax, 3};
        SDL_Rect hp = {enemy.x, enemy.y - 8, bar, 3};

        SDL_SetRenderDrawColor(ren, 50, 50, 50, 255);
        SDL_RenderFillRect(ren, &bg);

        SDL_SetRenderDrawColor(ren, 0, 0, 255, 255);
        SDL_RenderFillRect(ren, &hp);

        SDL_RenderPresent(ren);
        SDL_Delay(100);

        if (player.health <= 0)
            running = 0;
    }

    // =========================
    // CLEANUP
    // =========================
    for (int i = 0; i < FRAME_COUNT; i++) {
        SDL_DestroyTexture(enemy.frames[i]);
        SDL_DestroyTexture(enemy.deathFrames[i]);
    }

    SDL_DestroyTexture(enemy.attack);
    SDL_DestroyTexture(enemy.attack2);

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);

    IMG_Quit();
    SDL_Quit();

    return 0;
}
