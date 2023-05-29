#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

#include <math.h>
#include "draw.h"
#include "simstate.h"

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern TTF_Font *font;

#define PI 3.14159265358f

// global sim variables
double TIME_STEP = 0.1;
double GLOB_AMPLITUDE;
double GLOB_LAMBDA;
double GLOB_PERIOD;

double wave_func(double t, double x,
                 double period, double lambda,
                 double amplitude, double phi)
{
    return amplitude * sin(2*PI*(t/period - x/lambda) + phi);
}

void draw_waves_basic()
{
#define SCALE 50
    static double t = 0;
    t += TIME_STEP;

    for (int x = 10; x < 500; x++) {
        int y = (int)(SCALE * wave_func(t, (double)x/SCALE, 5.f, 30.f, 1.f, 0.f));
        pixelRGBA(renderer, x, y + 300, 255, 0, 0, 255);
    }
}

Scene SCENES[] = {
    [SCENE_MENU] = {
        .drawfn = NULL,
        .widgets = {
            {
                .widget_type = WIDGET_BUTTON,
                .x1 = 0, .y1 = 0,
                .x2 = 550, .y2 = 250,
                .label = "start",
                .callback = callback_switch_scene,
                .callback_data = &SCENES[SCENE_BASIC_WAVE_FUNC],
            },
            {
                .widget_type = WIDGET_END
            }
        }
    },
    [SCENE_BASIC_WAVE_FUNC] = {
        .drawfn = draw_waves_basic,
        .widgets = {
            {
                .widget_type = WIDGET_BUTTON,
                .x1 = 0, .y1 = 0,
                .x2 = 250, .y2 = 50,
                .label = "Back to Menu",
                .callback = callback_switch_scene,
                .callback_data = &SCENES[SCENE_MENU],
            },
            {
                .widget_type = WIDGET_END
            }
        }
    }
};

void trigger_widget(int x, int y)
{
    Scene *scene = SIM_STATE.sel_scene;

    for (int i = 0; i < CONFIG_MAX_WIDGETS; i++) {
        if (scene->widgets[i].widget_type == WIDGET_END)
            break;

        if (x > scene->widgets[i].x1 && y > scene->widgets[i].y1 &&
            x < scene->widgets[i].x2 && y < scene->widgets[i].y2) {
            if (!scene->widgets[i].callback)
                return;

            scene->widgets[i].callback(scene->widgets[i].callback_data);
        }
    }
}

void draw_scene(Scene *scene)
{
    // draw main contents of the scene
    if (scene->drawfn)
        scene->drawfn();

    // draw associated widgets
    for (int i = 0; i < CONFIG_MAX_WIDGETS; i++) {
        if (scene->widgets[i].widget_type == WIDGET_END)
            break;

        draw_widget(&scene->widgets[i]);
    }
}
