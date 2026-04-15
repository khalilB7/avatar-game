#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include "hero.h"

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Hero Game",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, SDL_WINDOW_SHOWN);

    SDL_Surface* screen = SDL_GetWindowSurface(window);

    Hero hero;
    initializePlayer(&hero);

    SDL_Event event;
    int running = 1;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = 0;

            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        hero.dir = DIR_LEFT;
                        hero.state = STATE_WALK;
                        break;

                    case SDLK_RIGHT:
                        hero.dir = DIR_RIGHT;
                        hero.state = STATE_WALK;
                        break;

                    case SDLK_UP:
                        hero.dir = DIR_UP;
                        hero.state = STATE_WALK;
                        break;

                    case SDLK_DOWN:
                        hero.dir = DIR_DOWN;
                        hero.state = STATE_WALK;
                        break;

                    case SDLK_c:
                        hero.state = STATE_DASH;
                        hero.dashTime = 10;
                        break;

                    case SDLK_SPACE:
                        hero.state = STATE_JUMP;
                        hero.jumpTime = 10;
                        break;

                    case SDLK_h: // 🔥 TEST DAMAGE
                        takeDamage(&hero, 34);
                        break;
                }
            }

            if (event.type == SDL_KEYUP) {
                if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_DOWN) {
                    hero.state = STATE_IDLE;
                }
            }
        }

        moveHero(&hero);

        // Boundary check to prevent character from going off screen
        if (hero.posHero.x < 0) hero.posHero.x = 0;
        if (hero.posHero.x > 800 - hero.frameWidth) hero.posHero.x = 800 - hero.frameWidth;
        if (hero.posHero.y < 0) hero.posHero.y = 0;
        if (hero.posHero.y > 600 - hero.frameHeight) hero.posHero.y = 600 - hero.frameHeight;

        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

        showPlayer(screen, &hero);
        drawHealth(&hero, screen);

        SDL_UpdateWindowSurface(window);

        SDL_Delay(16);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
