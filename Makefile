all:
	gcc main.c -I./SDL2/include/SDL2 -L./SDL2/lib -lmingw32 -lSDL2main -lSDL2 -o memory_card_game