#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>  // Include SDL_ttf for text rendering
#include <stdio.h>
#include "hero.h"

#define MAX_PLAYERS 2  // Maximum number of players

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();  // Initialize SDL_ttf

    SDL_Window* window = SDL_CreateWindow("Hero Game",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, SDL_WINDOW_SHOWN);

    SDL_Surface* screen = SDL_GetWindowSurface(window);

    // Load font for text rendering
    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/liberation2/LiberationSerif-BoldItalic.ttf", 24);  // Adjust font path and size as needed
    if (font == NULL) {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        return 1;  // Exit if font is not loaded
    }

    Hero players[MAX_PLAYERS];  // Array to hold multiple players
    int numPlayers = 1;  // Start with 1 player

    // Initialize the first player
    initializePlayer(&players[0], 1);

    SDL_Event event;
    int running = 1;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = 0;

            // Handle keypresses for movement, actions, etc.
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    // Player 1 controls (Arrow keys)
                    case SDLK_LEFT:
                        players[0].dir = DIR_LEFT;
                        players[0].state = STATE_WALK;
                        break;

                    case SDLK_RIGHT:
                        players[0].dir = DIR_RIGHT;
                        players[0].state = STATE_WALK;
                        break;

                    case SDLK_UP:
                        players[0].dir = DIR_UP;
                        players[0].state = STATE_WALK;
                        break;

                    case SDLK_DOWN:
                        players[0].dir = DIR_DOWN;
                        players[0].state = STATE_WALK;
                        break;

                    case SDLK_c:
                        players[0].state = STATE_DASH;
                        players[0].dashTime = 10;
                        break;

                    case SDLK_SPACE:
                        players[0].state = STATE_JUMP;
                        players[0].jumpTime = 10;
                        break;

                    // Player 2 controls (WASD keys)
                    case SDLK_a:  // Move player 2 left
                        players[1].dir = DIR_LEFT;
                        players[1].state = STATE_WALK;
                        break;

                    case SDLK_d:  // Move player 2 right
                        players[1].dir = DIR_RIGHT;
                        players[1].state = STATE_WALK;
                        break;

                    case SDLK_w:  // Move player 2 up
                        players[1].dir = DIR_UP;
                        players[1].state = STATE_WALK;
                        break;

                    case SDLK_s:  // Move player 2 down
                        players[1].dir = DIR_DOWN;
                        players[1].state = STATE_WALK;
                        break;

                    case SDLK_n:  // Player 2 dash
                        players[1].state = STATE_DASH;
                        players[1].dashTime = 10;
                        break;

                    case SDLK_m:  // Player 2 jump
                        players[1].state = STATE_JUMP;
                        players[1].jumpTime = 10;
                        break;

                    case SDLK_p:  // Add another character when "P" is pressed
                        if (numPlayers < MAX_PLAYERS) {
                            initializePlayer(&players[numPlayers], numPlayers + 1);  // Initialize new player
                            numPlayers++;
                        }
                        break;
                }
            }

            // Check for mouse click (clicking on the player)
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                // Check if the mouse click is within the first player's bounds
                if (mouseX >= players[0].posHero.x && mouseX <= players[0].posHero.x + players[0].frameWidth &&
                    mouseY >= players[0].posHero.y && mouseY <= players[0].posHero.y + players[0].frameHeight) {
                    takeDamage(&players[0], 1);  // Each click reduces 1 life for the first player
                }

                // Check if the mouse click is within the second player's bounds
                if (numPlayers > 1 && mouseX >= players[1].posHero.x && mouseX <= players[1].posHero.x + players[1].frameWidth &&
                    mouseY >= players[1].posHero.y && mouseY <= players[1].posHero.y + players[1].frameHeight) {
                    takeDamage(&players[1], 1);  // Each click reduces 1 life for the second player
                }
            }

            if (event.type == SDL_KEYUP) {
                if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_DOWN) {
                    players[0].state = STATE_IDLE;
                }

                // Player 2 Keyup for movement
                if (event.key.keysym.sym == SDLK_a || event.key.keysym.sym == SDLK_d || event.key.keysym.sym == SDLK_w || event.key.keysym.sym == SDLK_s) {
                    players[1].state = STATE_IDLE;
                }
            }
        }

        // Move both players
        moveHero(&players[0]);
        if (numPlayers > 1) {
            moveHero(&players[1]);
        }

        // Boundary check to prevent characters from going off screen
        if (players[0].posHero.x < 0) players[0].posHero.x = 0;
        if (players[0].posHero.x > 800 - players[0].frameWidth) players[0].posHero.x = 800 - players[0].frameWidth;
        if (players[0].posHero.y < 0) players[0].posHero.y = 0;
        if (players[0].posHero.y > 600 - players[0].frameHeight) players[0].posHero.y = 600 - players[0].frameHeight;

        if (numPlayers > 1) {
            if (players[1].posHero.x < 0) players[1].posHero.x = 0;
            if (players[1].posHero.x > 800 - players[1].frameWidth) players[1].posHero.x = 800 - players[1].frameWidth;
            if (players[1].posHero.y < 0) players[1].posHero.y = 0;
            if (players[1].posHero.y > 600 - players[1].frameHeight) players[1].posHero.y = 600 - players[1].frameHeight;
        }

        // Clear the screen
        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

        // Render both players
        showPlayer(screen, &players[0]);
        if (numPlayers > 1) {
            showPlayer(screen, &players[1]);
        }

        // Draw health for both players
        drawHealth(&players[0], screen, font, (SDL_Color){255, 255, 255});
        if (numPlayers > 1) {
            drawHealth(&players[1], screen, font, (SDL_Color){255, 255, 255});
        }

        SDL_UpdateWindowSurface(window);

        SDL_Delay(16);
    }

    SDL_DestroyWindow(window);
    TTF_Quit();  // Quit SDL_ttf
    SDL_Quit();  // Quit SDL
    return 0;
}
