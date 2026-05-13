# ─────────────────────────────────────────────────────────────────────────────
# Makefile  –  2-Player Arena
# ─────────────────────────────────────────────────────────────────────────────

CC      = gcc
TARGET  = game

# SDL2 flags (sdl2-config handles include + lib paths automatically)
SDL_CFLAGS := $(shell sdl2-config --cflags)
SDL_LIBS   := $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf -lSDL2_mixer

CFLAGS  = -Wall -Wextra -O2 -I/usr/include/SDL2 -D_REENTRANT $(SDL_CFLAGS)
LIBS    = $(SDL_LIBS) -lm

# ─── Source files ─────────────────────────────────────────────────────────────
SRCS =  main.c          \
        mainmenu.c      \
        optionsmenu.c   \
        savemenu.c      \
        highscore.c     \
        hero.c          \
        enemy.c         \
        game.c          \
        partage.c       \
        minimap.c       \
        pausemenu.c     \
        savesystem.c    \
        charselect.c

OBJS = $(SRCS:.c=.o)

# ─── Rules ────────────────────────────────────────────────────────────────────
.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# ─── Explicit header dependencies ─────────────────────────────────────────────
main.o:        main.c        gamestate.h mainmenu.h optionsmenu.h savemenu.h \
               highscore.h   hero.h enemy.h game.h partage.h minimap.h       \
               pausemenu.h   savesystem.h charselect.h
mainmenu.o:    mainmenu.c    mainmenu.h   gamestate.h
optionsmenu.o: optionsmenu.c optionsmenu.h gamestate.h
savemenu.o:    savemenu.c    savemenu.h   gamestate.h
highscore.o:   highscore.c   highscore.h  gamestate.h
hero.o:        hero.c        hero.h
enemy.o:       enemy.c       enemy.h      minimap.h
game.o:        game.c        game.h
partage.o:     partage.c     partage.h    hero.h
minimap.o:     minimap.c     minimap.h
pausemenu.o:   pausemenu.c   pausemenu.h
savesystem.o:  savesystem.c  savesystem.h hero.h enemy.h
charselect.o:  charselect.c  charselect.h gamestate.h

clean:
	rm -f $(OBJS) $(TARGET)
