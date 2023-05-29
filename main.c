#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <GL/glew.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif /* __EMSCRIPTEN__ */

#include "config.h"
#include "draw.h"

SDL_Window *window;
SDL_Renderer *renderer;

bool RUN = true;

struct SimState {
    Scene *scene;
} SIM_STATE;

void panic_sdl(const char *msg)
{
    fprintf(stderr, "sdl error: %s: %s", msg, SDL_GetError());
    exit(1);
}

void loop(void)
{
    Uint64 frame_start = SDL_GetTicks64();

    static SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_QUIT:
#ifdef __EMSCRIPTEN__
            emscripten_cancel_main_loop();
#else
            RUN = false;
            return;
#endif
        case SDL_WINDOWEVENT:
            if (ev.window.event == SDL_WINDOWEVENT_RESIZED) {
                int win_w, win_h;
                SDL_GetWindowSize(window, &win_w, &win_h);
                printf("SIZE: %d %d\n", win_w, win_h);
            }
            break;
        }
    }

    SDL_SetRenderDrawColor(renderer, 18, 18, 18, 255);
    SDL_RenderClear(renderer);
    draw();
    SDL_RenderPresent(renderer);

    /* limit fps */
    Uint64 frame_time = SDL_GetTicks64() - frame_start;
    if (frame_time < FPS_DELTA) {
        SDL_Delay(FPS_DELTA-frame_time);
    }
}

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        panic_sdl("init");

    window = SDL_CreateWindow("_dwm_float",
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              CONFIG_WINDOW_WIDTH, CONFIG_WINDOW_HEIGHT,
                              SDL_WINDOW_SHOWN);
    if (!window)
        panic_sdl("create_window");

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer)
        panic_sdl("create_renderer");

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loop, -1, 1);
#else
    while (RUN) loop();
#endif

    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
