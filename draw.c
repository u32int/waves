#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

#include <math.h>
#include <limits.h>

#include "draw.h"
#include "config.h"
#include "simstate.h"

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern TTF_Font *font;
extern TTF_Font *font_small;
extern TTF_Font *font_huge;

#define PI 3.14159265358f

#define DEFAULT_TIME_STEP 0.1f
#define DEFAULT_DOPPLER_V 1
#define DEFAULT_DOPPLER_WAVE_SPEED 3
#define DEFAULT_GLOB_AMPLITUDE 2.f
#define DEFAULT_GLOB_LAMBDA 30.f
#define DEFAULT_GLOB_PERIOD 5.f
#define DEFAULT_GLOB_PHI 0.f
#define DEFAULT_GLOB_WAVE_POINTS 400

// global sim variables
static double TIME_STEP = DEFAULT_TIME_STEP;
static int DOPPLER_V = DEFAULT_DOPPLER_V;
static int DOPPLER_WAVE_SPEED = DEFAULT_DOPPLER_WAVE_SPEED;
static double GLOB_AMPLITUDE = DEFAULT_GLOB_AMPLITUDE;
static double GLOB_LAMBDA = DEFAULT_GLOB_LAMBDA;
static double GLOB_PERIOD = DEFAULT_GLOB_PERIOD;
static double GLOB_PHI = DEFAULT_GLOB_PHI;
static int GLOB_WAVE_POINTS = DEFAULT_GLOB_WAVE_POINTS;

#define WAVE_STEP (CONFIG_WINDOW_WIDTH/GLOB_WAVE_POINTS)

double wave_func(double t, double x,
                 double period, double lambda,
                 double amplitude, double phi)
{
    return amplitude * sin(2*PI*(t/period - x/lambda) + phi);
}

void render_text(const char* text, int x, int y, TTF_Font *text_font)
{
    SDL_Surface *text_surface;

    SDL_Color fgcolor = {255, 255, 255};
    SDL_Color bgcolor = {18, 18, 18};

    text_surface = TTF_RenderUTF8_Shaded(text_font, text, fgcolor, bgcolor);
    
    if(!text_surface) {
        fprintf(stderr, "sdl_ttf error: %s", TTF_GetError());
        exit(1);
    }

    int txt_w = text_surface->w;
    int txt_h = text_surface->h;

    SDL_Rect text_rect = { .h = txt_h, .w = txt_w,
                           .x = x, .y = y};

    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);

    SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);

    SDL_FreeSurface(text_surface);
    SDL_DestroyTexture(text_texture);
}

