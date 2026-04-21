#include "hero.h"
#include <stdio.h>
#include <string.h>

/* ══════════════════════════════════════════════════════════════════════
 * Internal (static) helpers – never declared in hero.h
 * ══════════════════════════════════════════════════════════════════════ */

static void setState(Hero *hero, State newState)
{
    if (hero->state == newState) return;

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

static void clampPosition(Hero *hero, int worldW, int worldH)
{
    if (hero->posHero.x < 0)
        hero->posHero.x = 0;
    if (hero->posHero.x > worldW - hero->frameWidth)
        hero->posHero.x = worldW - hero->frameWidth;
    if (hero->posHero.y < 0)
        hero->posHero.y = 0;
    if (hero->posHero.y > worldH - hero->frameHeight)
        hero->posHero.y = worldH - hero->frameHeight;
}

static void advanceAnim(Hero *hero)
{
    hero->frameTimer++;
    if (hero->frameTimer < hero->frameDelay) return;
    hero->frameTimer = 0;

    int loop = (hero->state == STATE_IDLE ||
                hero->state == STATE_WALK ||
                hero->state == STATE_DASH ||
                hero->state == STATE_JUMP);

    if (loop) {
        hero->currentFrame = (hero->currentFrame + 1) % hero->maxFrames;
    } else {
        if (hero->currentFrame < hero->maxFrames - 1)
            hero->currentFrame++;
    }
    hero->frameRect.x = hero->currentFrame * hero->frameWidth;
}

/* ══════════════════════════════════════════════════════════════════════
 * Lifecycle
 * ══════════════════════════════════════════════════════════════════════ */

void initializePlayer(Hero *hero, int id, SDL_Renderer *renderer)
{
    memset(hero, 0, sizeof(Hero));

    hero->id          = id;
    hero->frameWidth  = 128;
    hero->frameHeight = 128;

    /* Stagger starting X positions */
    hero->posHero.x = 100 + (id - 1) * 300;
    hero->posHero.y = SCREEN_HEIGHT - hero->frameHeight - 50;
    hero->posHero.w = hero->frameWidth;
    hero->posHero.h = hero->frameHeight;

    /* Load sprite sheets from relative paths (images/ folder) */
    const char *pathR = (id == 1) ? "images/soldierR.png" : "images/C1R.png";
    const char *pathL = (id == 1) ? "images/soldierL.png" : "images/C1L.png";

    SDL_Surface *sr = IMG_Load(pathR);
    SDL_Surface *sl = IMG_Load(pathL);
    hero->spriteSheetRight = sr ? SDL_CreateTextureFromSurface(renderer, sr) : NULL;
    hero->spriteSheetLeft  = sl ? SDL_CreateTextureFromSurface(renderer, sl) : NULL;
    if (sr) SDL_FreeSurface(sr);
    if (sl) SDL_FreeSurface(sl);

    SDL_Surface *hi = IMG_Load("images/Icon12.png");
    hero->healthIcon = hi ? SDL_CreateTextureFromSurface(renderer, hi) : NULL;
    if (hi) SDL_FreeSurface(hi);

    /* Initial animation state */
    hero->dir        = DIR_RIGHT;
    hero->state      = STATE_IDLE;
    hero->maxFrames  = FRAMES_IDLE;
    hero->frameDelay = 4;
    hero->frameRect  = (SDL_Rect){0, ROW_IDLE, 128, 128};

    hero->lives       = MAX_LIVES;
    hero->shootFrames = FRAMES_SHOOT;

    /* Default world boundary = full window */
    hero->worldW = SCREEN_WIDTH;
    hero->worldH = SCREEN_HEIGHT;
}

void freePlayer(Hero *hero)
{
    if (hero->spriteSheetRight) SDL_DestroyTexture(hero->spriteSheetRight);
    if (hero->spriteSheetLeft)  SDL_DestroyTexture(hero->spriteSheetLeft);
    if (hero->healthIcon)       SDL_DestroyTexture(hero->healthIcon);
}

void setHeroBounds(Hero *hero, int w, int h)
{
    hero->worldW = w;
    hero->worldH = h;
    /* Re-clamp position immediately */
    if (hero->posHero.x < 0)                          hero->posHero.x = 0;
    if (hero->posHero.y < 0)                          hero->posHero.y = 0;
    if (hero->posHero.x > w - hero->frameWidth)       hero->posHero.x = w - hero->frameWidth;
    if (hero->posHero.y > h - hero->frameHeight)      hero->posHero.y = h - hero->frameHeight;
}

/* ══════════════════════════════════════════════════════════════════════
 * Per-frame update
 * ══════════════════════════════════════════════════════════════════════ */

void moveHero(Hero *hero)
{
    const int speed  = 5;
    const int worldW = hero->worldW;
    const int worldH = hero->worldH;

    /* Tick cooldowns */
    if (hero->shootCooldown > 0) hero->shootCooldown--;
    if (hero->invTimer      > 0) hero->invTimer--;
    if (hero->invTimer     == 0) hero->invincible = 0;

    /* Dead – freeze on last animation frame */
    if (hero->dead) {
        setState(hero, STATE_DEAD);
        advanceAnim(hero);
        return;
    }

    /* Hurt – slide, then recover */
    if (hero->state == STATE_HURT) {
        hero->hurtTimer--;
        if (hero->hurtTimer <= 0)
            setState(hero, STATE_IDLE);
        advanceAnim(hero);
        return;
    }

    /* Dash – fast horizontal burst */
    if (hero->state == STATE_DASH) {
        const int dashSpeed = 15;
        hero->posHero.x += (hero->dir == DIR_RIGHT) ? dashSpeed : -dashSpeed;
        clampPosition(hero, worldW, worldH);
        hero->dashTime--;
        if (hero->dashTime <= 0)
            setState(hero, STATE_IDLE);
        advanceAnim(hero);
        return;
    }

    /* Jump – parabolic arc */
    if (hero->state == STATE_JUMP) {
        hero->posHero.y += (int)hero->jumpVy;
        hero->jumpVy    += 1.2f;   /* gravity */
        clampPosition(hero, worldW, worldH);
        hero->jumpTime--;
        if (hero->jumpTime <= 0 || hero->posHero.y >= worldH - hero->frameHeight) {
            hero->posHero.y = worldH - hero->frameHeight;
            setState(hero, STATE_IDLE);
        }
        advanceAnim(hero);
        return;
    }

    /* Shoot animation – play out, then return to idle/walk */
    if (hero->state == STATE_SHOOT) {
        hero->shootAnimTimer--;
        if (hero->shootAnimTimer <= 0) {
            if (hero->moveLeft || hero->moveRight || hero->moveUp || hero->moveDown)
                setState(hero, STATE_WALK);
            else
                setState(hero, STATE_IDLE);
        }
        advanceAnim(hero);
        return;
    }

    /* Normal walking / idle */
    int moving = hero->moveLeft || hero->moveRight || hero->moveUp || hero->moveDown;
    if (moving) {
        setState(hero, STATE_WALK);
        if (hero->moveLeft)  { hero->posHero.x -= speed; hero->dir = DIR_LEFT;  }
        if (hero->moveRight) { hero->posHero.x += speed; hero->dir = DIR_RIGHT; }
        if (hero->moveUp)      hero->posHero.y -= speed;
        if (hero->moveDown)    hero->posHero.y += speed;
        clampPosition(hero, worldW, worldH);
    } else {
        setState(hero, STATE_IDLE);
    }

    advanceAnim(hero);
}

/* ══════════════════════════════════════════════════════════════════════
 * Rendering
 * ══════════════════════════════════════════════════════════════════════ */

void showPlayer(SDL_Renderer *renderer, Hero *hero, int camX, int camY)
{
    /* Blink during invincibility frames */
    if (hero->invincible && (hero->invTimer % 6 < 3)) return;

    SDL_Texture *sheet = (hero->dir == DIR_RIGHT)
                         ? hero->spriteSheetRight
                         : hero->spriteSheetLeft;

    SDL_Rect dest = {
        hero->posHero.x - camX,
        hero->posHero.y - camY,
        hero->frameWidth,
        hero->frameHeight
    };

    if (!sheet) {
        /* Coloured rectangle fallback when textures are missing */
        SDL_SetRenderDrawColor(renderer,
            hero->id == 1 ?  80 : 220,
            hero->id == 1 ? 140 :  80,
            hero->id == 1 ? 240 :  80,
            255);
        SDL_RenderFillRect(renderer, &dest);
        return;
    }

    SDL_RenderCopy(renderer, sheet, &hero->frameRect, &dest);
}

void drawBullets(Hero *hero, SDL_Renderer *renderer, int camX, int camY)
{
    if (hero->id == 1)
        SDL_SetRenderDrawColor(renderer, 255, 230,  0, 255);
    else
        SDL_SetRenderDrawColor(renderer, 255, 100, 30, 255);

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!hero->bullets[i].active) continue;
        SDL_Rect r = {
            (int)hero->bullets[i].x - camX,
            (int)hero->bullets[i].y - camY,
            12, 5
        };
        SDL_RenderFillRect(renderer, &r);
    }
}

