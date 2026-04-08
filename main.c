/* main.c */
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include "buttons.h"
#include "background.h"
#include "music.h"

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

int main(int argc, char *argv[]) {
    // Initialisation SDL (vidéo + audio)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "Erreur SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    // Initialiser SDL_ttf
    if (TTF_Init() != 0) {
        fprintf(stderr, "Erreur TTF_Init: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }
    // Initialiser SDL_image (support PNG)
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        fprintf(stderr, "Erreur IMG_Init: %s\n", IMG_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Créer la fenêtre et le renderer
    SDL_Window *window = SDL_CreateWindow("Menu d\u2019options",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "Erreur SDL_CreateWindow: %s\n", SDL_GetError());
        IMG_Quit(); TTF_Quit(); SDL_Quit();
        return 1;
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "Erreur SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        IMG_Quit(); TTF_Quit(); SDL_Quit();
        return 1;
    }

    // Charger le fond d'écran (ex: "background.png")
    loadBackground(renderer, "/home/khalil/option/official_avatar_movie_poster.png");

    // Charger une police TTF pour les textes des boutons et du volume
    TTF_Font *font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 24);
    if (!font) {
        fprintf(stderr, "Impossible de charger la police: %s\n", TTF_GetError());
        // On peut continuer sans texte (dangereux, mais géré ci-dessous)
    }

    // Initialiser les boutons (avec la police pour le texte)
    initButtons(renderer, font);

    // Initialiser la musique de fond (ex: "music.mp3")
    if (initMusic("/home/khalil/option/whvle-aboard-a-aurora-game-menu-pulse-203549.wav") != 0) {
        fprintf(stderr, "Échec initialisation musique\n");
        // On continue quand même sans musique
    }
    // Volume initial en pourcentage
    int volumePercent = Mix_VolumeMusic(-1) * 100 / MIX_MAX_VOLUME;

    // Préparer le texte du volume (sera mis à jour au changement)
    SDL_Texture *volText = NULL;
    SDL_Rect volRect = {0, 0, 0, 0};
    if (font) {
        char volStr[32];
        sprintf(volStr, "Volume : %d%%", volumePercent);
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface *surf = TTF_RenderText_Blended(font, volStr, white);
        if (surf) {
            volText = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_QueryTexture(volText, NULL, NULL, &volRect.w, &volRect.h);
            volRect.x = (WINDOW_WIDTH - volRect.w) / 2;
            volRect.y = 50; // position verticale fixe pour le texte du volume
            SDL_FreeSurface(surf);
        }
    }

    // Boucle principale
    SDL_Event e;
    int running = 1;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                running = 0;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                int mx = e.button.x, my = e.button.y;
                ButtonID bid = checkButtonClick(mx, my);
                switch (bid) {
                    case BTN_VOL_UP:
                        if (font) {
                            volumePercent = increaseVolume();
                            // Mettre à jour le texte du volume
                            if (volText) SDL_DestroyTexture(volText);
                            char buf[32];
                            sprintf(buf, "Volume : %d%%", volumePercent);
                            SDL_Color white = {255, 255, 255, 255};
                            SDL_Surface *surf = TTF_RenderText_Blended(font, buf, white);
                            if (surf) {
                                volText = SDL_CreateTextureFromSurface(renderer, surf);
                                SDL_QueryTexture(volText, NULL, NULL, &volRect.w, &volRect.h);
                                volRect.x = (WINDOW_WIDTH - volRect.w) / 2;
                                volRect.y = 50;
                                SDL_FreeSurface(surf);
                            }
                        } else {
                            increaseVolume();
                        }
                        break;
                    case BTN_VOL_DOWN:
                        if (font) {
                            volumePercent = decreaseVolume();
                            if (volText) SDL_DestroyTexture(volText);
                            char buf2[32];
                            sprintf(buf2, "Volume : %d%%", volumePercent);
                            SDL_Color white2 = {255, 255, 255, 255};
                            SDL_Surface *surf2 = TTF_RenderText_Blended(font, buf2, white2);
                            if (surf2) {
                                volText = SDL_CreateTextureFromSurface(renderer, surf2);
                                SDL_QueryTexture(volText, NULL, NULL, &volRect.w, &volRect.h);
                                volRect.x = (WINDOW_WIDTH - volRect.w) / 2;
                                volRect.y = 50;
                                SDL_FreeSurface(surf2);
                            }
                        } else {
                            decreaseVolume();
                        }
                        break;
                    case BTN_WINDOW:
                        // Passer en mode fenêtre (SDL_WINDOW_FULLSCREEN désactivé)
                        SDL_SetWindowFullscreen(window, 0);
                        break;
                    case BTN_FULLSCREEN:
                        // Passer en plein écran (mode desktop):contentReference[oaicite:8]{index=8}
                        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                        break;
                    case BTN_RETURN:
                        running = 0;
                        break;
                    default:
                        break;
                }
            }
        }

        // Mettre à jour le survol pour l'affichage (chaque frame)
        int mx, my;
        SDL_GetMouseState(&mx, &my);
        updateButtonHover(mx, my);

        // Rendu
        SDL_RenderClear(renderer);
        renderBackground(renderer);
        renderButtons(renderer);
        if (volText) {
            SDL_RenderCopy(renderer, volText, NULL, &volRect);
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    // Libération des ressources
    if (volText) SDL_DestroyTexture(volText);
    destroyButtons();
    destroyBackground();
    destroyMusic();
    if (font) TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    return 0;
}
