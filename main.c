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

#include "simstate.h"
#include "config.h"
#include "draw.h"

SDL_Window *window;
SDL_Renderer *renderer;
TTF_Font *font;

SimState SIM_STATE = {
    .mouse_down = false,
    .sel_scene = &SCENES[SCENE_MENU],
};

bool RUN = true;

void panic_sdl(const char *msg)
{
    fprintf(stderr, "sdl error: %s: %s", msg, SDL_GetError());
    exit(1);
}

void loop(void)
{
#ifndef __EMSCRIPTEN__
    Uint64 frame_start = SDL_GetTicks64();
#endif /* __EMSCRIPTEN__ */

    static SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_QUIT:
#ifdef __EMSCRIPTEN__
            emscripten_cancel_main_loop();
#else
            RUN = false;
            return;
#endif /* __EMSCRIPTEN__ */
        case SDL_MOUSEBUTTONDOWN:
            SIM_STATE.mouse_down = true;
            widget_trigger(ev.button.x, ev.button.y);
            break;
        case SDL_MOUSEBUTTONUP:
            SIM_STATE.mouse_down = false;
            break;
        case SDL_MOUSEMOTION:
            if (SIM_STATE.mouse_down)
                widget_update_sliders(ev.button.x, ev.button.y);
            break;
        }
    }

    SDL_SetRenderDrawColor(renderer, 18, 18, 18, 255);
    SDL_RenderClear(renderer);

    draw_scene(SIM_STATE.sel_scene);

    SDL_RenderPresent(renderer);

#ifndef __EMSCRIPTEN__
    /* limit fps */
    Uint64 frame_time = SDL_GetTicks64() - frame_start;
    if (frame_time < CONFIG_FPS_DELTA) {
        SDL_Delay(CONFIG_FPS_DELTA-frame_time);
    }
#endif /* __EMSCRIPTEN__ */
}

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        panic_sdl("init");

    window = SDL_CreateWindow("Mechanical Waves",
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              CONFIG_WINDOW_WIDTH, CONFIG_WINDOW_HEIGHT,
                              SDL_WINDOW_OPENGL);
    if (!window)
        panic_sdl("CreateWindow");

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer)
        panic_sdl("CreateRenderer");

    if (TTF_Init())
        panic_sdl("TTF_Init");
        
    font = TTF_OpenFont("./res/LiberationSans-Regular.ttf", CONFIG_FONT_SIZE);
    if (!font)
        panic_sdl("TTF_OpenFont");

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loop, 0, 1);
#else
    while (RUN) loop();
#endif

    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
