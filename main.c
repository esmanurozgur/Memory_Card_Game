#include <stdio.h>
#include <stdlib.h> 
#include <time.h>  
#include <SDL.h>

//oyun 4x4'lük bir matris olacak, yani 16 kart olacak.
#define ROWS 4
#define COLS 4

int gameBoard[ROWS][COLS]; 

// kartlari karistirip oyun tahtasini hazirlayan fonksiyon
void initBoard() 
{
    int cards[ROWS * COLS]; //kartlari tek boyutlu bir diziye ekleyelim
    
    // 1. Adim: 1'den 8'e kadar olan sayilari ciftler halinde diziye ekle
    for (int i = 0; i < (ROWS * COLS) / 2; i++) {
        cards[i * 2] = i + 1;
        cards[i * 2 + 1] = i + 1;
    } // donguyu calistirdigimda cards[]= {1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8}

    srand(time(NULL)); // her seferinde farkli bir karisma olmasi icin rastgele sayilar uretmek icin srand() fonksiyonu

    for (int i = (ROWS * COLS) - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = cards[i];
        cards[i] = cards[j];
        cards[j] = temp;
    } //dizinin sonundan baslayip rand() kullanarak farkli bir elemanla yerlerini degistiriyoruz


    int index = 0;
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            gameBoard[r][c] = cards[index++];
        }
    } // karistirilmis kartlari 4x4'lük oyun tahtasina yerlestiriyoruz
}


void printBoardToConsole()
{
    printf("--- OYUN TAHTASI (ARKA PLAN MANTIGI) ---\n");
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            printf("%d\t", gameBoard[r][c]);
        }
        printf("\n"); 
    }
    printf("----------------------------------------\n");
} // oyun tahtasini konsola yazdiran fonksiyon

int main(int argc, char *argv[]) {

    //argc argumen sayisi
    //argv argumenlerin string dizisi halinde tutuldugu parametre

    initBoard();
    printBoardToConsole(); 

    
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL Baslatilamadi! Hata: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Memory Card Game", 
                                          SDL_WINDOWPOS_CENTERED, 
                                          SDL_WINDOWPOS_CENTERED, 
                                          800, 600, SDL_WINDOW_SHOWN);//baslik, x ve y konumu, genislik, yukseklik, pencere gorunurlugu
    if (window == NULL) {
        printf("Pencere olusturulamadi! Hata: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Delay(3000); //3000 milisaniye (3 saniye) bekleyerek pencerenin gorunmesini sagliyor

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}