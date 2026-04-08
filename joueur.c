#include "joueur.h"


void initJoueur(Joueur *j, SDL_Renderer *renderer)
{
    // Load the sprite (single image for the character)
    j->sprite = IMG_LoadTexture(renderer, "/home/khalil/avatar-game/backavatar.png");  
    j->pos.x = 100;  
    j->pos.y = 700;  
    j->pos.w = 128;   
    j->pos.h = 128;   

    j->score = 0;
    j->hearts = 3;
    j->flip = SDL_FLIP_NONE;  // No flip initially (right direction)
    j->lastScoreTime = SDL_GetTicks();  // Start the timer for score
}


void deplacerJoueur(Joueur *j, SDL_Event event)
{
    if (event.type == SDL_KEYDOWN) 
    {
        switch (event.key.keysym.sym)
        {
            case SDLK_d: 
                j->flip = SDL_FLIP_NONE;  
                j->pos.x += 5;  
                break;

            case SDLK_q:  
                j->flip = SDL_FLIP_HORIZONTAL; 
                j->pos.x -= 5;  
                break;
        }
    }
}


void afficherJoueur(Joueur j, SDL_Renderer *renderer)
{
    SDL_RenderCopyEx(renderer, j.sprite, NULL, &j.pos, 0, NULL, j.flip); 
}


void displayHearts(Joueur j, SDL_Renderer *renderer)
{
    SDL_Texture *heartFull = IMG_LoadTexture(renderer, "/home/khalil/avatar-game/6-pixel-heart-4.png");
    SDL_Texture *heartEmpty = IMG_LoadTexture(renderer, "/home/khalil/avatar-game/6-pixel-heart-4.png");

    SDL_Rect heartRect = { 10, 10, 32, 32 };  

    for (int i = 0; i < 3; i++) {
        if (i < j.hearts) {
            SDL_RenderCopy(renderer, heartFull, NULL, &heartRect);  // Full heart
        } else {
            SDL_RenderCopy(renderer, heartEmpty, NULL, &heartRect);  // Empty heart
        }
        heartRect.x += 40; 
    }
}

// Update the animation frame based on movement direction
void updateAnimation(Joueur *j) {}
// No animation logic since we're using a single image for movement
// The logic for movement flip is handled by SDL_RenderCopyEx


void updateScore(Joueur *j, Uint32 currentTime)
{
    if (currentTime - j->lastScoreTime >= 3000) {  
        j->score += 50;  
        j->lastScoreTime = currentTime;  // Reset the timer
    }
}

// Render the score in gold color
void renderScore(Joueur j, SDL_Renderer *renderer, TTF_Font *font)
{
    char scoreText[32];
    sprintf(scoreText, "Score: %d", j.score);

    SDL_Color gold = {255, 215, 0};  // RGB for gold color

    // Create surface and texture for the score text
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, scoreText, gold);
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    // Set the position for the score text
    SDL_Rect textRect = { 300, 10, textSurface->w, textSurface->h };  

    // Render the score text
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    // Clean up
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}
