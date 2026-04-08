#include "game.h"


void initGame(SDL_Window **window, SDL_Renderer **renderer, TTF_Font **font)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    *window = SDL_CreateWindow("Horizontal Movement Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 500, 900, SDL_WINDOW_SHOWN);
    if (*window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (*renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }

    if (TTF_Init() == -1) {
        printf("TTF_Init: %s\n", TTF_GetError());
        exit(1);
    }

    *font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf", 24);


    if (*font == NULL) {
        printf("TTF Error: %s\n", TTF_GetError());
        return ;
}


    IMG_Init(IMG_INIT_PNG);  

    
    Joueur joueur;
    initJoueur(&joueur, *renderer);
}

// Game loop
void gameLoop(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font)
{
    Joueur joueur;
    SDL_Event event;
    int quit = 0;

    initJoueur(&joueur, renderer);

    while (!quit)
    {
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) quit = 1;

        Uint32 currentTime = SDL_GetTicks();
        updateScore(&joueur, currentTime);  // Update score every 3 seconds

        handleEvents(&event, &joueur);  // Handle player input (movement only horizontally)
        render(renderer, &joueur, font);  // Render the player, hearts, and score

        SDL_RenderPresent(renderer);  // Present the rendered frame
        SDL_Delay(16);  // Maintain 60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    IMG_Quit();
    TTF_Quit();
}

// Handle player input events
void handleEvents(SDL_Event *event, Joueur *j)
{
    deplacerJoueur(j, *event);  // Handle player horizontal movement
}

// Render player, hearts, and score
void render(SDL_Renderer *renderer, Joueur *j, TTF_Font *font)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Clear screen to black
    SDL_RenderClear(renderer);  // Clear screen

    afficherJoueur(*j, renderer);  // Display player with animation
    displayHearts(*j, renderer);   // Display hearts
    renderScore(*j, renderer, font);  // Display score

    // Render other game elements if needed
}
