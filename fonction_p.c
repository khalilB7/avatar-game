#include "fonction_p.h"
#include <stdio.h>
#include <stdlib.h>

int lastHoveredIndex = -1;

void init_menu(Menu *menu, SDL_Renderer *renderer) {
    SDL_Surface *temp = IMG_Load("BACK.png");
    menu->background = SDL_CreateTextureFromSurface(renderer, temp);
    SDL_FreeSurface(temp);

    char *buttonPaths[5] = {"play.png", "option.png", "score.png", "history.png", "exit.png"};
    char *buttonHoverPaths[5] = {"playhover.png", "optionhover.png", "scorehover.png", "historyhover.png", "exithover.png"};

    for (int i = 0; i < 5; i++) {
        temp = IMG_Load(buttonPaths[i]);
        menu->buttons[i] = SDL_CreateTextureFromSurface(renderer, temp);
        SDL_FreeSurface(temp);

        temp = IMG_Load(buttonHoverPaths[i]);
        menu->buttons_hover[i] = SDL_CreateTextureFromSurface(renderer, temp);
        SDL_FreeSurface(temp);
    }

   

    if (TTF_Init() == -1) exit(1);
    menu->font = TTF_OpenFont("bubble.ttf", 80);
    menu->textColor = (SDL_Color){255, 182, 193, 255}; 

    temp = IMG_Load("Fichier 3logo.png");
    menu->logo = SDL_CreateTextureFromSurface(renderer, temp);
    menu->poslogo = (SDL_Rect){SCREEN_WIDTH - 390, 20, temp->w, temp->h};
    SDL_FreeSurface(temp);
}

void afficher_menu(Menu menu, SDL_Renderer *renderer, int hoveredIndex) {
    SDL_RenderCopy(renderer, menu.background, NULL, NULL);

    SDL_Surface *titleSurf = TTF_RenderText_Solid(menu.font, "HARMONIA", menu.textColor);
    SDL_Texture *titleTex = SDL_CreateTextureFromSurface(renderer, titleSurf);
    SDL_Rect titlePos = {(SCREEN_WIDTH - titleSurf->w) / 2, 50, titleSurf->w, titleSurf->h};
    SDL_RenderCopy(renderer, titleTex, NULL, &titlePos);
    SDL_FreeSurface(titleSurf);
    SDL_DestroyTexture(titleTex);

    SDL_RenderCopy(renderer, menu.logo, NULL, &menu.poslogo);

    int buttonY = 300;
    for (int i = 0; i < 5; i++) {
        SDL_Texture *tex = (i == hoveredIndex) ? menu.buttons_hover[i] : menu.buttons[i];
        int w, h;
        SDL_QueryTexture(tex, NULL, NULL, &w, &h);
        SDL_Rect pos;

        if (i == 4) {
            pos = (SDL_Rect){80, SCREEN_HEIGHT - h - 150, w, h};
        } else {
            pos = (SDL_Rect){SCREEN_WIDTH / 2 - w / 2, buttonY, w, h};
            buttonY += 100;
        }

        SDL_RenderCopy(renderer, tex, NULL, &pos);

        if (i == hoveredIndex && i != lastHoveredIndex) {
            Mix_PlayChannel(-1, menu.hoverSound, 0);
            lastHoveredIndex = i;
        }
    }
}

void liberer_menu(Menu *menu) {
    SDL_DestroyTexture(menu->background);
    for (int i = 0; i < 5; i++) {
        SDL_DestroyTexture(menu->buttons[i]);
        SDL_DestroyTexture(menu->buttons_hover[i]);
    }
    SDL_DestroyTexture(menu->logo);
    Mix_FreeMusic(menu->bgMusic);
    Mix_FreeChunk(menu->hoverSound);
    TTF_CloseFont(menu->font);
}
