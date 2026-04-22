#ifndef HERO_H
#define HERO_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>


#define SCREEN_WIDTH   800
#define SCREEN_HEIGHT  600
#define MAX_BULLETS    20
#define MAX_LIVES       3


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


#define ROW_IDLE    0
#define ROW_WALK    128
#define ROW_DASH    256
#define ROW_JUMP    384
#define ROW_SHOOT   512
#define ROW_HURT    640
#define ROW_DEAD    768


#define FRAMES_IDLE   4
#define FRAMES_WALK   6
#define FRAMES_DASH   4
#define FRAMES_JUMP   6
#define FRAMES_SHOOT  4
#define FRAMES_HURT   3
#define FRAMES_DEAD   5

// Bullet struct 
typedef struct {
    float     x, y;
    float     vx;
    int       active;
    Direction dir;
} Bullet;

// Hero struct 
typedef struct {
    int id;

    SDL_Texture *spriteSheetRight;
    SDL_Texture *spriteSheetLeft;
    SDL_Texture *healthIcon;

    SDL_Rect frameRect;   
    SDL_Rect posHero;    

    Direction dir;
    State     state;

    int frameWidth, frameHeight;
    int frameDelay, frameTimer;
    int currentFrame, maxFrames;

    
    int   dashTime;
    int   jumpTime;
    float jumpVy;

    
    int worldW, worldH;

    
    int moveLeft, moveRight, moveUp, moveDown;

    
    int lives;
    int hurtTimer;
    int dead;
    int invincible;
    int invTimer;

   
    Bullet bullets[MAX_BULLETS];
    int    shootCooldown;
    int    shootAnimTimer;
    int    shootFrames;

    
    int score;
    int scoreTimer;
    int scoreActive;   
} Hero;


void setState_public(Hero *hero, State newState);


void initializePlayer(Hero *hero, int id, SDL_Renderer *renderer);
void freePlayer(Hero *hero);


void moveHero(Hero *hero);
void setHeroBounds(Hero *hero, int w, int h);


void showPlayer(SDL_Renderer *renderer, Hero *hero, int camX, int camY);
void drawBullets(Hero *hero, SDL_Renderer *renderer, int camX, int camY);
void drawHUD(Hero *hero, SDL_Renderer *renderer, TTF_Font *font,
             int offsetX, int offsetY);

void shoot(Hero *hero);
void updateBullets(Hero *hero);
void checkBulletHit(Hero *attacker, Hero *target);


void takeDamage(Hero *hero);


void updateScore(Hero *hero);
void addKillScore(Hero *hero);

#endif 
