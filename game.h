#ifndef GAME_H
#define GAME_H

#define ROWS 4
#define COLS 4

typedef struct {
    int val; 
    int state; //0: kapali, 1: acik, 2: eslesmis
} Card; // Card yapisi, her kartin degerini ve durumunu tutar

typedef struct {
    Card board[ROWS][COLS];
    int moves;
    int flippedCount;
    int firstRow, firstCol;
    int secondRow, secondCol;
    int timeRemaining;
    int isGameOver;
    int matchedPairs;
} GameState; // GameState yapisi, oyun tahtasini ve oyun durumunu tutar

void initGame(GameState *game);
void printBoardToConsole(GameState *game);
 
#endif // GAME_H