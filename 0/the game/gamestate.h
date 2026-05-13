#ifndef GAMESTATE_H
#define GAMESTATE_H

typedef enum {
    STATE_MAIN_MENU   = 0,
    STATE_OPTIONS     = 1,
    STATE_SAVE_LOAD   = 2,
    STATE_PLAYING     = 3,
    STATE_HIGHSCORE   = 4,   /* high-score / name-input screen     */
    STATE_DEAD_SCREEN = 5,   /* "YOU ARE DEAD" interstitial        */
    STATE_EXIT        = 6,
    STATE_CHAR_SELECT = 7    /* character selection before new game */
} AppState;

#endif /* GAMESTATE_H */
