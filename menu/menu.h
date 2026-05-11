#ifndef MENU_H
#define MENU_H

#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <stdbool.h>

typedef struct {
    SDL_Rect rect;
    const char* text;
    bool hovered;
} Button;

typedef struct {
    SDL_Texture* bgTexture;
    SDL_Texture* buttonTexture;
    SDL_Texture* cornerTexture;  // Top-right image
    Mix_Music* bgMusic;
    Mix_Chunk* hoverSound;
    TTF_Font* font;
    Button* buttons;
    int buttonCount;
} Menu;

// Initialize menu
bool Menu_Init(Menu* menu, SDL_Renderer* renderer);

// Handle input/events
void Menu_HandleEvent(Menu* menu, SDL_Event* e, bool* quit);

// Render menu
void Menu_Render(Menu* menu, SDL_Renderer* renderer);

// Cleanup menu
void Menu_Cleanup(Menu* menu);

#endif
