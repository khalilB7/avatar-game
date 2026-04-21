#include "hero.h"
#include <SDL_image.h>
#include <stdio.h>
#include <SDL_ttf.h>  // Add SDL_ttf for font support

void initializePlayer(Hero *hero, int id) {
    hero->id = id;
    sprintf(hero->name, "Hero%d", id);

    hero->maxHP = 100;
    hero->currentHP = 100;
    hero->health = 3; // Player starts with 3 health (for visualization)
    hero->lives = 3;    // Set the lives to 3

    hero->frameWidth = 128;
    hero->frameHeight = 128;

    hero->posHero.x = 100 + (id * 150); // Give each player a different starting X position
    hero->posHero.y = 300;

    // Assign different sprite sheets for each player
    if (id == 1) {
        hero->spriteSheetRight = IMG_Load("/home/khalil/game av/images/soldierR.png");
        hero->spriteSheetLeft  = IMG_Load("/home/khalil/game av/images/soldierL.png");
    } else if (id == 2) {
        hero->spriteSheetRight = IMG_Load("/home/khalil/game av/images/persoR.png"); // Player 2 sprite sheet for right
        hero->spriteSheetLeft  = IMG_Load("/home/khalil/game av/images/persoL.png"); // Player 2 sprite sheet for left
    }

    hero->healthIcon       = IMG_Load("/home/khalil/game av/images/Icon12.png");

    hero->frameRect.x = 0;
    hero->frameRect.y = 0;
    hero->frameRect.w = hero->frameWidth;
    hero->frameRect.h = hero->frameHeight;

    hero->dir = DIR_RIGHT;
    hero->state = STATE_IDLE;

    hero->frameDelay = 6;
    hero->frameTimer = 0;

    hero->dashTime = 0;
    hero->jumpTime = 0;

    hero->hurtTimer = 0;
    hero->dead = 0;
}

void showPlayer(SDL_Surface* screen, Hero *hero) {
    SDL_Surface* imageHero;

    if (hero->dir == DIR_RIGHT)
        imageHero = hero->spriteSheetRight;
    else
        imageHero = hero->spriteSheetLeft;

    switch (hero->state) {
        case STATE_WALK: hero->frameRect.y = hero->frameHeight; break;
        case STATE_DASH: hero->frameRect.y = hero->frameHeight * 2; break;
        case STATE_JUMP: hero->frameRect.y = hero->frameHeight * 3; break;
        case STATE_HURT: hero->frameRect.y = hero->frameHeight * 4; break;
        case STATE_DEAD: hero->frameRect.y = hero->frameHeight * 5; break;
        default: hero->frameRect.y = 0; break;
    }

    SDL_BlitSurface(imageHero, &hero->frameRect, screen, &hero->posHero);
}

