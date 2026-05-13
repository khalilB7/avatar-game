#ifndef SAVESYSTEM_H
#define SAVESYSTEM_H

/*
 * savesystem.h  –  Game-state serialisation (save / load).
 *
 * Saves to a plain binary file.  All fields are stored as fixed-width
 * integers so the file is trivially portable on the same machine.
 *
 * Saved data
 * ──────────
 *   • magic + version  (corruption guard)
 *   • number of active players
 *   • per-player: position, lives, score, direction, dead flag
 *   • elapsed game time in seconds
 *   • active enemy states (position + health)
 *
 * After a successful save the best score is also appended to the
 * high-score file (players.txt) so the leaderboard updates automatically.
 */

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "hero.h"
#include "enemy.h"

#define SAVE_MAGIC    0x41564147   /* "AVAG" */
#define SAVE_VERSION  3            /* bumped: added playerName field */
#define SAVE_FILE     "savegame.bin"

#define SAVE_MAX_ENEMIES 16   /* must match NUM_ENEMIES in main.c */

/* ── Per-player snapshot ─────────────────────────────────────────────── */
typedef struct {
    int  x, y;
    int  lives;
    int  score;
    int  dir;            /* 0=left, 1=right */
    int  dead;
    char playerName[32]; /* "Soldier" or "Warrior" */
} SavedPlayer;

/* ── Per-enemy snapshot ──────────────────────────────────────────────── */
typedef struct {
    int x, y;
    int health;
    int active;
} SavedEnemy;

/* ── Full save record ────────────────────────────────────────────────── */
typedef struct {
    Uint32 magic;
    Uint32 version;
    Uint32 elapsedSeconds;
    int    numPlayers;
    SavedPlayer players[2];
    int         numEnemies;
    SavedEnemy  enemies[SAVE_MAX_ENEMIES];
} SaveRecord;

/*
 * Save the current game state.
 * Also appends the best player score to `scoreFile` (may be NULL → skipped).
 * Returns true on success.
 */
bool SaveSystem_Save(const Hero    *players,   int numPlayers,
                     const Enemy   *enemies,   int numEnemies,
                     Uint32         elapsedSeconds,
                     const char    *saveFile,
                     const char    *scoreFile);

/*
 * Load a previously saved state.
 * Fills `players` (up to numPlayers) and `enemies` (up to numEnemies).
 * Caller must have already called initializePlayer / initEnemy so the
 * SDL textures exist; this function only patches the numeric fields.
 * Returns true on success.
 */
bool SaveSystem_Load(Hero    *players,   int *numPlayers,
                     Enemy   *enemies,   int numEnemies,
                     Uint32  *elapsedSeconds,
                     const char *saveFile);

/* Returns true if a valid save file exists. */
bool SaveSystem_Exists(const char *saveFile);

/* ── Save-slot metadata (used by SaveMenu) ───────────────────────────── */
typedef struct {
    bool   exists;
    char   name[64];           /* e.g. "Slot 1 - 2 players"            */
    char   playerName[32];     /* "Soldier" or "Warrior"               */
    int    numPlayers;
    int    score;              /* best score stored in this slot       */
    Uint32 elapsedSeconds;     /* survival time stored in this slot    */
} SaveSlotInfo;

/*
 * Fill `slots[SAVE_SLOT_COUNT]` with metadata for each save slot.
 * Slots that have no file on disk will have exists=false.
 */
#define SAVE_SLOT_COUNT 5
void SaveSystem_QuerySlots(SaveSlotInfo *slots);

#endif /* SAVESYSTEM_H */
