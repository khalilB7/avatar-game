#include "enemy.h"
#include <stdlib.h>
#include <time.h>

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window *win = SDL_CreateWindow("Mummy Enemy",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, 0);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    srand(time(NULL));

    Enemy mummy = {300, 408, 72, 72, 0, 8, 0, 3, 0, 0};
    Player player = {100, 408, 64, 64, 3};
    Item bonus = {400, 448, 32, 32};

    SDL_Surface *surf;

    // Load movement frames
    surf = IMG_Load("left.png");  mummy.framesRight[0] = SDL_CreateTextureFromSurface(ren, surf); SDL_FreeSurface(surf);
    surf = IMG_Load("right.png"); mummy.framesRight[1] = SDL_CreateTextureFromSurface(ren, surf); SDL_FreeSurface(surf);
    surf = IMG_Load("up.png");    mummy.framesRight[2] = SDL_CreateTextureFromSurface(ren, surf); SDL_FreeSurface(surf);

    // Attack frame
    surf = IMG_Load("down.png");  mummy.attack = SDL_CreateTextureFromSurface(ren, surf); SDL_FreeSurface(surf);

    // Death animation
    surf = IMG_Load("dead.png"); mummy.deathFrames[0] = SDL_CreateTextureFromSurface(ren, surf); SDL_FreeSurface(surf);
    surf = IMG_Load("dead.png"); mummy.deathFrames[1] = SDL_CreateTextureFromSurface(ren, surf); SDL_FreeSurface(surf);
    surf = IMG_Load("dead.png"); mummy.deathFrames[2] = SDL_CreateTextureFromSurface(ren, surf); SDL_FreeSurface(surf);

    int running = 1; SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) if (e.type == SDL_QUIT) running = 0;

        updateEnemy(&mummy, &player);

        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        SDL_Rect playerRect = {player.x, player.y, player.w, player.h};
        SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
        SDL_RenderFillRect(ren, &playerRect);

        // Player health bar
        SDL_Rect playerHealthBar = {20, 20, player.health * 40, 10};
        SDL_SetRenderDrawColor(ren, 0, 255, 0, 255);
        SDL_RenderFillRect(ren, &playerHealthBar);

        // Item
        SDL_Rect itemRect = {bonus.x, bonus.y, bonus.w, bonus.h};
        SDL_SetRenderDrawColor(ren, 0, 255, 0, 255);
        SDL_RenderFillRect(ren, &itemRect);

        SDL_Rect enemyRect = {mummy.x, mummy.y, mummy.w, mummy.h};

        // Item pickup
        if (checkCollision(playerRect, itemRect)) {
            player.health++;
            bonus.x = -100;
        }

        // Enemy rendering
        if (mummy.isDead) {
            SDL_RenderCopyEx(ren, mummy.deathFrames[mummy.deathFrame], NULL, &enemyRect,
                             0, NULL, SDL_FLIP_NONE);
        } else if (checkCollision(playerRect, enemyRect)) {
            SDL_RenderCopyEx(ren, mummy.attack, NULL, &enemyRect,
                             0, NULL, (mummy.dx < 0) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
            player.health--;
            if (player.health <= 0) running = 0;
        } else {
            SDL_RenderCopyEx(ren, mummy.framesRight[mummy.frame], NULL, &enemyRect,
                             0, NULL, (mummy.dx < 0) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
        }

        // Enemy health bar
        SDL_Rect enemyHealthBar = {mummy.x, mummy.y - 10, mummy.health * 20, 5};
        SDL_SetRenderDrawColor(ren, 0, 0, 255, 255);
        SDL_RenderFillRect(ren, &enemyHealthBar);

        SDL_RenderPresent(ren);
        SDL_Delay(100);
    }

    // Cleanup textures
    for (int i = 0; i < FRAME_COUNT; i++) {
        SDL_DestroyTexture(mummy.framesRight[i]);
        SDL_DestroyTexture(mummy.deathFrames[i]);
    }
    SDL_DestroyTexture(mummy.attack);

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
