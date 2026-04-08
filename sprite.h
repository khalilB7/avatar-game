#ifndef SPRITE_H
#define SPRITE_H

#include <SDL2/SDL.h>
#include <stdint.h> // pour Uint32

typedef struct {
    SDL_Texture* texture;
    int frameWidth;
    int frameHeight;
    int numFrames;
    int cols;

    int currentFrame;
    Uint32 lastTime;
    Uint32 speed; // ms par frame

    SDL_Rect srcRect;
    SDL_Rect dstRect;
} Sprite;

// Charge un sprite depuis une image
// Retourne 0 si succès, -1 si erreur
int Sprite_Load(Sprite* sprite, SDL_Renderer* renderer, const char* path,
                int frameW, int frameH, int numFrames, int cols);

// Met à jour l'animation du sprite
void Sprite_Update(Sprite* sprite);

// Affiche le sprite
void Sprite_Render(Sprite* sprite, SDL_Renderer* renderer);

// Libère la mémoire
void Sprite_Destroy(Sprite* sprite);

#endif // SPRITE_H