void draw_scene_menu()
{
    static double t = 0;
    t += DEFAULT_TIME_STEP;
    int y = 50 * sin(((double)CONFIG_WINDOW_WIDTH/2-300)/50 + t);

    for (int x = CONFIG_WINDOW_WIDTH/2-300; x < CONFIG_WINDOW_WIDTH/2+300; x += WAVE_STEP) {
        int ny = 50 * sin((double)x/50 + t);
        lineRGBA(renderer,
                 x-WAVE_STEP, 200+y,
                 x,           200+ny,
                 255, 0, 0, 255);
        y = ny;
    }

    render_text("fale mechaniczne",
                CONFIG_WINDOW_WIDTH/2-360, 10, font_huge);
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

    int red_y = (int)(SCALE * wave_func(t, (double)START_POS/SCALE,
                                        DEFAULT_GLOB_PERIOD,
                                        DEFAULT_GLOB_LAMBDA, DEFAULT_GLOB_AMPLITUDE, 0.f));

    int blue_y = (int)(SCALE * wave_func(t, (double)START_POS/SCALE,
                                         DEFAULT_GLOB_PERIOD,
                                         DEFAULT_GLOB_LAMBDA, DEFAULT_GLOB_AMPLITUDE, GLOB_PHI));

    int combined_y = blue_y + red_y;

    for (int x = START_POS+WAVE_STEP; x < CONFIG_WINDOW_WIDTH; x += WAVE_STEP) {
        int red_ny = (int)(SCALE * wave_func(t, (double)x/SCALE,
                                             DEFAULT_GLOB_PERIOD,
                                             DEFAULT_GLOB_LAMBDA, DEFAULT_GLOB_AMPLITUDE, 0.f));
        lineRGBA(renderer,
                 x-WAVE_STEP, CONFIG_WINDOW_HEIGHT/2+red_y,
                 x, CONFIG_WINDOW_HEIGHT/2+red_ny,
                 255, 0, 0, 100);
        red_y = red_ny;

        int blue_ny = (int)(SCALE * wave_func(t, (double)x/SCALE,
                                              DEFAULT_GLOB_PERIOD,
                                              DEFAULT_GLOB_LAMBDA, DEFAULT_GLOB_AMPLITUDE, GLOB_PHI));
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

typedef struct DopplerPoint {
    int x, r, a;
    bool hit;
} DopplerPoint;

void draw_scene_doppler()
{
#define DOPPLER_LAMBDA 45
#define DP_BUFFER_SIZE 16

#define DP_SRC_Y (CONFIG_WINDOW_HEIGHT/2)

#define DP_LISTENER_X (CONFIG_WINDOW_WIDTH/2)
#define DP_LISTENER_Y (CONFIG_WINDOW_HEIGHT/2)

#define DP_GRAPH_SIZE 50

    static DopplerPoint buffer[DP_BUFFER_SIZE];
    static int buffer_idx = 0;
    static int sound_src_pos = 0;
    static int graph[DP_GRAPH_SIZE] = {0};
    static int graph_idx;
    static int prev_hit_t = -1;
    static int t = 0;
    t = (t + 1) % INT_MAX;

    sound_src_pos = (sound_src_pos + DOPPLER_V) % CONFIG_WINDOW_WIDTH;
    filledCircleRGBA(renderer, sound_src_pos, DP_SRC_Y, 20, 255, 0, 0, 255); // source

    if (t % (DOPPLER_LAMBDA) == 0) {
        buffer[buffer_idx].x = sound_src_pos;
        buffer[buffer_idx].r = 1;
        buffer[buffer_idx].a = 255;
        buffer[buffer_idx].hit = false;
        buffer_idx = (buffer_idx + 1) % (DP_BUFFER_SIZE);
    }

    for (int i = 0; i < DP_BUFFER_SIZE; i++) {
        if (buffer[i].a != 0 && t % 5 == 0) {
            buffer[i].a -= 5;
        }

        int x_dist = abs(DP_LISTENER_X - buffer[i].x);
        int y_dist = abs(DP_LISTENER_Y - DP_SRC_Y);
        int p = sqrt(x_dist*x_dist + y_dist*y_dist);
        if (!buffer[i].hit && abs(p - buffer[i].r) < DOPPLER_WAVE_SPEED) {
            buffer[i].hit = true;

            if (prev_hit_t > 0) {
                graph[graph_idx] = (int)(1/(double)(t - prev_hit_t) * 400) - 10;

                graph_idx = (graph_idx + 1) % DP_GRAPH_SIZE;
            }

            prev_hit_t = t;
        }

#define DP_GR_X1 (CONFIG_WINDOW_WIDTH-400)
#define DP_GR_Y1 (CONFIG_WINDOW_HEIGHT-350)
#define DP_GR_WIDTH 300
#define DP_GR_HEIGHT 300
#define DP_GR_STEP (DP_GR_WIDTH/DP_GRAPH_SIZE)


        render_text("f [hz]", DP_GR_X1 - 75, DP_GR_Y1, font_small);
        render_text("t [s]", DP_GR_X1, DP_GR_Y1 + DP_GR_HEIGHT, font_small);

        lineColor(renderer, DP_GR_X1, DP_GR_Y1, DP_GR_X1, DP_GR_Y1+DP_GR_HEIGHT, 0xFFFFFFFF);
        lineColor(renderer, DP_GR_X1, DP_GR_Y1+DP_GR_HEIGHT, DP_GR_X1+DP_GR_WIDTH, DP_GR_Y1+DP_GR_HEIGHT, 0xFFFFFFFF);
             
        for (int g = 0; g < graph_idx-1; g++) {
            lineRGBA(renderer,
                     DP_GR_X1 + DP_GR_STEP*g, DP_GR_Y1 + DP_GR_HEIGHT/2 - 5*graph[g],
                     DP_GR_X1 + DP_GR_STEP*(g+1), DP_GR_Y1 + DP_GR_HEIGHT/2 - 5*graph[g+1],
                     255, 0, 0, 255);
        }

        if (buffer[i].a > 0) {
            aacircleRGBA(renderer, buffer[i].x, DP_SRC_Y, buffer[i].r,
                         255, 255, 255, buffer[i].a);

            buffer[i].r += DOPPLER_WAVE_SPEED;
        }

    }

    filledCircleRGBA(renderer, DP_LISTENER_X, DP_LISTENER_Y, 20, 0, 255, 0, 255); // listener
}

// this is horrible and ugly but idk how to ensure consistent indexes for passing ptrs to .data (enum??)
#define BASIC_LAMBDA_SLIDER 0
#define BASIC_AMPLITUDE_SLIDER 1
#define BASIC_TIME_SLIDER 2
#define BASIC_POINT_SLIDER 3

#define INTERF_OFFSET 0
#define INTERF_TIME_SLIDER 1

#define DOPPLER_V_SLIDER 0
#define DOPPLER_WAVE_SPEED_SLIDER 1

Scene SCENES[] = {
    [SCENE_MENU] = {
        .drawfn = draw_scene_menu,
        .widgets = {
            {
                .widget_type = WIDGET_BUTTON,
                .x1 = CONFIG_WINDOW_WIDTH/2 - 150, .y1 = 350,
                .x2 = CONFIG_WINDOW_WIDTH/2 + 150, .y2 = 440,
                .label = "fn. falowa",
                .callback = callback_switch_scene,
                .callback_data = &SCENES[SCENE_BASIC_WAVE_FUNC],
            },
            {
                .widget_type = WIDGET_BUTTON,
                .x1 = CONFIG_WINDOW_WIDTH/2 - 150, .y1 = 460,
                .x2 = CONFIG_WINDOW_WIDTH/2 + 150, .y2 = 540,
                .label = "interferencja",
                .callback = callback_switch_scene,
                .callback_data = &SCENES[SCENE_INTERFERENCE],
            },
            {
                .widget_type = WIDGET_BUTTON,
                .x1 = CONFIG_WINDOW_WIDTH/2 - 150, .y1 = 560,
                .x2 = CONFIG_WINDOW_WIDTH/2 + 150, .y2 = 640,
                .label = "doppler",
                .callback = callback_switch_scene,
                .callback_data = &SCENES[SCENE_DOPPLER],
            },
            {
                .widget_type = WIDGET_END
            }
        }
    },
    [SCENE_DOPPLER] = {
        .drawfn =  draw_scene_doppler,
        .widgets = {
            [DOPPLER_V_SLIDER] = {
                .widget_type = WIDGET_SLIDER,
                .x1 = 1100, .y1 = 10,
                .x2 = 1300, .y2 = 150,
                .label = "[v] źródła",
                .slider_min = 0, .slider_max = 5,
                .slider_value = DEFAULT_DOPPLER_V, .slider_var = &DOPPLER_V,
                .callback = callback_slider_setvar_int,
                .callback_data = &SCENES[SCENE_DOPPLER].widgets[DOPPLER_V_SLIDER]
            },
            [DOPPLER_WAVE_SPEED_SLIDER] = {
                .widget_type = WIDGET_SLIDER,
                .x1 = 800, .y1 = 10,
                .x2 = 1000, .y2 = 150,
                .label = "[v] fali",
                .slider_min = 1, .slider_max = 3,
                .slider_value = DEFAULT_DOPPLER_WAVE_SPEED, .slider_var = &DOPPLER_WAVE_SPEED,
                .callback = callback_slider_setvar_int,
                .callback_data = &SCENES[SCENE_DOPPLER].widgets[DOPPLER_WAVE_SPEED_SLIDER]
            },
            {
                .widget_type = WIDGET_BUTTON,
                .x1 = 000, .y1 = 0,
                .x2 = 240, .y2 = 80,
                .label = "< Menu",
                .callback = callback_switch_scene,
                .callback_data = &SCENES[SCENE_MENU],
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
                .label = "przesunięcie",
                .slider_min = 0, .slider_max = PI*2,
                .slider_value = 0, .slider_var = &GLOB_PHI,
                .callback = callback_slider_setvar_double,
                .callback_data = &SCENES[SCENE_INTERFERENCE].widgets[INTERF_OFFSET] // this widget
            },
            [INTERF_TIME_SLIDER] = {
                .widget_type = WIDGET_SLIDER,
                .x1 = 1100, .y1 = 10,
                .x2 = 1300, .y2 = 150,
                .label = "upływ czasu",
                .slider_min = 0, .slider_max = 1,
                .slider_value = DEFAULT_TIME_STEP, .slider_var = &TIME_STEP,
                .callback = callback_slider_setvar_double,
                .callback_data = &SCENES[SCENE_INTERFERENCE].widgets[INTERF_TIME_SLIDER] 
            },
            {
                .widget_type = WIDGET_BUTTON,
                .x1 = 0, .y1 = 0,
                .x2 = 240, .y2 = 80,
                .label = "< Menu",
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
                .slider_value = DEFAULT_GLOB_LAMBDA, .slider_var = &GLOB_LAMBDA,
                .callback = callback_slider_setvar_double,
                .callback_data = &SCENES[SCENE_BASIC_WAVE_FUNC].widgets[BASIC_LAMBDA_SLIDER] 
            },
            [BASIC_AMPLITUDE_SLIDER] = {
                .widget_type = WIDGET_SLIDER,
                .x1 = 850, .y1 = 10,
                .x2 = 1050, .y2 = 150,
                .label = "amplituda",
                .slider_min = 0, .slider_max = 5,
                .slider_value = DEFAULT_GLOB_AMPLITUDE, .slider_var = &GLOB_AMPLITUDE,
                .callback = callback_slider_setvar_double,
                .callback_data = &SCENES[SCENE_BASIC_WAVE_FUNC].widgets[BASIC_AMPLITUDE_SLIDER] 
            },
            [BASIC_TIME_SLIDER] = {
                .widget_type = WIDGET_SLIDER,
                .x1 = 1100, .y1 = 10,
                .x2 = 1300, .y2 = 150,
                .label = "upływ czasu",
                .slider_min = 0, .slider_max = 1,
                .slider_value = DEFAULT_TIME_STEP, .slider_var = &TIME_STEP,
                .callback = callback_slider_setvar_double,
                .callback_data = &SCENES[SCENE_BASIC_WAVE_FUNC].widgets[BASIC_TIME_SLIDER] 
            },
            [BASIC_POINT_SLIDER] = {
                .widget_type = WIDGET_SLIDER,
                .x1 = 1100, .y1 = CONFIG_WINDOW_HEIGHT-150,
                .x2 = 1300, .y2 = CONFIG_WINDOW_HEIGHT-40,
                .label = "sym. punkty",
                .slider_min = 10, .slider_max = 500,
                .slider_value = DEFAULT_GLOB_WAVE_POINTS, .slider_var = &GLOB_WAVE_POINTS,
                .callback = callback_slider_setvar_int,
                .callback_data = &SCENES[SCENE_BASIC_WAVE_FUNC].widgets[BASIC_POINT_SLIDER] 
            },
            {
                .widget_type = WIDGET_BUTTON,
                .x1 = 0, .y1 = 0,
                .x2 = 240, .y2 = 80,
                .label = "< Menu",
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
