#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

#include <math.h>

#include "draw.h"

extern SDL_Window *window;
extern SDL_Renderer *renderer;

#define PI 3.14159265358f

double wave_func(double t, double x,
                 double period, double lambda,
                 double amplitude, double phi)
{
    return amplitude * sin(2*PI*(t/period - x/lambda) + phi);
    //return sin(t - x);
}

void draw_waves_basic(void)
{
#define SCALE 50
    static double t = 0;
    t += 0.1;

    for (int x = 10; x < 500; x++) {
        int y = (int)(SCALE * wave_func(t, (double)x/SCALE, 5.f, 30.f, 1.f, 0.f));
        pixelRGBA(renderer, x, y + 300, 255, 0, 0, 255);
    }
}

void draw(void)
{
    draw_waves_basic();
}
