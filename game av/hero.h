#ifndef HERO_H
#define HERO_H

#include <SDL.h>
#include <SDL_ttf.h>

#define MAX_BULLETS   20
#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

typedef enum { DIR_LEFT, DIR_RIGHT } Direction;

typedef enum {
    STATE_IDLE,
    STATE_WALK,
    STATE_DASH,
    STATE_JUMP,
    STATE_SHOOT,
    STATE_HURT,
    STATE_DEAD
} State;

// ---------------------------------------------------------------------------
// Sprite-sheet row layout  (row y-offset in pixels, frame size 128x128)
//   Row 0  (y=   0) : IDLE
//   Row 1  (y= 128) : WALK
//   Row 2  (y= 256) : DASH
//   Row 3  (y= 384) : JUMP
//   Row 4  (y= 512) : SHOOT
//   Row 5  (y= 640) : HURT
//   Row 6  (y= 768) : DEAD
// ---------------------------------------------------------------------------

// Per-state frame counts  (adjust to match your actual sprite sheet)
#define FRAMES_IDLE  4
#define FRAMES_WALK  6
#define FRAMES_DASH  4
#define FRAMES_JUMP  6
#define FRAMES_SHOOT 4
#define FRAMES_HURT  3
#define FRAMES_DEAD  5

typedef struct {
    int x, y;
    int speed;
    int active;
    Direction dir;
} Bullet;

typedef struct {
    int id;

    SDL_Surface *spriteSheetRight;
    SDL_Surface *spriteSheetLeft;
    SDL_Surface *healthIcon;

    SDL_Rect frameRect;   // source rect into the sprite sheet
    SDL_Rect posHero;     // destination on screen

    Direction dir;
    State     state;

    int frameWidth, frameHeight;
    int frameDelay, frameTimer;   // ticks between frame advances
    int currentFrame;             // column index (0-based)
    int maxFrames;                // frame count for the current state

    int dashTime, jumpTime;

    int moveLeft, moveRight, moveUp, moveDown;

    // Health
    int lives;
    int hurtTimer;
    int dead;

    // Shooting
    Bullet bullets[MAX_BULLETS];
    int shootCooldown;

    // Shoot animation
    int shootAnimTimer;
    int shootFrames;

    // Score
    int score;
    int scoreTimer;   // ticks since last time-based score increment

} Hero;

// Core
void initializePlayer(Hero *hero, int id);
void showPlayer(SDL_Surface *screen, Hero *hero);
void moveHero(Hero *hero);
void takeDamage(Hero *hero);
void drawHearts(Hero *hero, SDL_Surface *screen, TTF_Font *font);

// Shooting
void shoot(Hero *hero);
void updateBullets(Hero *hero);
void drawBullets(Hero *hero, SDL_Surface *screen);
void checkBulletHit(Hero *attacker, Hero *target);

// Score
void updateScore(Hero *hero);          // call every game tick
void addKillScore(Hero *hero);         // call when this player kills an enemy
void drawScore(Hero *hero, SDL_Surface *screen, TTF_Font *font);

#endif