void drawHUD(Hero *hero, SDL_Renderer *renderer, TTF_Font *font,
             int offsetX, int offsetY)
{
    if (!font) return;

    /* Player label */
    char label[4];
    sprintf(label, "P%d", hero->id);
    SDL_Color nameColor = (hero->id == 1)
        ? (SDL_Color){100, 210, 255, 255}
        : (SDL_Color){255, 160,  80, 255};

    SDL_Surface *ns = TTF_RenderText_Solid(font, label, nameColor);
    if (ns) {
        SDL_Texture *nt = SDL_CreateTextureFromSurface(renderer, ns);
        SDL_Rect np = {offsetX, offsetY, ns->w, ns->h};
        SDL_RenderCopy(renderer, nt, NULL, &np);
        SDL_FreeSurface(ns);
        SDL_DestroyTexture(nt);
    }

    /* Life icons */
    int iconSize    = 24;
    int iconPadding =  4;
    int heartX      = offsetX + 40;
    int heartY      = offsetY +  2;

    for (int i = 0; i < hero->lives; i++) {
        SDL_Rect pos = {heartX + i * (iconSize + iconPadding), heartY, iconSize, iconSize};
        if (hero->healthIcon) {
            SDL_RenderCopy(renderer, hero->healthIcon, NULL, &pos);
        } else {
            SDL_SetRenderDrawColor(renderer, 220, 40, 40, 255);
            SDL_RenderFillRect(renderer, &pos);
        }
    }

    /* Score */
    char scoreText[32];
    sprintf(scoreText, "Score: %d", hero->score);
    SDL_Color sc = (hero->id == 1)
        ? (SDL_Color){ 80, 200, 255, 255}
        : (SDL_Color){255, 140,  50, 255};

    SDL_Surface *ss = TTF_RenderText_Solid(font, scoreText, sc);
    if (ss) {
        SDL_Texture *st = SDL_CreateTextureFromSurface(renderer, ss);
        SDL_Rect sp = {offsetX, offsetY + 30, ss->w, ss->h};
        SDL_RenderCopy(renderer, st, NULL, &sp);
        SDL_FreeSurface(ss);
        SDL_DestroyTexture(st);
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * Bullets
 * ══════════════════════════════════════════════════════════════════════ */

void shoot(Hero *hero)
{
    if (hero->shootCooldown > 0 || hero->dead) return;

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!hero->bullets[i].active) {
            Bullet *b = &hero->bullets[i];
            b->active = 1;
            b->y      = hero->posHero.y + hero->frameHeight / 2.0f - 2.5f;
            b->dir    = hero->dir;
            b->vx     = (hero->dir == DIR_RIGHT) ?  12.0f : -12.0f;
            b->x      = (hero->dir == DIR_RIGHT)
                        ? (float)(hero->posHero.x + hero->frameWidth)
                        : (float)(hero->posHero.x - 10);

            hero->shootCooldown  = 15;
            setState(hero, STATE_SHOOT);
            hero->shootAnimTimer = hero->frameDelay * hero->shootFrames;
            break;
        }
    }
}

