CC     = gcc
CFLAGS = -Wall -Wextra -O2 $(shell sdl2-config --cflags)
LIBS   = $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf -lm

TARGET = game
SRCS   = main.c hero.c game.c partage.c minimap.c enemy.c
OBJS   = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Header dependencies
main.o:    main.c    hero.h enemy.h game.h partage.h minimap.h
hero.o:    hero.c    hero.h
game.o:    game.c    game.h
partage.o: partage.c partage.h hero.h
minimap.o: minimap.c minimap.h
enemy.o:   enemy.c   enemy.h minimap.h

clean:
	rm -f $(OBJS) $(TARGET)
