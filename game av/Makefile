CC = gcc
CFLAGS = `sdl2-config --cflags` -lSDL2_image
LDFLAGS = `sdl2-config --libs` -lSDL2_image

SRC = main.c hero.c
OBJ = $(SRC:.c=.o)
EXEC = game

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)
