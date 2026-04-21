#ifndef HERO_H
#define HERO_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#define MAX_BULLETS   20
#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600
#define MAX_LIVES     3

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

/* Sprite sheet row offsets (pixels) */
#define ROW_IDLE    0
#define ROW_WALK    128
#define ROW_DASH    256
#define ROW_JUMP    384
#define ROW_SHOOT   512
#define ROW_HURT    640
#define ROW_DEAD    768

/* Frame counts per state */
#define FRAMES_IDLE  4
#define FRAMES_WALK  6
#define FRAMES_DASH  4
#define FRAMES_JUMP  6
#define FRAMES_SHOOT 4
#define FRAMES_HURT  3
#define FRAMES_DEAD  5

typedef struct {
    float x, y;
    float vx;          /* horizontal velocity */
    int   active;
    Direction dir;
} Bullet;

typedef struct {
    int id;

    SDL_Texture *spriteSheetRight;
    SDL_Texture *spriteSheetLeft;
    SDL_Texture *healthIcon;

    SDL_Rect frameRect;   /* source rect on sprite sheet */
    SDL_Rect posHero;     /* world position & size       */

    Direction dir;
    State     state;

    int frameWidth, frameHeight;
    int frameDelay, frameTimer;
    int currentFrame;
    int maxFrames;

    /* Special-move timers */
    int dashTime;
    int jumpTime;
    float jumpVy;         /* vertical velocity for jump arc */

    /* Boundary: player must stay within [0..worldW) x [0..worldH) */
    int worldW, worldH;

    /* Input flags (set by main loop) */
    int moveLeft, moveRight, moveUp, moveDown;

    /* Health */
    int lives;
    int hurtTimer;
    int dead;
    int invincible;       /* brief invincibility after hurt */
    int invTimer;

    /* Shooting */
    Bullet bullets[MAX_BULLETS];
    int    shootCooldown;
    int    shootAnimTimer;
    int    shootFrames;

    /* Score */
    int score;
    int scoreTimer;

} Hero;

/* Lifecycle */
void initializePlayer(Hero *hero, int id, SDL_Renderer *renderer);
void freePlayer(Hero *hero);

/* Per-frame update — bounds taken from hero->worldW / hero->worldH */
void moveHero(Hero *hero);
void setHeroBounds(Hero *hero, int w, int h);

/* Rendering */
void showPlayer(SDL_Renderer *renderer, Hero *hero, int camX, int camY);
void drawBullets(Hero *hero, SDL_Renderer *renderer, int camX, int camY);
void drawHUD(Hero *hero, SDL_Renderer *renderer, TTF_Font *font, int offsetX, int offsetY);

/* Bullets */
void shoot(Hero *hero);
void updateBullets(Hero *hero);
void checkBulletHit(Hero *attacker, Hero *target);

/* Score */
void updateScore(Hero *hero);
void addKillScore(Hero *hero);

/* Damage */
void takeDamage(Hero *hero);

#endif /* HERO_H */
