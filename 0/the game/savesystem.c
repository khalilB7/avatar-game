/*
 * savesystem.c  –  Game-state serialisation (save / load).
 */

#include "savesystem.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ── internal helpers ────────────────────────────────────────────────── */

static void append_highscore(const char *scoreFile,
                              const Hero *players, int numPlayers)
{
    if (!scoreFile) return;

    /* Find best score */
    int best = 0;
    const char *bestName = "Player";
    for (int i = 0; i < numPlayers; i++) {
        if (players[i].score > best) {
            best = players[i].score;
            bestName = (i == 0) ? "Player1" : "Player2";
        }
    }
    if (best <= 0) return;

    FILE *f = fopen(scoreFile, "a");
    if (f) {
        fprintf(f, "%s %d\n", bestName, best);
        fclose(f);
    }
}

/* ── public API ──────────────────────────────────────────────────────── */

bool SaveSystem_Save(const Hero    *players,   int numPlayers,
                     const Enemy   *enemies,   int numEnemies,
                     Uint32         elapsedSeconds,
                     const char    *saveFile,
                     const char    *scoreFile)
{
    if (!saveFile) saveFile = SAVE_FILE;

    SaveRecord rec;
    memset(&rec, 0, sizeof(rec));

    rec.magic          = SAVE_MAGIC;
    rec.version        = SAVE_VERSION;
    rec.elapsedSeconds = elapsedSeconds;
    rec.numPlayers     = (numPlayers > 2) ? 2 : numPlayers;

    for (int i = 0; i < rec.numPlayers; i++) {
        rec.players[i].x     = players[i].posHero.x;
        rec.players[i].y     = players[i].posHero.y;
        rec.players[i].lives = players[i].lives;
        rec.players[i].score = players[i].score;
        rec.players[i].dir   = (int)players[i].dir;
        rec.players[i].dead  = players[i].dead;
        /* player name derived from their id (1=Soldier, 2=Warrior) */
        const char *pname = (players[i].id == 2) ? "Warrior" : "Soldier";
        strncpy(rec.players[i].playerName, pname,
                sizeof(rec.players[i].playerName) - 1);
        rec.players[i].playerName[sizeof(rec.players[i].playerName)-1] = '\0';
    }

    int eCount = (numEnemies > SAVE_MAX_ENEMIES) ? SAVE_MAX_ENEMIES : numEnemies;
    rec.numEnemies = eCount;
    for (int i = 0; i < eCount; i++) {
        rec.enemies[i].x      = enemies[i].x;
        rec.enemies[i].y      = enemies[i].y;
        rec.enemies[i].health = enemies[i].health;
        rec.enemies[i].active = enemies[i].active;
    }

    FILE *f = fopen(saveFile, "wb");
    if (!f) {
        fprintf(stderr, "[SaveSystem] Cannot open %s for writing\n", saveFile);
        return false;
    }
    size_t written = fwrite(&rec, sizeof(rec), 1, f);
    fclose(f);

    if (written != 1) {
        fprintf(stderr, "[SaveSystem] Write error\n");
        return false;
    }

    /* Update high-score file automatically */
    append_highscore(scoreFile, players, numPlayers);

    fprintf(stderr, "[SaveSystem] Saved OK → %s\n", saveFile);
    return true;
}

