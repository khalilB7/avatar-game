#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include "ennemie.h"

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 700

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window* window = SDL_CreateWindow("Enemy Test",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, 0);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Initialize enemy
    ennemie e;
    initialiserennemie(&e, renderer);

    SDL_Rect player = {500, 570, 50, 50}; // simple player

    int running = 1;
    SDL_Event event;

    while(running)
    {
        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_QUIT) running = 0;
        }

        // Move enemy AI
        moveAI(player, &e);
        deplacerEnnemi(&e);
        appliquerGravite(&e);
        animer(&e);

        // Render
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw player (simple rectangle)
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &player);

        // Draw enemy
        affichere(e, renderer);

        SDL_RenderPresent(renderer);

        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyTexture(e.sprite);
    SDL_DestroyTexture(e.sprite2);
    SDL_DestroyTexture(e.sprite3);
    SDL_DestroyTexture(e.sprite4);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
