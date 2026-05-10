#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>
#include <SDL_image.h>

#define ROWS 4
#define COLS 4

int gameBoard[ROWS][COLS]; // oyun tahtasini temsil eden 2 boyutlu dizi
int cardState[ROWS][COLS]={0}; //kartlar baslangicta kapali


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

    initBoard();
    printBoardToConsole(); 

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




    int flippedCount = 0; //kac kartin acik oldugunu sayar
    int firstRow = -1, firstCol = -1; //acilan ilk kart
    int secondRow = -1, secondCol = -1; //acilan ikinci kart
    //su an hafiza tamamen bos, henuz hicbir kart acilmadi


    int moves=0; //kullanici kac hamle yaptigini sayar, her iki kart acildiginda moves 1 artar


    while (isRunning)
    {
        
        
        while (SDL_PollEvent(&event))
        {
            
            if (event.type == SDL_QUIT) {
                isRunning = 0; 
            }
            //event.type, kullanicinin ne tur bir eylemde bulundugunu belirtir, SDL_QUIT ise pencerenin kapatilmasi anlamina gelir
            // Eger event.type SDL_QUIT ise, isRunning 0 yapilir ve oyun dongusu kirilir, bu da oyunun kapanmasini saglar

            else if(event.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX = event.button.x; //fare tiklamasinin x koordinati
                int mouseY = event.button.y; //fare tiklamasinin y koordinati

                //tiklanan kartin hangi satir ve sutunda oldugunu bulmak icin
                int col = (mouseX - startX) / (cardWidth + padding);
                int row = (mouseY - startY) / (cardHeight + padding);

                //tiklanan yer gecerli bi satir ve sutunda mi kontrolu
                if (col >= 0 && col < COLS && row >= 0 && row < ROWS) {

                    if(cardState[row][col]==0 && flippedCount < 2) { //kart kapaliysa ve 2 karttan az acik varsa
                        cardState[row][col] = 1; //kart acilir
                    }

                    if(flippedCount == 0) {
                        //acilan ilk kartin konumunu kaydettim
                        firstRow = row;
                        firstCol = col;
                        flippedCount = 1; //bir kart acildi
                    } 
                        
                    else if(flippedCount == 1) {
                        //acilan ikinci kartin konumunu kaydettim
                        secondRow = row;
                        secondCol = col;
                        flippedCount = 2; //iki kart acildi
                        moves++; //kullanici bir hamle yapti, moves sayisini arttir
                        printf("\nHamle sayisi: %d\n", moves);
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


                if(cardState[r][c] == 0) {
                    SDL_SetRenderDrawColor(renderer,118, 206, 242, 255); // Kapali kartlar icin acik mavi
                    SDL_RenderFillRect(renderer, &rect); // Dikdortgenin icini boyayarak ekrana cizdirir} 
                }
                else if(cardState[r][c] == 1 || cardState[r][c] == 2) 
                {
                    int val= gameBoard[r][c]; //kart degeri, 1'den 8'e kadar

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
        
       if(flippedCount == 2)
       {
        SDL_Delay(1000); //kartlar acik kalacak ve kullaniciya gormesi icin 1 saniye bekle
       

        //eslesme kontrolu
        if(gameBoard[firstRow][firstCol] == gameBoard[secondRow][secondCol]) {
            //kartlar eslesiyor, kartState'i 2 yaparak eslesmis olarak isaretliyorum
            cardState[firstRow][firstCol] = 2;
            cardState[secondRow][secondCol] = 2;
            printf("Eslesme bulundu!\n");
        } 
        
        else {
            //kartlar eslesmiyor, kartState'i tekrar 0 yaparak kapali hale getiriyorum
            cardState[firstRow][firstCol] = 0;
            cardState[secondRow][secondCol] = 0;
            printf("Yanlis secim!Kartlar kapatiliyor.\n");
        }

        flippedCount = 0; //kartlar kapatildi veya eslesti, acik kart sayisini sifirla

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