#include <stdio.h>
#include <SDL.h>

int main(int argc, char *argv[]) {
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL Baslatilamadi! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Pencere olustur
    SDL_Window *window = SDL_CreateWindow("Memory Card Game", 
                                          SDL_WINDOWPOS_CENTERED, 
                                          SDL_WINDOWPOS_CENTERED, 
                                          800, 600, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Pencere olusturulamadi! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Ekranda 3000 milisaniye (3 saniye) bekle
    SDL_Delay(3000);

    // Hafizayi temizle ve cik
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}