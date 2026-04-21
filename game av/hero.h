#ifndef HERO_H
#define HERO_H

#include <SDL.h>
#include <stdbool.h>
#include <SDL_ttf.h>

// Direction
typedef enum { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT } Direction;

// State
typedef enum {
    STATE_IDLE,
    STATE_WALK,
    STATE_DASH,
    STATE_JUMP,
    STATE_HURT,
    STATE_DEAD
} State;

typedef struct {
    int id;
    char name[50];

    int maxHP, currentHP;
    int health; // Track the number of hits the player can take

    SDL_Surface* spriteSheetRight;
    SDL_Surface* spriteSheetLeft;
    SDL_Surface* healthIcon;

    SDL_Rect frameRect;
    SDL_Rect posHero;

    Direction dir;
    State state;

    int frameWidth, frameHeight;

    int leftCounter, rightCounter, upCounter, downCounter;
    int dashCounter, jumpCounter;

    int dashTime;
    int jumpTime;

    int frameDelay;
    int frameTimer;

    // 🔥 HEALTH SYSTEM
    int lives;      // 3 segments (3 health points)
    int hurtTimer;
    int dead;

} Hero;

void initializePlayer(Hero *hero, int id);
void showPlayer(SDL_Surface* screen, Hero *hero);
void moveHero(Hero *hero);
void takeDamage(Hero *hero, int damage);
void drawHealth(Hero *hero, SDL_Surface *screen, TTF_Font *font, SDL_Color color);

#endif