void updateBullets(Hero *hero)
{
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!hero->bullets[i].active) continue;
        hero->bullets[i].x += hero->bullets[i].vx;
        if (hero->bullets[i].x < -50.0f ||
            hero->bullets[i].x > (float)(hero->worldW + 50))
            hero->bullets[i].active = 0;
    }
}

void checkBulletHit(Hero *attacker, Hero *target)
{
    if (target->dead || target->invincible) return;

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!attacker->bullets[i].active) continue;

        SDL_Rect bullet = {
            (int)attacker->bullets[i].x,
            (int)attacker->bullets[i].y,
            12, 5
        };
        SDL_Rect tRect = {
            target->posHero.x + 16,
            target->posHero.y + 16,
            target->frameWidth  - 32,
            target->frameHeight - 24
        };

        if (SDL_HasIntersection(&bullet, &tRect)) {
            attacker->bullets[i].active = 0;
            takeDamage(target);
            if (target->dead)
                addKillScore(attacker);
        }
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * Damage
 * ══════════════════════════════════════════════════════════════════════ */

void takeDamage(Hero *hero)
{
    if (hero->dead || hero->invincible) return;

    hero->lives--;
    if (hero->lives <= 0) {
        hero->lives = 0;
        hero->dead  = 1;
        setState(hero, STATE_DEAD);
        return;
    }
    setState(hero, STATE_HURT);
    hero->hurtTimer  = 20;
    hero->invincible = 1;
    hero->invTimer   = 60;   /* ~1 second at 60 fps */
}

/* ══════════════════════════════════════════════════════════════════════
 * Score
 * ══════════════════════════════════════════════════════════════════════ */

void updateScore(Hero *hero)
{
    if (hero->dead) return;
    hero->scoreTimer++;
    if (hero->scoreTimer >= 60) {
        hero->scoreTimer = 0;
        hero->score     += 5;
    }
}

void addKillScore(Hero *hero)
{
    hero->score += 20;
}
