#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

typedef enum { MENU_OPTIONS, MENU_SAUVEGARDE } EtatMenu;

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

    SDL_Window* window = SDL_CreateWindow("Avatar System", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1000, 562, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Thabbet f-asemi el tsawer f-dossier assets/
    SDL_Texture* texOptions = IMG_LoadTexture(renderer, "photo2.png");
    SDL_Texture* texSauvegarde = IMG_LoadTexture(renderer, "photo1.png");

    if (!texOptions || !texSauvegarde) {
        printf("ERREUR: Mal9itech el tsawer! Thabbet f-dossier assets.\n");
        return 1;
    }

    // --- Coordonnées Rectangles (Hathom lezem ykounou kbar bech l-mouse t-captihom) ---
    // Options: Bouton RETOUR louta 3al issar
    SDL_Rect btnVersSave = {20, 450, 250, 100}; 
    // Sauvegarde: Bouton RETOUR fil west louta
    SDL_Rect btnVersOptions = {350, 450, 300, 100};

    EtatMenu etatActuel = MENU_OPTIONS;
    int running = 1;
    SDL_Event event;

    while (running) {
        // 1. Gérer les inputs
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;

            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mx = event.button.x;
                int my = event.button.y;
                SDL_Point p = {mx, my};

                if (etatActuel == MENU_OPTIONS) {
                    // Ken d9at louta 3al issar f-el Options
                    if (SDL_PointInRect(&p, &btnVersSave)) {
                        printf("Passage vers SAUVEGARDE...\n");
                        etatActuel = MENU_SAUVEGARDE;
                    }
                } 
                else if (etatActuel == MENU_SAUVEGARDE) {
                    // Ken d9at fil west louta f-el Sauvegarde
                    if (SDL_PointInRect(&p, &btnVersOptions)) {
                        printf("Retour vers OPTIONS...\n");
                        etatActuel = MENU_OPTIONS;
                    }
                }
            }
        }

        // 2. RENDERING (Hna el s7i7: dima n-warriw el taswira 3la 7asb l-etat)
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (etatActuel == MENU_OPTIONS) {
            SDL_RenderCopy(renderer, texOptions, NULL, NULL);
            // Ken t7eb tchouf el blasa mta3 el bouton (Rectangle a7mer) bech t-thabbet:
            // SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            // SDL_RenderDrawRect(renderer, &btnVersSave);
        } else {
            SDL_RenderCopy(renderer, texSauvegarde, NULL, NULL);
            // SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            // SDL_RenderDrawRect(renderer, &btnVersOptions);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16); 
    }

    SDL_DestroyTexture(texOptions);
    SDL_DestroyTexture(texSauvegarde);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
