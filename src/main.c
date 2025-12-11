#include <stdio.h>
#include <SDL2/SDL.h>
#include <assert.h>

#define panic(...) do { fprintf(stderr, __VA_ARGS__); exit(0); } while (0)
#define expect(ptr, ...) __extension__({void* _ptr = (ptr); if (!_ptr) { panic(__VA_ARGS__); } _ptr;})

// TODO: implement ppm loading and parcing
// TODO: implement resize event rerender
int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    assert(!SDL_Init(SDL_INIT_VIDEO));
    SDL_Window* window = expect(
        SDL_CreateWindow("ppmviewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0),
        "Unable to create sdl window: %s", 
        SDL_GetError()
    );

    SDL_Renderer* renderer = expect(
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED),
        "Unable to create sdl renderer: %s", 
        SDL_GetError()
    ); 
    
    int quit = 0;
    int need_redraw = 1;

    while (!quit) {
        SDL_Event e = {0};
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT: {
                    quit = 1;
                    break;
                }
            }
        }

        // TODO: redraw on resize event
        if (need_redraw) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            SDL_RenderClear(renderer);
            SDL_RenderPresent(renderer);
        }

        need_redraw = 0;
        SDL_Delay(1);
    }

    SDL_Quit();

    return 0;
}