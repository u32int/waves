#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

#include <math.h>
#include "draw.h"
#include "config.h"
#include "simstate.h"

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern TTF_Font *font;

#define PI 3.14159265358f

// global sim variables
double TIME_STEP = 0.1;
double GLOB_AMPLITUDE = 1.f;
double GLOB_LAMBDA = 30.f;
double GLOB_PERIOD = 5.f;
double GLOB_PHI = 0.f;

int GLOB_WAVE_POINTS = 1000;
#define WAVE_STEP (CONFIG_WINDOW_WIDTH/GLOB_WAVE_POINTS)

double wave_func(double t, double x,
                 double period, double lambda,
                 double amplitude, double phi)
{
    return amplitude * sin(2*PI*(t/period - x/lambda) + phi);
}

void draw_text(const char *label, int x1, int y1, int x2, int y2)
{
    SDL_Color color = {255, 255, 255};

    SDL_Surface *surface = TTF_RenderText_Solid(font, label, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect rect;
    rect.x = x1;
    rect.y = y1;
    rect.w = abs(x1-x2); 
    rect.h = abs(y1-y2);

    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void draw_scene_menu()
{
    static double t = 0;
    t += TIME_STEP;
    int y = 50 * sin(((double)CONFIG_WINDOW_WIDTH/2-300)/50 + t);

    for (int x = CONFIG_WINDOW_WIDTH/2-300; x < CONFIG_WINDOW_WIDTH/2+300; x += WAVE_STEP) {
        int ny = 50 * sin((double)x/50 + t);
        lineRGBA(renderer,
                 x-WAVE_STEP, 200+y,
                 x,           200+ny,
                 255, 0, 0, 255);
        y = ny;
    }

    draw_text("mechanical waves",
              CONFIG_WINDOW_WIDTH/2-400, 10,
              CONFIG_WINDOW_WIDTH/2+400, 120);
}

void draw_scene_basic()
{
#define SCALE 50
#define START_POS 10
    static double t = 0;
    t += TIME_STEP;
    int y = (int)(SCALE *
                  wave_func(t, (double)START_POS/SCALE, GLOB_PERIOD, GLOB_LAMBDA, GLOB_AMPLITUDE, 0.f));

    // animate basic wave equation
    for (int x = START_POS+WAVE_STEP; x < CONFIG_WINDOW_WIDTH; x += WAVE_STEP) {
        int ny = (int)(SCALE * wave_func(t, (double)x/SCALE, GLOB_PERIOD, GLOB_LAMBDA, GLOB_AMPLITUDE, 0.f));
        lineRGBA(renderer,
                 x-WAVE_STEP, CONFIG_WINDOW_HEIGHT/2+y,
                 x, CONFIG_WINDOW_HEIGHT/2+ny,
                 255, 0, 0, 255);
        y = ny;
    }
}

void draw_scene_interference()
{
#define SCALE 50
#define START_POS 10
    static double t = 0;
    t += TIME_STEP;

    // ...

    int red_y = (int)(SCALE *
                  wave_func(t, (double)START_POS/SCALE, GLOB_PERIOD, GLOB_LAMBDA, GLOB_AMPLITUDE, 0.f));

    int blue_y = (int)(SCALE *
                  wave_func(t, (double)START_POS/SCALE, GLOB_PERIOD, GLOB_LAMBDA, GLOB_AMPLITUDE, 0.f));

    int combined_y = blue_y + red_y;

    for (int x = START_POS+WAVE_STEP; x < CONFIG_WINDOW_WIDTH; x += WAVE_STEP) {
        int red_ny = (int)(SCALE *
                           wave_func(t, (double)x/SCALE, GLOB_PERIOD, GLOB_LAMBDA, GLOB_AMPLITUDE, 0.f));
        lineRGBA(renderer,
                 x-WAVE_STEP, CONFIG_WINDOW_HEIGHT/2+red_y,
                 x, CONFIG_WINDOW_HEIGHT/2+red_ny,
                 255, 0, 0, 100);
        red_y = red_ny;

        int blue_ny = (int)(SCALE *
                           wave_func(t, (double)x/SCALE, GLOB_PERIOD, GLOB_LAMBDA, GLOB_AMPLITUDE, GLOB_PHI));
        lineRGBA(renderer,
                 x-WAVE_STEP, CONFIG_WINDOW_HEIGHT/2+blue_y,
                 x, CONFIG_WINDOW_HEIGHT/2+blue_ny,
                 0, 0, 255, 100);
        blue_y = blue_ny;

        int combined_ny = red_ny + blue_ny;
        lineRGBA(renderer,
                 x-WAVE_STEP, CONFIG_WINDOW_HEIGHT/2+combined_y,
                 x, CONFIG_WINDOW_HEIGHT/2+combined_ny,
                 0, 255, 0, 255);
        combined_y = combined_ny;
    }
}

// this is horrible and ugly but idk how to ensure consistent indexes for passing ptrs to .data (enum??)
#define BASIC_LAMBDA_SLIDER 0
#define BASIC_AMPLITUDE_SLIDER 1
#define BASIC_TIME_SLIDER 2
#define BASIC_POINT_SLIDER 3

#define INTERF_OFFSET 0
#define INTERF_TIME_SLIDER 1

Scene SCENES[] = {
    [SCENE_MENU] = {
        .drawfn = draw_scene_menu,
        .widgets = {
            {
                .widget_type = WIDGET_BUTTON,
                .x1 = CONFIG_WINDOW_WIDTH/2 - 150, .y1 = 350,
                .x2 = CONFIG_WINDOW_WIDTH/2 + 150, .y2 = 440,
                .label = "wave func",
                .callback = callback_switch_scene,
                .callback_data = &SCENES[SCENE_BASIC_WAVE_FUNC],
            },
            {
                .widget_type = WIDGET_BUTTON,
                .x1 = CONFIG_WINDOW_WIDTH/2 - 150, .y1 = 460,
                .x2 = CONFIG_WINDOW_WIDTH/2 + 150, .y2 = 540,
                .label = "interference",
                .callback = callback_switch_scene,
                .callback_data = &SCENES[SCENE_INTERFERENCE],
            },
            {
                .widget_type = WIDGET_END
            }
        }
    },
    [SCENE_INTERFERENCE] = {
        .drawfn = draw_scene_interference,
        .widgets = {
            [INTERF_OFFSET] = {
                .widget_type = WIDGET_SLIDER,
                .x1 = 1100, .y1 = CONFIG_WINDOW_HEIGHT-150,
                .x2 = 1300, .y2 = CONFIG_WINDOW_HEIGHT-40,
                .label = "offset",
                .slider_min = 0, .slider_max = PI*2,
                .slider_value = 0, .slider_var = &GLOB_PHI,
                .callback = callback_slider_setvar_double,
                .callback_data = &SCENES[SCENE_INTERFERENCE].widgets[INTERF_OFFSET] // this widget
            },
            [INTERF_TIME_SLIDER] = {
                .widget_type = WIDGET_SLIDER,
                .x1 = 900, .y1 = 10,
                .x2 = 1100, .y2 = 150,
                .label = "time flow",
                .slider_min = 0.01, .slider_max = 1,
                .slider_value = 0.1f, .slider_var = &TIME_STEP,
                .callback = callback_slider_setvar_double,
                .callback_data = &SCENES[SCENE_INTERFERENCE].widgets[INTERF_TIME_SLIDER] // this widget
            },
            {
                .widget_type = WIDGET_BUTTON,
                .x1 = 0, .y1 = 0,
                .x2 = 300, .y2 = 80,
                .label = "Back to Menu",
                .callback = callback_switch_scene,
                .callback_data = &SCENES[SCENE_MENU],
            },
            {
                .widget_type = WIDGET_END
            }
        }
    },
    [SCENE_BASIC_WAVE_FUNC] = {
        .drawfn = draw_scene_basic,
        .widgets = {
            [BASIC_LAMBDA_SLIDER] = {
                .widget_type = WIDGET_SLIDER,
                .x1 = 600, .y1 = 10,
                .x2 = 800, .y2 = 150,
                .label = "lambda",
                .slider_min = 0, .slider_max = 100,
                .slider_value = 30.f, .slider_var = &GLOB_LAMBDA,
                .callback = callback_slider_setvar_double,
                .callback_data = &SCENES[SCENE_BASIC_WAVE_FUNC].widgets[BASIC_LAMBDA_SLIDER] // this widget
            },
            [BASIC_AMPLITUDE_SLIDER] = {
                .widget_type = WIDGET_SLIDER,
                .x1 = 850, .y1 = 10,
                .x2 = 1050, .y2 = 150,
                .label = "amplitude",
                .slider_min = 0, .slider_max = 5,
                .slider_value = 1.f, .slider_var = &GLOB_AMPLITUDE,
                .callback = callback_slider_setvar_double,
                .callback_data = &SCENES[SCENE_BASIC_WAVE_FUNC].widgets[BASIC_AMPLITUDE_SLIDER] // this widget
            },
            [BASIC_TIME_SLIDER] = {
                .widget_type = WIDGET_SLIDER,
                .x1 = 1100, .y1 = 10,
                .x2 = 1300, .y2 = 150,
                .label = "time flow",
                .slider_min = 0.01, .slider_max = 1,
                .slider_value = 0.1f, .slider_var = &TIME_STEP,
                .callback = callback_slider_setvar_double,
                .callback_data = &SCENES[SCENE_BASIC_WAVE_FUNC].widgets[BASIC_TIME_SLIDER] // this widget
            },
            [BASIC_POINT_SLIDER] = {
                .widget_type = WIDGET_SLIDER,
                .x1 = 1100, .y1 = CONFIG_WINDOW_HEIGHT-150,
                .x2 = 1300, .y2 = CONFIG_WINDOW_HEIGHT-40,
                .label = "wave points",
                .slider_min = 10, .slider_max = CONFIG_WINDOW_WIDTH,
                .slider_value = 1000.f, .slider_var = &GLOB_WAVE_POINTS,
                .callback = callback_slider_setvar_int,
                .callback_data = &SCENES[SCENE_BASIC_WAVE_FUNC].widgets[BASIC_POINT_SLIDER] // this widget
            },
            {
                .widget_type = WIDGET_BUTTON,
                .x1 = 0, .y1 = 0,
                .x2 = 300, .y2 = 80,
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
