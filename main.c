#include "game.h"

int main()
{
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    SDL_Window *window = SDL_CreateWindow("Game",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600, 0);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

    Game g;
    initGame(&g, renderer);

    SDL_Event e;
    int quit = 0;

    while(!quit)
    {
        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT)
                quit = 1;

            handleScrolling(&g, e);
        }

        updateGame(&g);

        SDL_RenderClear(renderer);

        renderGame(&g, renderer);
        renderTimer(&g, renderer);
        renderGuide(renderer, g.font);

        SDL_RenderPresent(renderer);
    }

    freeGame(&g);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
