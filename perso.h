#ifndef FONCTION_H
#define FONCTION_H
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define SCREEN_H 800
#define SCREEN_W 1200

typedef struct {
    SDL_Texture *img;
    SDL_Rect pos_img_ecran;
} image;

typedef struct {
    SDL_Rect position;
    SDL_Texture *images[17];
    int current_image;
    int direction;
    int move;
    int jump;
    int jump_speed;
    int is_attacking;
    int attack_frame;
    int healthpoints;
} Personne;

void initPerso(Personne *p, SDL_Renderer *renderer);
void afficherPersoWithCamera(Personne p, SDL_Renderer *renderer, SDL_Rect camera);
void updateCamera(SDL_Rect *camera, Personne p, int level_width);

#endif