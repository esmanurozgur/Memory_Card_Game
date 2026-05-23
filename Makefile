all:
	gcc main.c game.c -I./SDL2/include/SDL2 -L./SDL2/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -o game