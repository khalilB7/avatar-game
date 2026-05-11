#include "menu.h"
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper to load texture
SDL_Texture* loadTexture(const char* path, SDL_Renderer* renderer) {
    SDL_Surface* surf = IMG_Load(path);
    if (!surf) {
        printf("IMG_Load Error: %s\n", IMG_GetError());
        return NULL;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return tex;
}

bool Menu_Init(Menu* menu, SDL_Renderer* renderer) {
    int screenWidth = 800;
    int screenHeight = 600;

    // Load assets
    menu->bgTexture = loadTexture("avatar.jpg", renderer);
    menu->buttonTexture = loadTexture("button.png", renderer);
    menu->cornerTexture = loadTexture("cc.jpg", renderer);
    menu->bgMusic = Mix_LoadMUS("ost.mp3");
    menu->hoverSound = Mix_LoadWAV("aa.wav");
    menu->font = TTF_OpenFont("Arial.ttf", 24);

    if (!menu->bgTexture) { printf("Failed to load avatar.jpg\n"); return false; }
    if (!menu->buttonTexture) { printf("Failed to load button.png\n"); return false; }
    if (!menu->cornerTexture) { printf("Failed to load cc.jpg\n"); return false; }
    if (!menu->bgMusic) { printf("Failed to load ost.wav: %s\n", Mix_GetError()); return false; }
    if (!menu->hoverSound) { printf("Failed to load aa.wav: %s\n", Mix_GetError()); return false; }
    if (!menu->font) { printf("Failed to load arial.ttf: %s\n", TTF_GetError()); return false; }

    Mix_PlayMusic(menu->bgMusic, -1);

    // Define buttons
    menu->buttonCount = 5;
    menu->buttons = (Button*)malloc(sizeof(Button) * menu->buttonCount);
    const char* names[] = {"Jouer", "Options", "Meilleurs scores", "Histoire", "Quitter"};

    int leftX = 100;
    int startY = 150;
    int gapY = 70;

    for (int i = 0; i < menu->buttonCount; i++) {
        menu->buttons[i].rect.w = 200;
        menu->buttons[i].rect.h = 50;
        menu->buttons[i].text = names[i];
        menu->buttons[i].hovered = false;

        if (i < 4) {
            // Left column
            menu->buttons[i].rect.x = leftX;
            menu->buttons[i].rect.y = startY + i * gapY;
        } else {
            // Bottom-right Quitter
            menu->buttons[i].rect.x = screenWidth - menu->buttons[i].rect.w - 50;
            menu->buttons[i].rect.y = screenHeight - menu->buttons[i].rect.h - 50;
        }
    }

    return true;
}

void Menu_HandleEvent(Menu* menu, SDL_Event* e, bool* quit) {
    if (e->type == SDL_QUIT) *quit = true;

    if (e->type == SDL_KEYDOWN) {
        switch (e->key.keysym.sym) {
            case SDLK_j: printf("Jouer clicked\n"); break;
            case SDLK_o: printf("Options clicked\n"); break;
            case SDLK_m: printf("Meilleurs scores clicked\n"); break;
            case SDLK_ESCAPE: *quit = true; break;
        }
    }

    if (e->type == SDL_MOUSEBUTTONDOWN) {
        int x = e->button.x, y = e->button.y;
        for (int i = 0; i < menu->buttonCount; i++) {
            SDL_Rect r = menu->buttons[i].rect;
            if (x > r.x && x < r.x + r.w && y > r.y && y < r.y + r.h) {
                if (strcmp(menu->buttons[i].text, "Quitter") == 0) *quit = true;
                else printf("%s clicked\n", menu->buttons[i].text);
            }
        }
    }

    // Mouse hover
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    for (int i = 0; i < menu->buttonCount; i++) {
        SDL_Rect r = menu->buttons[i].rect;
        bool hovering = (mx > r.x && mx < r.x + r.w && my > r.y && my < r.y + r.h);
        if (hovering && !menu->buttons[i].hovered) {
            menu->buttons[i].hovered = true;
            Mix_PlayChannel(-1, menu->hoverSound, 0);
        } else if (!hovering) {
            menu->buttons[i].hovered = false;
        }
    }
}

void Menu_Render(Menu* menu, SDL_Renderer* renderer) {
    int screenWidth = 800;
    int screenHeight = 600;

    SDL_RenderClear(renderer);

    // Background
    SDL_RenderCopy(renderer, menu->bgTexture, NULL, NULL);

    // Top-right corner image
    int cornerW = 150;
    int cornerH = 150;
    SDL_Rect cornerRect = {screenWidth - cornerW - 20, 20, cornerW, cornerH};
    SDL_RenderCopy(renderer, menu->cornerTexture, NULL, &cornerRect);

    // Buttons
    for (int i = 0; i < menu->buttonCount; i++) {
        SDL_Rect dst = menu->buttons[i].rect;
        if (menu->buttons[i].hovered) {
            dst.x -= 5; dst.y -= 5; dst.w += 10; dst.h += 10;
        }
        SDL_RenderCopy(renderer, menu->buttonTexture, NULL, &dst);

        SDL_Color color = {255, 255, 255, 255};
        SDL_Surface* surf = TTF_RenderText_Blended(menu->font, menu->buttons[i].text, color);
        SDL_Texture* textTex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_Rect textRect = {dst.x + 20, dst.y + 10, surf->w, surf->h};
        SDL_FreeSurface(surf);
        SDL_RenderCopy(renderer, textTex, NULL, &textRect);
        SDL_DestroyTexture(textTex);
    }

    SDL_RenderPresent(renderer);
}

void Menu_Cleanup(Menu* menu) {
    free(menu->buttons);
    SDL_DestroyTexture(menu->bgTexture);
    SDL_DestroyTexture(menu->buttonTexture);
    SDL_DestroyTexture(menu->cornerTexture);
    Mix_FreeMusic(menu->bgMusic);
    Mix_FreeChunk(menu->hoverSound);
    TTF_CloseFont(menu->font);
}
