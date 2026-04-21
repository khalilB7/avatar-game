#include "hero.h"
#include <SDL_image.h>
#include <stdio.h>

// ---------------------------------------------------------------------------
// Sprite-sheet row y-offsets (one row per state, 128 px per row)
// ---------------------------------------------------------------------------
#define ROW_IDLE    0
#define ROW_WALK    128
#define ROW_DASH    256
#define ROW_JUMP    384
#define ROW_SHOOT   512
#define ROW_HURT    640
#define ROW_DEAD    768

// ---------------------------------------------------------------------------
// initializePlayer
// ---------------------------------------------------------------------------
void initializePlayer(Hero *hero, int id) {
    hero->id = id;

    hero->frameWidth  = 128;
    hero->frameHeight = 128;

    hero->posHero.x = 100 + (id * 200);
    hero->posHero.y = 300;
    hero->posHero.w = hero->frameWidth;
    hero->posHero.h = hero->frameHeight;

    if (id == 1) {
        hero->spriteSheetRight = IMG_Load("/home/khalil/game av/images/soldierR.png");
        hero->spriteSheetLeft  = IMG_Load("/home/khalil/game av/images/soldierL.png");
    } else {
        hero->spriteSheetRight = IMG_Load("/home/khalil/game av/images/C1R.png");
        hero->spriteSheetLeft  = IMG_Load("/home/khalil/game av/images/C1L.png");
    }

    hero->healthIcon = IMG_Load("/home/khalil/game av/images/Icon12.png");

    hero->dir   = DIR_RIGHT;
    hero->state = STATE_IDLE;

    hero->currentFrame = 0;
    hero->maxFrames    = FRAMES_IDLE;
    hero->frameRect    = (SDL_Rect){0, ROW_IDLE, 128, 128};

    // Lower frameDelay = faster / smoother animation
    hero->frameDelay = 4;
    hero->frameTimer = 0;

    hero->dashTime = 0;
    hero->jumpTime = 0;

    hero->moveLeft = hero->moveRight = hero->moveUp = hero->moveDown = 0;

    hero->lives     = 3;
    hero->hurtTimer = 0;
    hero->dead      = 0;

    hero->shootCooldown  = 0;
    hero->shootFrames    = FRAMES_SHOOT;
    hero->shootAnimTimer = 0;

    hero->score      = 0;
    hero->scoreTimer = 0;

    for (int i = 0; i < MAX_BULLETS; i++)
        hero->bullets[i].active = 0;
}

// ---------------------------------------------------------------------------
// Helper: set a new animation state cleanly (resets frame counter)
// ---------------------------------------------------------------------------
static void setState(Hero *hero, State newState) {
    if (hero->state == newState) return;  // already in this state, no reset

    hero->state        = newState;
    hero->currentFrame = 0;
    hero->frameTimer   = 0;

    switch (newState) {
        case STATE_IDLE:  hero->maxFrames = FRAMES_IDLE;  hero->frameRect.y = ROW_IDLE;  break;
        case STATE_WALK:  hero->maxFrames = FRAMES_WALK;  hero->frameRect.y = ROW_WALK;  break;
        case STATE_DASH:  hero->maxFrames = FRAMES_DASH;  hero->frameRect.y = ROW_DASH;  break;
        case STATE_JUMP:  hero->maxFrames = FRAMES_JUMP;  hero->frameRect.y = ROW_JUMP;  break;
        case STATE_SHOOT: hero->maxFrames = FRAMES_SHOOT; hero->frameRect.y = ROW_SHOOT; break;
        case STATE_HURT:  hero->maxFrames = FRAMES_HURT;  hero->frameRect.y = ROW_HURT;  break;
        case STATE_DEAD:  hero->maxFrames = FRAMES_DEAD;  hero->frameRect.y = ROW_DEAD;  break;
    }
    hero->frameRect.x = 0;
}

// ---------------------------------------------------------------------------
// showPlayer — renders the correct frame; dead hero plays death anim once
//              then freezes on the last death frame forever
// ---------------------------------------------------------------------------
void showPlayer(SDL_Surface *screen, Hero *hero) {
    SDL_Surface *img = (hero->dir == DIR_RIGHT)
                       ? hero->spriteSheetRight
                       : hero->spriteSheetLeft;

    // --- DEAD: always use the DEAD row, play through once then freeze ---
    if (hero->dead) {
        hero->frameRect.y = ROW_DEAD;

        // Advance the death animation until the last frame
        hero->frameTimer++;
        if (hero->frameTimer >= hero->frameDelay) {
            hero->frameTimer = 0;
            if (hero->currentFrame < FRAMES_DEAD - 1)
                hero->currentFrame++;
            // once at last frame, currentFrame stays there forever
        }
        hero->frameRect.x = hero->currentFrame * hero->frameWidth;
        SDL_BlitSurface(img, &hero->frameRect, screen, &hero->posHero);
        return;
    }

    // Always keep the source row in sync with the current state
    switch (hero->state) {
        case STATE_WALK:  hero->frameRect.y = ROW_WALK;  break;
        case STATE_DASH:  hero->frameRect.y = ROW_DASH;  break;
        case STATE_JUMP:  hero->frameRect.y = ROW_JUMP;  break;
        case STATE_SHOOT: hero->frameRect.y = ROW_SHOOT; break;
        case STATE_HURT:  hero->frameRect.y = ROW_HURT;  break;
        default:          hero->frameRect.y = ROW_IDLE;  break;
    }

    // Advance frame on timer
    hero->frameTimer++;
    if (hero->frameTimer >= hero->frameDelay) {
        hero->frameTimer = 0;

        if (hero->state == STATE_SHOOT || hero->state == STATE_HURT) {
            // Clamp at last frame; the state machine handles the transition
            if (hero->currentFrame < hero->maxFrames - 1)
                hero->currentFrame++;
        } else {
            // All other live states loop
            hero->currentFrame = (hero->currentFrame + 1) % hero->maxFrames;
        }

        hero->frameRect.x = hero->currentFrame * hero->frameWidth;
    }

    SDL_BlitSurface(img, &hero->frameRect, screen, &hero->posHero);
}