void moveHero(Hero *hero) {
    int speed = 5;

    // DEAD - Play death animation and stop all actions
    if (hero->dead) {
        hero->frameTimer++;
        if (hero->frameTimer >= hero->frameDelay) {
            hero->frameTimer = 0;
            hero->frameRect.x += hero->frameWidth;
        }
        if (hero->frameRect.x >= hero->frameWidth * 6) {
            hero->frameRect.x = 0; // Reset to the first frame of the death animation
            return;  // Stop further actions after the death animation
        }
        return;  // Stop further movement or actions for the dead player
    }

    // HURT - Play hurt animation when damaged
    if (hero->state == STATE_HURT) {
        hero->frameTimer++;
        if (hero->frameTimer >= hero->frameDelay) {
            hero->frameTimer = 0;
            hero->frameRect.x += hero->frameWidth;
        }

        hero->hurtTimer--;
        if (hero->hurtTimer <= 0)
            hero->state = STATE_IDLE;  // After hurt animation, return to idle state

        return;
    }

    // WALK - Handle walk animation and movement
    if (hero->state == STATE_WALK) {
        hero->frameTimer++;
        if (hero->frameTimer >= hero->frameDelay) {
            hero->frameTimer = 0;
            hero->frameRect.x += hero->frameWidth;
        }

        if (hero->frameRect.x >= hero->frameWidth * 6) {
            hero->frameRect.x = 0; // Reset to first frame
        }

        // Movement
        if (hero->dir == DIR_LEFT) hero->posHero.x -= speed;
        if (hero->dir == DIR_RIGHT) hero->posHero.x += speed;
        if (hero->dir == DIR_UP) hero->posHero.y -= speed;
        if (hero->dir == DIR_DOWN) hero->posHero.y += speed;
    }

    // DASH - Handle dash animation and movement
    if (hero->state == STATE_DASH) {
        hero->frameTimer++;
        if (hero->frameTimer >= hero->frameDelay) {
            hero->frameTimer = 0;
            hero->frameRect.x += hero->frameWidth;
        }

        if (hero->frameRect.x >= hero->frameWidth * 6) {
            hero->frameRect.x = 0; // Reset to first frame
        }

        int dashSpeed = 15;
        if (hero->dir == DIR_LEFT) hero->posHero.x -= dashSpeed;
        if (hero->dir == DIR_RIGHT) hero->posHero.x += dashSpeed;

        hero->dashTime--;
        if (hero->dashTime <= 0)
            hero->state = STATE_IDLE;
    }

    // JUMP - Handle jump animation and movement
    if (hero->state == STATE_JUMP) {
        hero->frameTimer++;
        if (hero->frameTimer >= hero->frameDelay) {
            hero->frameTimer = 0;
            hero->frameRect.x += hero->frameWidth;
        }

        if (hero->frameRect.x >= hero->frameWidth * 6) {
            hero->frameRect.x = 0;
        }

        int jumpHeight = 10;
        int gravity = 2; // Gravity strength

        // First half → GO UP
        if (hero->jumpTime > 10) {
            hero->posHero.y -= jumpHeight;
        }
        // Second half → GO DOWN
        else if (hero->jumpTime > 0) {
            hero->posHero.y += jumpHeight + gravity; // Apply gravity
        }

        // End of jump
        if (hero->jumpTime <= 0) {
            hero->state = STATE_IDLE;
        }

        hero->jumpTime--;
    }
}

void takeDamage(Hero *hero, int damage) {
    if (hero->dead) return; // If the player is already dead, don't take damage

    // Decrease player's lives by 1 for each hit
    hero->lives -= 1;  
    if (hero->lives <= 0) {
        hero->lives = 0;
        hero->dead = 1;  // Player dies after losing 3 lives
        hero->state = STATE_DEAD;  // Set state to dead to play the death animation
    } else {
        hero->state = STATE_HURT;  // Play hurt animation
        hero->hurtTimer = 10;
    }
}

void drawHealth(Hero *hero, SDL_Surface *screen, TTF_Font *font, SDL_Color color) {
    SDL_Rect pos = {10, 10};

    // Player health icon
    SDL_BlitSurface(hero->healthIcon, NULL, screen, &pos);

    // Draw the text displaying the player’s name and health
    char healthText[50];
    sprintf(healthText, "Player %d: %d", hero->id, hero->lives);

    SDL_Surface* textSurface = TTF_RenderText_Solid(font, healthText, color);
    SDL_Rect textRect = {10, 50 + (hero->id - 1) * 30, textSurface->w, textSurface->h}; // Adjusting the position for Player 2
    SDL_BlitSurface(textSurface, NULL, screen, &textRect);

    // Display the remaining lives as hearts
    for (int i = 0; i < hero->lives; i++) {  
        SDL_Rect xPos = {50 + i * 30, 10, 20, 20};
        SDL_FillRect(screen, &xPos, SDL_MapRGB(screen->format, 255, 0, 0));
    }
    SDL_FreeSurface(textSurface);
}
