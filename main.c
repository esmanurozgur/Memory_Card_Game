#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>

#define ROWS 4
#define COLS 4

int gameBoard[ROWS][COLS]; // oyun tahtasini temsil eden 2 boyutlu dizi

void initBoard() {
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
            gameBoard[r][c] = cards[index++];
        }
    }//cards[] dizisindeki degerler gameBoard[][] dizisine aktarildi
}

void printBoardToConsole() {
    printf("--- OYUN TAHTASI (ARKA PLAN) ---\n");
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            printf("%d\t", gameBoard[r][c]);
        }
        printf("\n"); 
    }
    printf("--------------------------------\n");
}// bu fonk ile gameboard[][] konsala yazdirilir

int main(int argc, char *argv[])
 {
    //argc arg. sayisi, argv arg. degerlerini string olarak tutan dizi

    initBoard();
    printBoardToConsole(); 

    //SDL baslatmak icin SDL_Init() fonksiyonu
    //acilirsa 0 doner, hata olursa negatif deger doner

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL Baslatilamadi! Hata: %s\n", SDL_GetError());
        return 1;
    }

    //SDL ile pencere olusturmak icin SDL_CreateWindow() fonksiyonu
    //parametreler: baslik, x konumu, y konumu, genislik, yukseklik, pencere gorunurlugu
    // *window basarili olursa pencereye isaret eden pointer, hata olursa NULL doner

    SDL_Window *window = SDL_CreateWindow("Memory Card Game", 
                                          SDL_WINDOWPOS_CENTERED, 
                                          SDL_WINDOWPOS_CENTERED, 
                                          800, 600, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Pencere olusturulamadi! Hata: %s\n", SDL_GetError()); //SDL_GetError() fonksiyonu en son olusan hatayi string olarak dondurur
        SDL_Quit(); // SDL_Init() ile baslatilan subsistemleri kapatir
        return 1;
    }




    // SDL_Renderer, pencereye cizim yapmamizi saglayan bir yapidir. SDL_CreateRenderer() fonksiyonu ile olusturulur
    // parametreler: pencere pointeri, render driver indexi (-1 genellikle varsayilan), render ozellikleri
    // SDL_RENDERER_ACCELERATED, donusturucu destekliyorsa donusturucuyu kullanarak cizim yapacaktir
    // basariliysa renderer pointeri, hata olursa NULL doner

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer olusturulamadi! Hata: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    
    int isRunning = 1; //oyunun calismaya devam edip etmeyecegni kontrol eden yapidir, 1 ise calisiyor, 0 ise duracak
    SDL_Event event; // klavye, fare veya pencere olaylarini (event) tutar

    
    while (isRunning) {
        
        
        while (SDL_PollEvent(&event))
        {
            
            if (event.type == SDL_QUIT) {
                isRunning = 0; 
            }
            //event.type, kullanicinin ne tur bir eylemde bulundugunu belirtir, SDL_QUIT ise pencerenin kapatilmasi anlamina gelir
            // Eger event.type SDL_QUIT ise, isRunning 0 yapilir ve oyun dongusu kirilir, bu da oyunun kapanmasini saglar


        } //SDL_PollEvent(&event) ile eventleri kontrol ederiz


        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // rgba degeriyle rengimiz siyah ve tam opak 
        SDL_RenderClear(renderer); //renderirin tamamini temizler ve setRenderDrawColor() ile belirlenen renk ile doldurur

        
        int cardWidth = 100;  // Kart genisligi
        int cardHeight = 100; // Kart yuksekligi
        int padding = 20;     // Kartlar arasi bosluk
        
        // Ekranin (800x600) tam ortasina hizalamak icin baslangic noktalari secerken
        //4x4 lük sistemimizde 4 kart genisligi ve 3 tane de padding var
        //ekrangenisligi - (4*kartgenisligi + 3*padding) formulunu x'in baslangic noktasini bulmak icin kullaniriz
        //ekran yuksekligi - (4*kartyuksekligi + 3*padding) formulunu y'nin baslangic noktasini bulmak icin kullaniriz
        
        int startX = 170; // 800 - (4*100 + 3*20) = 800 - 460 = 170
        int startY = 70;  // 600 - (4*100 + 3*20) = 600 - 460 = 70

        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                // SDL_Rect, bir dikdortgenin nerede cizilecegini (x,y) koordinatlarini ve boyutunu (w,h) weight height tutan bir yapidir
                SDL_Rect rect;

                rect.x = startX + c * (cardWidth + padding); //170 +sutun*120

                rect.y = startY + r * (cardHeight + padding); //70 + satir*120

                rect.w = cardWidth;

                rect.h = cardHeight;

                
                SDL_SetRenderDrawColor(renderer, 118,206,242,1); // Kartlar icin rgba rengi belirledim

                SDL_RenderFillRect(renderer, &rect);
                // Dikdortgenin icini boyayarak ekrana cizdirir
                // SDL_RenderFillRect() fonksiyonu parametre olarak renderer pointeri ve cizilecek dikdortgenin koordinatlarini tutan
                // SDL_Rect yapisini alir
            }
        }

        SDL_RenderPresent(renderer);
        // Renderer'in cizimlerini ekrana yansitir, bu fonksiyon cagrilmazsa yaptigimiz cizimler ekranda gorunmez

    }// Oyun dongusu burada bitiyor, kullanici pencereyi kapatana kadar kartlar ekranda kalacak

    // Oyun bittiginde hafizayi temizlememiz lazim
    SDL_DestroyRenderer(renderer);// Renderer'i yok et
    SDL_DestroyWindow(window);// Pencereyi yok et
    SDL_Quit();// SDL subsistemlerini kapat

    return 0;
}