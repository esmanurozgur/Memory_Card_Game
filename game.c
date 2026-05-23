#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "game.h"


void initGame(GameState *game) {
    int cards[ROWS * COLS]; 
    
    for (int i = 0; i < (ROWS * COLS) / 2; i++) {
        cards[i * 2] = i + 1;
        cards[i * 2 + 1] = i + 1;
    } //cards[]= {1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8} 

    srand(time(NULL)); //rastgele sayi uretmek icin

    for (int i = (ROWS * COLS) - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = cards[i];
        cards[i] = cards[j];
        cards[j] = temp;
    }//cards[] artik rastgele siralanmis durumda
    //15. elemandan baslayarak her eleman icin rastgele bir index secilir ve o indexteki degerle yer degistirilir

    int index = 0;
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            game->board[r][c].val = cards[index++];
            game->board[r][c].state = 0; //tum kartlar baslangicta kapali
        }
    }//cards[] dizisindeki degerler gameBoard[][] dizisine aktarildi

    game->moves = 0;
    game->flippedCount = 0;
    game->firstRow = -1;
    game->firstCol = -1;
    game->secondRow = -1;
    game->secondCol = -1;
    game->timeRemaining = 60; //oyun baslangicinda 60 saniye
    game->isGameOver = 0; //oyun baslangicinda oyun bitmedi olarak isaretlenir
}// initGame fonksiyonu GameState yapisini alir ve oyun tahtasini rastgele degerlerle doldurur ve oyun durumunu baslangicta sifirlar

void printBoardToConsole(GameState *game) {
    printf("--- OYUN TAHTASI (ARKA PLAN) ---\n");
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            printf("%d\t", game->board[r][c].val);
        }
        printf("\n"); 
    }
    printf("--------------------------------\n");
}// printBoardToConsole fonksiyonu GameState yapisini alir ve oyun tahtasini konsola yazdirir