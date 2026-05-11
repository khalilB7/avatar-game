#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include "menu.h"

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) { printf("SDL_Init Error: %s\n", SDL_GetError()); return 1; }
    if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) == 0) { printf("IMG_Init Error: %s\n", IMG_GetError()); return 1; }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) { printf("Mix_OpenAudio Error: %s\n", Mix_GetError()); return 1; }
    if (TTF_Init() != 0) { printf("TTF_Init Error: %s\n", TTF_GetError()); return 1; }

    SDL_Window* window = SDL_CreateWindow("Main Menu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Menu menu;
    if (!Menu_Init(&menu, renderer)) return 1;

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            Menu_HandleEvent(&menu, &e, &quit);
        }
        Menu_Render(&menu, renderer);
        SDL_Delay(16); // ~60 FPS
    }

    Menu_Cleanup(&menu);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    IMG_Quit();
    SDL_Quit();
    return 0;
}