// ---------------------------------------------------------------------------
// clampPosition — keep the hero fully inside the screen
// ---------------------------------------------------------------------------
static void clampPosition(Hero *hero) {
    if (hero->posHero.x < 0)
        hero->posHero.x = 0;
    if (hero->posHero.x > SCREEN_WIDTH - hero->frameWidth)
        hero->posHero.x = SCREEN_WIDTH - hero->frameWidth;
    if (hero->posHero.y < 0)
        hero->posHero.y = 0;
    if (hero->posHero.y > SCREEN_HEIGHT - hero->frameHeight)
        hero->posHero.y = SCREEN_HEIGHT - hero->frameHeight;
}

// ---------------------------------------------------------------------------
// moveHero — state machine updated every game tick
// ---------------------------------------------------------------------------
void moveHero(Hero *hero) {

    const int speed = 5;

    if (hero->shootCooldown > 0)
        hero->shootCooldown--;

    // Dead: completely frozen, stays on last death frame
    if (hero->dead) {
        setState(hero, STATE_DEAD);
        return;
    }

    // --- HURT ---
    if (hero->state == STATE_HURT) {
        hero->hurtTimer--;
        if (hero->hurtTimer <= 0)
            setState(hero, STATE_IDLE);
        return;
    }

    // --- DASH ---
    if (hero->state == STATE_DASH) {
        const int dashSpeed = 15;
        if (hero->dir == DIR_LEFT)  hero->posHero.x -= dashSpeed;
        if (hero->dir == DIR_RIGHT) hero->posHero.x += dashSpeed;
        clampPosition(hero);
        hero->dashTime--;
        if (hero->dashTime <= 0)
            setState(hero, STATE_IDLE);
        return;
    }

    // --- JUMP ---
    if (hero->state == STATE_JUMP) {
        const int jumpPower = 10;
        const int gravity   = 3;
        if (hero->jumpTime > 10)
            hero->posHero.y -= jumpPower;
        else if (hero->jumpTime > 0)
            hero->posHero.y += jumpPower + gravity;
        clampPosition(hero);
        hero->jumpTime--;
        if (hero->jumpTime <= 0)
            setState(hero, STATE_IDLE);
        return;
    }

    // --- SHOOT ANIMATION ---
    if (hero->state == STATE_SHOOT) {
        hero->shootAnimTimer--;
        if (hero->shootAnimTimer <= 0) {
            if (hero->moveLeft || hero->moveRight || hero->moveUp || hero->moveDown)
                setState(hero, STATE_WALK);
            else
                setState(hero, STATE_IDLE);
        }
        return;
    }

    // --- WALK / IDLE ---
    if (hero->moveLeft || hero->moveRight || hero->moveUp || hero->moveDown) {
        setState(hero, STATE_WALK);
        if (hero->moveLeft)  { hero->posHero.x -= speed; hero->dir = DIR_LEFT;  }
        if (hero->moveRight) { hero->posHero.x += speed; hero->dir = DIR_RIGHT; }
        if (hero->moveUp)    hero->posHero.y -= speed;
        if (hero->moveDown)  hero->posHero.y += speed;
        clampPosition(hero);
    } else {
        setState(hero, STATE_IDLE);
    }
}

// ---------------------------------------------------------------------------
// takeDamage
// ---------------------------------------------------------------------------
void takeDamage(Hero *hero) {
    if (hero->dead) return;
    hero->lives--;
    if (hero->lives <= 0) {
        hero->lives = 0;
        hero->dead  = 1;
        setState(hero, STATE_DEAD);
        return;
    }
    setState(hero, STATE_HURT);
    hero->hurtTimer = 20;
}

