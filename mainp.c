#include "partage.h"

int main()
{
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    SDL_Window *window = SDL_CreateWindow("Split Screen VS",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600, 0);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

    Game1 g;
    initGame1(&g, renderer);

    SDL_Event e;
    int quit = 0;

    // Added a frame cap variable so your movement isn't tied to your PC's max framerate
    const int FPS = 60;
    const int frameDelay = 1000 / FPS;
    Uint32 frameStart;
    int frameTime;

    while(!quit)
    {
        frameStart = SDL_GetTicks();

        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT)
                quit = 1;
        }

        updateGame1(&g);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        renderGame1(&g, renderer);

        SDL_RenderPresent(renderer);

        // Frame capping math to keep the game speed consistent
        frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) {
            SDL_Delay(frameDelay - frameTime);
        }
    }

    freeGame1(&g);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
