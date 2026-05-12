#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

typedef enum { MENU_SAUVEGARDE, MENU_OPTIONS } EtatMenu;

// Structure bech n-giriw el buttons as-hel
typedef struct {
    SDL_Rect rect;
    const char* name;
} Button;

void chargerTextures(SDL_Renderer* renderer, SDL_Texture** tex1, SDL_Texture** tex2);
void afficherMenu(SDL_Renderer* renderer, EtatMenu etat, SDL_Texture* tex1, SDL_Texture* tex2);
void libererTextures(SDL_Texture* tex1, SDL_Texture* tex2);

#endif
