#include "sprite.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>

int Sprite_Load(Sprite* sprite, SDL_Renderer* renderer, const char* path,
                int frameW, int frameH, int numFrames, int cols)
{
    if (!sprite || !renderer || !path) return -1;

    sprite->texture = IMG_LoadTexture(renderer, path);
    if (!sprite->texture) {
        SDL_Log("Failed to load texture %s: %s", path, IMG_GetError());
        return -1;
    }

    sprite->frameWidth = frameW;
    sprite->frameHeight = frameH;
    sprite->numFrames = numFrames;
    sprite->cols = cols;

    sprite->currentFrame = 0;
    sprite->lastTime = SDL_GetTicks();
    sprite->speed = 150;

    sprite->srcRect = (SDL_Rect){0, 0, frameW, frameH};
    sprite->dstRect = (SDL_Rect){0, 0, frameW, frameH};

    return 0;
}

void Sprite_Update(Sprite* sprite)
{
    if (!sprite || !sprite->texture) return;

    Uint32 now = SDL_GetTicks();
    if (now > sprite->lastTime + sprite->speed) {
        sprite->currentFrame = (sprite->currentFrame + 1) % sprite->numFrames;
        sprite->lastTime = now;
    }

    // Animation verticale
    sprite->srcRect.x = (sprite->currentFrame % sprite->cols) * sprite->frameWidth;
    sprite->srcRect.y = (sprite->currentFrame / sprite->cols) * sprite->frameHeight;
}

void Sprite_Render(Sprite* sprite, SDL_Renderer* renderer)
{
    if (!sprite || !sprite->texture || !renderer) return;
    SDL_RenderCopy(renderer, sprite->texture, &sprite->srcRect, &sprite->dstRect);
}

void Sprite_Destroy(Sprite* sprite)
{
    if (!sprite) return;
    if (sprite->texture) {
        SDL_DestroyTexture(sprite->texture);
        sprite->texture = NULL;
    }
}
