#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "game.h"
#include <SDL_mixer.h>



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

    if (TTF_Init() == -1) {
        printf("SDL_ttf baslatilamadi! Hata: %s\n", TTF_GetError());
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer baslatilamadi! Hata: %s\n", Mix_GetError());
        return 1;
    }
    Mix_Chunk *flipSound = Mix_LoadWAV("assets/freesound_community-flipcard-91468.mp3"); //kart cevirme sesi
    Mix_Chunk *matchSound = Mix_LoadWAV("assets/oxidvideos-placing-playing-card-522514.mp3"); //kart eslesme sesi
    Mix_Chunk *wrongSound = Mix_LoadWAV("assets/freesound_community-wronganswer-37702.mp3"); //kart eslesmeme sesi
    //Mix_Chunk ses efektleri icin kullanilir

    if(flipSound == NULL || matchSound == NULL || wrongSound == NULL) {
        printf("Ses dosyalari yuklenemedi! Hata: %s\n", Mix_GetError());
        return 1;
    }

    TTF_Font *font = TTF_OpenFont("assets/IMPACT.ttf", 24); //yazilari gostermek icin bir font yukledim
    if (font == NULL) {
        printf("Font yuklenemedi! Hata: %s\n", TTF_GetError());
        return 1;
    }

    SDL_DisplayMode displayMode;
    SDL_GetDesktopDisplayMode(0, &displayMode); //mevcut ekranin cozunurlugunu alir
    int screenW = displayMode.w; //ekran genisligi
    int screenH = displayMode.h; //ekran yuksekligi

    //SDL ile pencere olusturmak icin SDL_CreateWindow() fonksiyonu
    //parametreler: baslik, x konumu, y konumu, genislik, yukseklik, pencere gorunurlugu
    // *window basarili olursa pencereye isaret eden pointer, hata olursa NULL doner

    SDL_Window *window = SDL_CreateWindow("Memory Card Game", 
                                          SDL_WINDOWPOS_CENTERED, 
                                          SDL_WINDOWPOS_CENTERED, 
                                          screenW, screenH, SDL_WINDOW_MAXIMIZED);
    
                                          //SDL_WINDOW_MAXIMIZED, pencereyi guncel ekranimiza gore en buyuk hale getirir
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
    

    int gameMode=0; //0: menu, 1: oyun, 2: oyun bitti
    int isRunning = 1; //oyunun calismaya devam edip etmeyecegni kontrol eden yapidir, 1 ise calisiyor, 0 ise duracak
    SDL_Event event; // klavye, fare veya pencere olaylarini (event) tutar


    SDL_GetWindowSize(window, &screenW, &screenH); //pencerenin genisligini ve yuksekligini alir, bu degerleri screenW ve screenH degiskenlerine atar
    int cardWidth = 100;  // Kart genisligi
    int cardHeight = 100; // Kart yuksekligi
    int padding = 20;     // Kartlar arasi bosluk



    // Ekranin (800x600) tam ortasina hizalamak icin baslangic noktalari secerken
    //4x4 lük sistemimizde 4 kart genisligi ve 3 tane de padding var
    //startX = (ekran genisligi - (4 kart genisligi + 3 padding)) / 2
    //startY = (ekran yuksekligi - (4 kart yuksekligi + 3 padding)) / 2
    int startX = (screenW - (4 * cardWidth + 3 * padding)) / 2; //kartlarin baslangic x konumu
    int startY = (screenH - (4 * cardHeight + 3 * padding)) / 2;  //kartlarin baslangic y konumu




    Uint32 startTime = SDL_GetTicks(); //oyuna basladıgımız anı milisaniye cinsinden verir
    
    //Butonlarin yerleri
    SDL_Rect startButton = {
        .x = (screenW - 200) / 2, //start butonunun x konumu
        .y = (screenH - 60) / 2, //start butonunun y konumu
        .w = 200, //start butonunun genisligi
        .h = 60  //start butonunun yuksekligi
    };
    SDL_Rect restartButton={
        .x = (screenW - 200) / 2, //restart butonunun x konumu
        .y = screenH - 150, //restart butonunun y konumu, ekranin altindan 150 piksel yukari
        .w = 200, //restart butonunun genisligi
        .h = 60   //restart butonunun yuksekligi
    };


    while (isRunning)
    {
        if(game.isGameOver == 0 && gameMode == 1) {
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
        else if(gameMode == 0){
            SDL_SetWindowTitle(window, "Memory Card Game - Hos Geldiniz!"); //giris ekranindayken pencere basligini belirliyoruz

        }
        else if(game.isGameOver == 1) {
            SDL_SetWindowTitle(window, "TEBRIKLER! Tum kartlari eslestirdiniz! - Yeniden baslat butonuna tiklayin"); //tum kartlar eslesmis ve oyun bitmisse pencere basligini degistiriyoruz
        }
    

        while (SDL_PollEvent(&event))
        {
            
            if (event.type == SDL_QUIT) {
                isRunning = 0; 
            }
            //event.type, kullanicinin ne tur bir eylemde bulundugunu belirtir, SDL_QUIT ise pencerenin kapatilmasi anlamina gelir
            // Eger event.type SDL_QUIT ise, isRunning 0 yapilir ve oyun dongusu kirilir, bu da oyunun kapanmasini saglar

            else if(event.type == SDL_KEYDOWN) {
                if(event.key.keysym.sym == SDLK_ESCAPE) { //kullanici ESC tusuna basarsa
                    isRunning = 0; //oyunu kapat
                }
            }
            else if(event.type == SDL_MOUSEBUTTONDOWN) {

                int mouseX = event.button.x; //fare tiklamasinin x koordinati
                int mouseY = event.button.y; //fare tiklamasinin y koordinati

                //durum 0:Giris ekrani
                if(gameMode == 0) {
                    if(mouseX >= startButton.x && mouseX <= startButton.x + startButton.w &&
                        mouseY >= startButton.y && mouseY <= startButton.y + startButton.h) {
                        //tiklanan x kutunun sol kenarından buyuk ve sag kenarindan kucuk
                        //tiklanan y kutunun ust kenarindan buyuk ve alt kenarindan kucukse
                        //start butonuna tiklandi, oyunu baslat
                        gameMode = 1; //oyun moduna gectik, artik kartlar ekranda olacak ve tiklanabilir olacaklar
                        startTime = SDL_GetTicks(); //oyunu baslattigimiz anin zamanini kaydedelim, bu zamani oyun sure hesaplamak icin kullanacagiz
                        printf("Oyun baslatildi!\n");
                    }
               }

                //durum 1: Oyun ekrani, kartlar tiklanabilir
                else if(gameMode == 1 && game.isGameOver == 0) {
                    //tiklanan kartin hangi satir ve sutunda oldugunu bulmak icin
                    int col = (mouseX - startX) / (cardWidth + padding);
                    int row = (mouseY - startY) / (cardHeight + padding);

                    //tiklanan yer gecerli bi satir ve sutunda mi kontrolu
                    if (col >= 0 && col < COLS && row >= 0 && row < ROWS) {

                        if(game.board[row][col].state == 0 && game.flippedCount < 2) { //kart kapaliysa ve 2 karttan az acik varsa
                            game.board[row][col].state = 1; //kart acilir
                            Mix_PlayChannel(-1, flipSound, 0); //kart cevirme sesi oynatilir

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
                //durum 2: Oyun bitti, restart butonu tiklanabilir
                else if(game.isGameOver == 1) {
                    if(mouseX >= restartButton.x && mouseX <= restartButton.x + restartButton.w &&
                        mouseY >= restartButton.y && mouseY <= restartButton.y + restartButton.h) {
                        //tiklanan x kutunun sol kenarından buyuk ve sag kenarindan kucuk
                        //tiklanan y kutunun ust kenarindan buyuk ve alt kenarindan kucukse
                        //restart butonuna tiklandi, oyunu yeniden baslat
                        initGame(&game); //oyunu baslatan fonksiyonu tekrar cagirarak oyunu sifirla
                        game.isGameOver = 0; //oyun bitmedi olarak isaretle
                        gameMode = 1; //oyun moduna gectik, artik kartlar ekranda olacak ve tiklanabilir olacaklar
                        startTime = SDL_GetTicks(); //yeni bir oyun baslattigimiz icin zamani sifirla 
                        printf("Oyun yeniden baslatildi!\n");
                    }
                }
                
            }

        } //SDL_PollEvent(&event) ile eventleri kontrol ederiz
    
        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // rgba degeriyle rengimiz siyah ve tam opak 
        SDL_RenderClear(renderer); //renderirin tamamini temizler ve setRenderDrawColor() ile belirlenen renk ile doldurur

        if(gameMode == 0) {
            //giris ekranini cizdir
            SDL_SetRenderDrawColor(renderer, 50, 50, 150, 255); // Koyu mavi arkaplan
            SDL_RenderClear(renderer); //renderirin tamamini temizler ve setRenderDrawColor() ile belirlenen renk ile doldurur
            
            //basla butonu
            SDL_SetRenderDrawColor(renderer, 0 ,200, 0 ,255); // Yesil renk
            SDL_RenderFillRect(renderer, &startButton); //start butonunu cizdir

            //start butonu yazisi
            SDL_Color white={255, 255, 255}; 
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Oyunu Baslat", white); //buton yazisini bir surface'e renderlar
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface); //surface'i texture'a cevirir
            SDL_Rect textRect;
            textRect.x = startButton.x + (startButton.w - textSurface->w) / 2; //butonun tam ortasina yaziyi yerlestir
            textRect.y = startButton.y + (startButton.h - textSurface->h) / 2; //butonun tam ortasina yaziyi yerlestir
            textRect.w = textSurface->w; //yazinin genisligi
            textRect.h = textSurface->h; //yazinin yuksekligi
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect); //buton yazisini ekrana cizdirir
            SDL_FreeSurface(textSurface); //surface'i serbest birakiyoruz cunku artik texture
            SDL_DestroyTexture(textTexture); //text texture'ini yok ediyoruz cunku her dongude yeniden olusturuluyor

        }
        

        else{

            for (int r = 0; r < ROWS; r++) {
                for (int c = 0; c < COLS; c++) {
                    // SDL_Rect, bir dikdortgenin nerede cizilecegini (x,y) koordinatlarini ve boyutunu (w,h) weight height tutan bir yapidir
                    SDL_Rect rect;

                    rect.x = startX + c * (cardWidth + padding); 
                    rect.y = startY + r * (cardHeight + padding); 
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
            
            if(font != NULL){
                char timeText[100];

                if(game.matchedPairs == 8) {
                    game.isGameOver = 1; //tum kartlar eslesmis, oyun bitmis
                    snprintf(timeText, sizeof(timeText), "Tebrikler! Tum kartlari eslestirdiniz. Kalan sure: %d saniye", game.timeRemaining);
                }
                else if(game.timeRemaining <= 0) {
                    snprintf(timeText, sizeof(timeText), "SURE BITTI! Eslesen kart ciftleri: %d", game.matchedPairs);
                }
                else {
                    snprintf(timeText, sizeof(timeText), "Kalan Sure: %d saniye | Hamle: %d | Eslesen Ciftler: %d", game.timeRemaining, game.moves, game.matchedPairs);
                }

                SDL_Color textColor = {255, 255, 255}; // Beyaz renk
                SDL_Surface* textSurface = TTF_RenderText_Solid(font, timeText, textColor); //yaziyi bir surface'e renderlar
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface); //surface'i texture'a cevirir

                SDL_Rect textRect;
                textRect.x = 20; //yazinin x konumu
                textRect.y = 20; //yazinin y konumu
                textRect.w = textSurface->w; //yazinin genisligi
                textRect.h = textSurface->h; //yazinin yuksekligi
                SDL_RenderCopy(renderer, textTexture, NULL, &textRect); //yaziyi ekrana cizdirir

                SDL_FreeSurface(textSurface); //surface'i serbest birakiyoruz cunku artik texture olarak kullanacagiz
                SDL_DestroyTexture(textTexture); //text texture'ini yok ediyoruz cunku her dongude yeniden olusturuluyor
            }
        
            if(game.isGameOver == 1) {
                //oyun bittiginde restart butonunu cizdir

                SDL_SetRenderDrawColor(renderer, 50, 200, 50, 255); // Yesil renk
                SDL_RenderFillRect(renderer, &restartButton); //restart butonunu cizdir
            
                if(font !=NULL){
                    SDL_Color buttonTextColor = {255, 255, 255}; // Beyaz renk
                    SDL_Surface* buttonTextSurface = TTF_RenderText_Solid(font, "Yeniden Baslat", buttonTextColor); //buton yazisini bir surface'e renderlar
             
                    if(buttonTextSurface != NULL) {
                        SDL_Texture* buttonTextTexture = SDL_CreateTextureFromSurface(renderer, buttonTextSurface); //surface'i texture'a cevirir

                        SDL_Rect buttonTextRect;
                        buttonTextRect.x = restartButton.x + (restartButton.w - buttonTextSurface->w) / 2; //butonun tam ortasina yaziyi yerlestir
                        buttonTextRect.y = restartButton.y + (restartButton.h - buttonTextSurface->h) / 2;
                        buttonTextRect.w = buttonTextSurface->w;
                        buttonTextRect.h = buttonTextSurface->h;

                        SDL_RenderCopy(renderer, buttonTextTexture, NULL, &buttonTextRect); //buton yazisini ekrana cizdirir
                        SDL_FreeSurface(buttonTextSurface); //surface'i serbest birakiyoruz cunku artik texture olarak kullanacagiz
                        SDL_DestroyTexture(buttonTextTexture); //buton text texture'ini yok ediyoruz cunku her dongude yeniden olusturuluyor
                    }
                }
   
            }
        }

        SDL_RenderPresent(renderer); //render edilen her seyi ekrana gosterir
        
    
        if(game.flippedCount ==2 && gameMode == 1) {
            SDL_Delay(1000); //2 kart acildiktan sonra 1 saniye bekle

            if(game.board[game.firstRow][game.firstCol].val == game.board[game.secondRow][game.secondCol].val) {
                //kartlar eslesmis
                Mix_PlayChannel(-1, matchSound, 0); //eslesme sesi oynatilir
                game.board[game.firstRow][game.firstCol].state = 2; //eslesen kartlar eslesmis olarak isaretlenir
                game.board[game.secondRow][game.secondCol].state = 2;
                game.matchedPairs++; //eslesen cift sayisini arttir
               
                if(game.matchedPairs == 8) {
                    game.isGameOver = 1; //tum kartlar eslesmis, oyun bitmis
                    printf("Tebrikler! Tum kartlari eslestirdiniz. Kalan sure: %d saniye\n", game.timeRemaining);
                }

                printf("Eslesen kartlar: %d\n", game.matchedPairs);
            }
                else {
                    //kartlar eslesmemis, tekrar kapatilir
                    game.board[game.firstRow][game.firstCol].state = 0;
                    game.board[game.secondRow][game.secondCol].state = 0;
                    Mix_PlayChannel(-1, wrongSound, 0); //eslesmeme sesi oynatilir
                    printf("Kartlar eslesmedi. Tekrar deneyin.\n");
                }

                game.flippedCount = 0; //acik kart sayisini sifirla, kullanici yeni kartlar acabilir

                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        isRunning = 0; //kullanici pencereyi kapatmak isterse oyun dongusunu kir
                    }
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
    if(font != NULL) {
        TTF_CloseFont(font); // Fontu kapat
    }
    if(flipSound != NULL) {
        Mix_FreeChunk(flipSound); //kart cevirme sesini serbest birak
    }
    if(matchSound != NULL) {
        Mix_FreeChunk(matchSound); //eslesme sesini serbest birak
    }
    if(wrongSound != NULL) {
        Mix_FreeChunk(wrongSound); //eslesmeme sesini serbest birak
    }
    Mix_CloseAudio(); // SDL_mixer subsistemini kapat
    SDL_DestroyWindow(window);// Pencereyi yok et
    TTF_CloseFont(font); // Fontu kapat
    TTF_Quit(); // SDL_ttf subsistemini kapat
    IMG_Quit(); // SDL_image subsistemini kapat
    SDL_Quit();// SDL subsistemlerini kapat

    return 0;
}