// ---------------------------------------------------------------------------
// drawHearts — HUD: player label + heart icons in one row
// ---------------------------------------------------------------------------
void drawHearts(Hero *hero, SDL_Surface *screen, TTF_Font *font) {
    // Each player's block is 70 px tall; player 1 at top-left, player 2 below
    int blockY = 15 + (hero->id - 1) * 70;
    int blockX = 15;

    // "P1" / "P2" label
    char label[4];
    sprintf(label, "P%d", hero->id);

    SDL_Color nameColor = (hero->id == 1)
                          ? (SDL_Color){100, 210, 255, 255}   // cyan-ish for P1
                          : (SDL_Color){255, 160, 80,  255};  // orange for P2

    SDL_Surface *nameSurf = TTF_RenderText_Solid(font, label, nameColor);
    SDL_Rect namePos = {blockX, blockY, 0, 0};
    SDL_BlitSurface(nameSurf, NULL, screen, &namePos);
    SDL_FreeSurface(nameSurf);

    // Hearts — drawn to the right of the label
    int heartStartX = blockX + 40;
    int iconSize    = 28;  // expected icon render size (icon is blitted as-is)
    for (int i = 0; i < hero->lives; i++) {
        SDL_Rect pos = {heartStartX + i * (iconSize + 4), blockY + 2, 0, 0};
        SDL_BlitSurface(hero->healthIcon, NULL, screen, &pos);
    }
}

// ---------------------------------------------------------------------------
// shoot — fires a bullet and triggers the shoot animation
// ---------------------------------------------------------------------------
void shoot(Hero *hero) {
    if (hero->shootCooldown > 0 || hero->dead) return;

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!hero->bullets[i].active) {
            hero->bullets[i].active = 1;
            hero->bullets[i].y      = hero->posHero.y + hero->frameHeight / 2;
            hero->bullets[i].dir    = hero->dir;
            hero->bullets[i].speed  = 10;

            if (hero->dir == DIR_RIGHT)
                hero->bullets[i].x = hero->posHero.x + hero->frameWidth;
            else
                hero->bullets[i].x = hero->posHero.x;

            hero->shootCooldown  = 15;
            setState(hero, STATE_SHOOT);
            hero->shootAnimTimer = hero->frameDelay * hero->shootFrames;
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// updateBullets
// ---------------------------------------------------------------------------
void updateBullets(Hero *hero) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!hero->bullets[i].active) continue;
        if (hero->bullets[i].dir == DIR_RIGHT)
            hero->bullets[i].x += hero->bullets[i].speed;
        else
            hero->bullets[i].x -= hero->bullets[i].speed;
        if (hero->bullets[i].x < 0 || hero->bullets[i].x > SCREEN_WIDTH)
            hero->bullets[i].active = 0;
    }
}

// ---------------------------------------------------------------------------
// drawBullets
// ---------------------------------------------------------------------------
void drawBullets(Hero *hero, SDL_Surface *screen) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!hero->bullets[i].active) continue;
        SDL_Rect rect = {hero->bullets[i].x, hero->bullets[i].y, 10, 5};
        SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 255, 255, 0));
    }
}

// ---------------------------------------------------------------------------
// checkBulletHit — only used for attacker vs enemies (called from main.c)
// ---------------------------------------------------------------------------
void checkBulletHit(Hero *attacker, Hero *target) {
    if (target->dead) return;

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!attacker->bullets[i].active) continue;

        SDL_Rect bullet     = {attacker->bullets[i].x, attacker->bullets[i].y, 10, 5};
        SDL_Rect targetRect = {
            target->posHero.x, target->posHero.y,
            target->frameWidth, target->frameHeight
        };

        if (SDL_HasIntersection(&bullet, &targetRect)) {
            takeDamage(target);
            attacker->bullets[i].active = 0;

            // If attacker killed the target, give kill score
            if (target->dead)
                addKillScore(attacker);
        }
    }
}

// ---------------------------------------------------------------------------
// Score system
// ---------------------------------------------------------------------------

// Call every game tick (assumed ~60 ticks/sec at 16 ms delay)
void updateScore(Hero *hero) {
    if (hero->dead) return;

    hero->scoreTimer++;
    // 60 ticks ≈ 1 second → +5 score per second
    if (hero->scoreTimer >= 60) {
        hero->scoreTimer = 0;
        hero->score += 5;
    }
}

// Call from your colleague's enemy code when this player kills an enemy
void addKillScore(Hero *hero) {
    hero->score += 20;
}

// Draws the score on screen — sits directly below the hearts line
void drawScore(Hero *hero, SDL_Surface *screen, TTF_Font *font) {
    // Same blockY logic as drawHearts; score line is ~28 px below it
    int blockY   = 15 + (hero->id - 1) * 70;
    int scoreY   = blockY + 32;   // below the hearts row
    int blockX   = 15 + 40;       // aligned with the hearts

    char text[32];
    sprintf(text, "Score: %d", hero->score);

    // Slightly smaller / dimmer colour so it doesn't compete with the hearts
    SDL_Color col = (hero->id == 1)
                    ? (SDL_Color){80, 200, 255, 255}
                    : (SDL_Color){255, 140, 50,  255};

    SDL_Surface *label = TTF_RenderText_Solid(font, text, col);
    SDL_Rect pos = {blockX, scoreY, 0, 0};
    SDL_BlitSurface(label, NULL, screen, &pos);
    SDL_FreeSurface(label);
}
