#include "menu.h"
#include <stdio.h>

void chargerTextures(SDL_Renderer* renderer, SDL_Texture** tex1, SDL_Texture** tex2) {
    // Thabet dima elli assemi el tsawer mrigla foust dossier assets
    *tex1 = IMG_LoadTexture(renderer, "photo1.png");
    *tex2 = IMG_LoadTexture(renderer, "photo2.png");

    if (*tex1 == NULL || *tex2 == NULL) {
        printf("Erreur: Impossible de charger les images! %s\n", IMG_GetError());
    }
}

void afficherMenu(SDL_Renderer* renderer, EtatMenu etat, SDL_Texture* tex1, SDL_Texture* tex2) {
    SDL_RenderClear(renderer);
    if (etat == MENU_SAUVEGARDE) {
        SDL_RenderCopy(renderer, tex1, NULL, NULL);
    } else {
        SDL_RenderCopy(renderer, tex2, NULL, NULL);
    }
    SDL_RenderPresent(renderer);
}

void libererTextures(SDL_Texture* tex1, SDL_Texture* tex2) {
    if (tex1) SDL_DestroyTexture(tex1);
    if (tex2) SDL_DestroyTexture(tex2);
}
