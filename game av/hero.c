#include "hero.h"
#include <SDL_image.h>
#include <stdio.h>

void initializePlayer(Hero *hero) {
    hero->id = 1;
    sprintf(hero->name, "Hero1");

    hero->maxHP = 100;
    hero->currentHP = 100;
    hero->lives = 3;

    hero->frameWidth = 128;
    hero->frameHeight = 128;

    hero->posHero.x = 100;
    hero->posHero.y = 300;

    hero->spriteSheetRight = IMG_Load("/home/khalil/game av/images/persoR.png");
    hero->spriteSheetLeft  = IMG_Load("/home/khalil/game av/images/persoL.png");
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

    // DEAD
    if (hero->dead) {
        hero->frameTimer++;
        if (hero->frameTimer >= hero->frameDelay) {
            hero->frameTimer = 0;
            hero->frameRect.x += hero->frameWidth;
        }
        if (hero->frameRect.x >= hero->frameWidth * 6) {
            hero->frameRect.x = 0; // Reset to first frame
        }
        return;
    }

    // HURT
    if (hero->state == STATE_HURT) {
        hero->frameTimer++;
        if (hero->frameTimer >= hero->frameDelay) {
            hero->frameTimer = 0;
            hero->frameRect.x += hero->frameWidth;
        }

        hero->hurtTimer--;
        if (hero->hurtTimer <= 0)
            hero->state = STATE_IDLE;

        return;
    }

    // WALK
    if (hero->state == STATE_WALK) {
        hero->frameTimer++;
        if (hero->frameTimer >= hero->frameDelay) {
            hero->frameTimer = 0;
            hero->frameRect.x += hero->frameWidth;
        }

        if (hero->frameRect.x >= hero->frameWidth * 6) {
            hero->frameRect.x = 0; // Reset to first frame
        }

        if (hero->dir == DIR_LEFT) hero->posHero.x -= speed;
        if (hero->dir == DIR_RIGHT) hero->posHero.x += speed;
        if (hero->dir == DIR_UP) hero->posHero.y -= speed;
        if (hero->dir == DIR_DOWN) hero->posHero.y += speed;
    }

    // DASH
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

	    // JUMP
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

	    // First half → GO UP
	    if (hero->jumpTime > 10) {
		hero->posHero.y -= jumpHeight;
	    }
	    // Second half → GO DOWN
	    else if (hero->jumpTime > 0) {
		hero->posHero.y += jumpHeight;
	    }

	    // End of jump
	    if (hero->jumpTime <= 0) {
		hero->state = STATE_IDLE;
	    }

	    hero->jumpTime--;
	}

}

void takeDamage(Hero *hero, int damage) {
    if (hero->dead) return;

    hero->currentHP -= damage;
    if (hero->currentHP < 0) hero->currentHP = 0;

    hero->lives = (hero->currentHP + 32) / 33;

    if (hero->currentHP == 0) {
        hero->dead = 1;
        hero->state = STATE_DEAD;
    } else {
        hero->state = STATE_HURT;
        hero->hurtTimer = 10;
    }
}

void drawHealth(Hero *hero, SDL_Surface *screen) {
    SDL_Rect pos = {10, 10};

    SDL_BlitSurface(hero->healthIcon, NULL, screen, &pos);

    for (int i = 0; i < hero->lives; i++) {
        SDL_Rect xPos = {50 + i * 30, 10, 20, 20};
        SDL_FillRect(screen, &xPos, SDL_MapRGB(screen->format, 255, 0, 0));
    }
}
