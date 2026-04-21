CC = gcc
CFLAGS = `sdl2-config --cflags` -lSDL2_image -lSDL2_ttf -Wall
LDFLAGS = `sdl2-config --libs` -lSDL2_image -lSDL2_ttf

SRC = main.c hero.c
OBJ = $(SRC:.c=.o)
EXEC = game

# To build the final executable
$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(LDFLAGS)

# To compile .c files to .o object files
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

# Clean the project (remove object files and executable)
clean:
	rm -f $(OBJ) $(EXEC)
