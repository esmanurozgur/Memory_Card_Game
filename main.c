#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>
#include <SDL_image.h>
#include "game.h"



//resmi SDL'in anlayacağı bir formata ceviren fonksiyon
//parametreler: resmin dosya yolu ve render yapacagimiz renderer pointeri

SDL_Texture *LoadTexture(const char *path, SDL_Renderer *renderer)
{
    SDL_Surface* loadedSurface = IMG_Load(path);//IMG_Load() path ile resmi yukleyip bir SDL_Surface pointeri dondurur, hata olursa NULL dondurur
    if (loadedSurface == NULL) {
        printf("Resim yuklenemedi! Hata: %s\n", IMG_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, loadedSurface); //yuklenen resmi rendererla uygun formata cevirir
    SDL_FreeSurface(loadedSurface); //surface'i serbest birakiyoruz cunku artik texture olarak kullanacagiz
    return texture;
}



int main(int argc, char *argv[])
 {
    //argc arg. sayisi, argv arg. degerlerini string olarak tutan dizi

    GameState game;
    initGame(&game);
    printBoardToConsole(&game);

    //SDL baslatmak icin SDL_Init() fonksiyonu
    //acilirsa 0 doner, hata olursa negatif deger doner

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL Baslatilamadi! Hata: %s\n", SDL_GetError());
        return 1;
    }

    
    int imgFlags = IMG_INIT_PNG; // PNG formatinda resimleri yuklemek istedigimizi belirtiyoruz
    //IMG_Init() fonksiyonu imgFlags ile belirtilen formatlari destekleyip desteklemedigini kontrol eder destekliyorsa o formatlari baslatir
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image baslatilamadi! Hata: %s\n", IMG_GetError());
        SDL_Quit();
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


    SDL_Texture *cardTexture[9]; // 1'den 8'e kadar kartlar ve kapali kart icin toplam 9 texture
    cardTexture[1]= LoadTexture("assets/c.png", renderer);
    cardTexture[2]= LoadTexture("assets/cpp.png", renderer);
    cardTexture[3]= LoadTexture("assets/java.png", renderer);
    cardTexture[4]= LoadTexture("assets/python.png", renderer);
    cardTexture[5]= LoadTexture("assets/js.png", renderer);
    cardTexture[6]= LoadTexture("assets/rush.png", renderer);
    cardTexture[7]= LoadTexture("assets/php.png", renderer);
    cardTexture[8]= LoadTexture("assets/swift.png", renderer);
    
    int isRunning = 1; //oyunun calismaya devam edip etmeyecegni kontrol eden yapidir, 1 ise calisiyor, 0 ise duracak
    SDL_Event event; // klavye, fare veya pencere olaylarini (event) tutar

    int cardWidth = 100;  // Kart genisligi
    int cardHeight = 100; // Kart yuksekligi
    int padding = 20;     // Kartlar arasi bosluk



    // Ekranin (800x600) tam ortasina hizalamak icin baslangic noktalari secerken
    //4x4 lük sistemimizde 4 kart genisligi ve 3 tane de padding var
    //ekrangenisligi - (4*kartgenisligi + 3*padding) formulunu x'in baslangic noktasini bulmak icin kullaniriz
    //ekran yuksekligi - (4*kartyuksekligi + 3*padding) formulunu y'nin baslangic noktasini bulmak icin kullaniriz
    int startX = 170; // 800 - (4*100 + 3*20) = 800 - 460 = 170
    int startY = 70;  // 600 - (4*100 + 3*20) = 600 - 460 = 70



    Uint32 startTime = SDL_GetTicks(); //oyuna basladıgımız anı milisaniye cinsinden verir

    while (isRunning)
    {
        if(game.isGameOver == 0) {
            Uint32 currentTime = SDL_GetTicks(); //suanki zamani al
            Uint32 elapsedTime = (currentTime - startTime) / 1000; //gecen zamani saniye cinsine cevir
            game.timeRemaining = 60 - elapsedTime; //kalan zamani hesapla

            if(game.timeRemaining <= 0) {
                game.isGameOver = 1; //zaman doldu, oyun bitmis
                printf("Zaman doldu! Oyun bitti.\n");
            }

            char titleBuffer[100];
            //sprintf() fonksiyonu titleBuffer'a formatli bir string yazmamizi saglar,
            // sizeof(titleBuffer) ile bufferin boyutunu belirtiyoruz, 
            //geri kalan parametreler format stringine gore degisir    
            snprintf(titleBuffer, sizeof(titleBuffer), "Memory Card Game - Sure: %d saniye | Hamle: %d", game.timeRemaining, game.moves);
            SDL_SetWindowTitle(window, titleBuffer);//SDL_SetWindowTitle() fonksiyonu pencerenin basligini degistirir, titleBuffer'daki stringi baslik olarak kullaniriz
        }

        else{
            SDL_SetWindowTitle(window, "SURE BITTI! - Yeniden baslatmak icin kapatin"); //oyun bittiginde pencere basligini degistiriyoruz
        }

        while (SDL_PollEvent(&event))
        {
            
            if (event.type == SDL_QUIT) {
                isRunning = 0; 
            }
            //event.type, kullanicinin ne tur bir eylemde bulundugunu belirtir, SDL_QUIT ise pencerenin kapatilmasi anlamina gelir
            // Eger event.type SDL_QUIT ise, isRunning 0 yapilir ve oyun dongusu kirilir, bu da oyunun kapanmasini saglar

            else if(event.type == SDL_MOUSEBUTTONDOWN) {

                if(game.isGameOver == 0){
                    int mouseX = event.button.x; //fare tiklamasinin x koordinati
                    int mouseY = event.button.y; //fare tiklamasinin y koordinati

                    //tiklanan kartin hangi satir ve sutunda oldugunu bulmak icin
                    int col = (mouseX - startX) / (cardWidth + padding);
                    int row = (mouseY - startY) / (cardHeight + padding);

                    //tiklanan yer gecerli bi satir ve sutunda mi kontrolu
                    if (col >= 0 && col < COLS && row >= 0 && row < ROWS) {

                         if(game.board[row][col].state == 0 && game.flippedCount < 2) { //kart kapaliysa ve 2 karttan az acik varsa
                            game.board[row][col].state = 1; //kart acilir
                         }

                        if(game.flippedCount == 0) {
                            //acilan ilk kartin konumunu kaydettim
                            game.firstRow = row;
                            game.firstCol = col;
                            game.flippedCount = 1; //bir kart acildi
                         } 
                        
                        else if(game.flippedCount == 1) {
                            //acilan ikinci kartin konumunu kaydettim
                            game.secondRow = row;
                            game.secondCol = col;
                            game.flippedCount = 2; //iki kart acildi
                            game.moves++; //kullanici bir hamle yapti, moves sayisini arttir
                            printf("\nHamle sayisi: %d\n", game.moves);
                        }
                    }
                }
            }

        } //SDL_PollEvent(&event) ile eventleri kontrol ederiz
    
        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // rgba degeriyle rengimiz siyah ve tam opak 
        SDL_RenderClear(renderer); //renderirin tamamini temizler ve setRenderDrawColor() ile belirlenen renk ile doldurur

    
        

        
     

        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                // SDL_Rect, bir dikdortgenin nerede cizilecegini (x,y) koordinatlarini ve boyutunu (w,h) weight height tutan bir yapidir
                SDL_Rect rect;

                rect.x = startX + c * (cardWidth + padding); //170 +sutun*120

                rect.y = startY + r * (cardHeight + padding); //70 + satir*120

                rect.w = cardWidth;

                rect.h = cardHeight;


                if(game.board[r][c].state == 0) {
                    SDL_SetRenderDrawColor(renderer,118, 206, 242, 255); // Kapali kartlar icin acik mavi
                    SDL_RenderFillRect(renderer, &rect); // Dikdortgenin icini boyayarak ekrana cizdirir} 
                }
                else if(game.board[r][c].state == 1 || game.board[r][c].state == 2) 
                {
                    int val= game.board[r][c].val; //kart degeri, 1'den 8'e kadar

                    if(cardTexture[val] != NULL) {
                        //kart degerine karslik gelen texture'i cizdirir
                        SDL_RenderCopy(renderer, cardTexture[val], NULL, &rect); //kart degerine karslik gelen texture'i cizdirir
                    }
                    else {
                        //eger texture yuklenemedi ise, karti beyaz olarak cizdirir
                        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Beyaz renk
                        SDL_RenderFillRect(renderer, &rect); // Dikdortgenin icini boyayarak ekrana cizdirir
                    }
                }
            }
        }

        SDL_RenderPresent(renderer);
        // Renderer'in cizimlerini ekrana yansitir, bu fonksiyon cagrilmazsa yaptigimiz cizimler ekranda gorunmez
        
       if(game.flippedCount == 2)
       {
        SDL_Delay(1000); //kartlar acik kalacak ve kullaniciya gormesi icin 1 saniye bekle
       

        //eslesme kontrolu
        if(game.board[game.firstRow][game.firstCol].val == game.board[game.secondRow][game.secondCol].val) {
            //kartlar eslesiyor, kartState'i 2 yaparak eslesmis olarak isaretliyorum
            game.board[game.firstRow][game.firstCol].state = 2;
            game.board[game.secondRow][game.secondCol].state = 2;
            printf("Eslesme bulundu!\n");
        } 
        
        else {
            //kartlar eslesmiyor, kartState'i tekrar 0 yaparak kapali hale getiriyorum
            game.board[game.firstRow][game.firstCol].state = 0;
            game.board[game.secondRow][game.secondCol].state = 0;
            printf("Yanlis secim!Kartlar kapatiliyor.\n");
        }

        game.flippedCount = 0; //kartlar kapatildi veya eslesti, acik kart sayisini sifirla

        while (SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                isRunning = 0; 
            } //kullanici bu bekleme suresince pencereyi kapatmak isterse, bu olay da yakalanir ve oyun kapanir
        }

      }

    } // Oyun dongusu burada bitiyor, kullanici pencereyi kapatana kadar kartlar ekranda kalacak


    //oyun bittiginde memory leaks olmamasi icin kaynaklari serbest birakmamiz lazim
    for(int i=1; i<=8; i++) {
        if(cardTexture[i] != NULL) {
            SDL_DestroyTexture(cardTexture[i]); // her bir texture'i yok et
        }
    }


    // Oyun bittiginde hafizayi temizlememiz lazim
    SDL_DestroyRenderer(renderer);// Renderer'i yok et
    SDL_DestroyWindow(window);// Pencereyi yok et
    IMG_Quit(); // SDL_image subsistemini kapat
    SDL_Quit();// SDL subsistemlerini kapat

    return 0;
}