bool SaveSystem_Load(Hero    *players,   int *numPlayers,
                     Enemy   *enemies,   int numEnemies,
                     Uint32  *elapsedSeconds,
                     const char *saveFile)
{
    if (!saveFile) saveFile = SAVE_FILE;

    FILE *f = fopen(saveFile, "rb");
    if (!f) {
        fprintf(stderr, "[SaveSystem] No save file: %s\n", saveFile);
        return false;
    }

    SaveRecord rec;
    size_t got = fread(&rec, sizeof(rec), 1, f);
    fclose(f);

    if (got != 1) {
        fprintf(stderr, "[SaveSystem] Read error\n");
        return false;
    }
    if (rec.magic != SAVE_MAGIC || rec.version != SAVE_VERSION) {
        fprintf(stderr, "[SaveSystem] Corrupt or incompatible save file\n");
        return false;
    }

    *elapsedSeconds = rec.elapsedSeconds;

    /* Patch player state (textures must already exist) */
    int pCount = rec.numPlayers;
    if (pCount > 2) pCount = 2;
    *numPlayers = pCount;

    for (int i = 0; i < pCount; i++) {
        players[i].posHero.x = rec.players[i].x;
        players[i].posHero.y = rec.players[i].y;
        players[i].lives     = rec.players[i].lives;
        players[i].score     = rec.players[i].score;
        players[i].dir       = (Direction)rec.players[i].dir;
        players[i].dead      = rec.players[i].dead;
    }

    /* Patch enemy state */
    int eCount = rec.numEnemies;
    if (eCount > numEnemies)  eCount = numEnemies;
    if (eCount > SAVE_MAX_ENEMIES) eCount = SAVE_MAX_ENEMIES;
    for (int i = 0; i < eCount; i++) {
        enemies[i].x      = rec.enemies[i].x;
        enemies[i].y      = rec.enemies[i].y;
        enemies[i].health = rec.enemies[i].health;
        enemies[i].active = rec.enemies[i].active;
    }

    fprintf(stderr, "[SaveSystem] Loaded OK ← %s\n", saveFile);
    return true;
}

bool SaveSystem_Exists(const char *saveFile)
{
    if (!saveFile) saveFile = SAVE_FILE;
    FILE *f = fopen(saveFile, "rb");
    if (f) { fclose(f); return true; }
    return false;
}

/*
 * Build per-slot metadata for SaveMenu.
 * We use one save file per slot: savegame0.bin … savegame4.bin.
 * Slot 0 also checks the legacy "savegame.bin" name so existing saves
 * are not lost.
 */
void SaveSystem_QuerySlots(SaveSlotInfo *slots)
{
    static const char *slotFiles[SAVE_SLOT_COUNT] = {
        SAVE_FILE,          /* slot 0 → legacy file */
        "savegame1.bin",
        "savegame2.bin",
        "savegame3.bin",
        "savegame4.bin",
    };

    for (int i = 0; i < SAVE_SLOT_COUNT; i++) {
        slots[i].exists          = false;
        slots[i].name[0]         = '\0';
        slots[i].score           = 0;
        slots[i].elapsedSeconds  = 0;

        FILE *f = fopen(slotFiles[i], "rb");
        if (!f) continue;

        SaveRecord rec;
        size_t got = fread(&rec, sizeof(rec), 1, f);
        fclose(f);

        if (got != 1 || rec.magic != SAVE_MAGIC || rec.version != SAVE_VERSION)
            continue;

        slots[i].exists         = true;
        slots[i].elapsedSeconds = rec.elapsedSeconds;
        slots[i].numPlayers     = rec.numPlayers;

        /* Best score among saved players; grab name from best scorer */
        int best = 0;
        int bestIdx = 0;
        for (int p = 0; p < rec.numPlayers && p < 2; p++) {
            if (rec.players[p].score > best) {
                best    = rec.players[p].score;
                bestIdx = p;
            }
        }
        slots[i].score = best;

        /* Player name: use best scorer's stored name */
        if (rec.players[bestIdx].playerName[0])
            strncpy(slots[i].playerName, rec.players[bestIdx].playerName,
                    sizeof(slots[i].playerName) - 1);
        else
            strncpy(slots[i].playerName,
                    (bestIdx == 1) ? "Warrior" : "Soldier",
                    sizeof(slots[i].playerName) - 1);
        slots[i].playerName[sizeof(slots[i].playerName)-1] = '\0';

        snprintf(slots[i].name, sizeof(slots[i].name),
                 "Slot %d", i + 1);
    }
}
