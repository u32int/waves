#include "widgets.h"
#include "simstate.h"
#include "draw.h"

#include <assert.h>

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern TTF_Font *font;

void callback_switch_scene(void *data)
{
    SIM_STATE.sel_scene = data;
}

void callback_slider_setvar(void *data)
{
    Widget *slider = (Widget *)data;
    assert(slider && slider->widget_type == WIDGET_SLIDER);

    *slider->slider_var = slider->slider_value;
}

void widget_draw_button(const char *label,
                   int x1, int y1, int x2, int y2)
{
    rectangleColor(renderer, x1, y1, x2, y2, 0xFFFFFFFF);

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

void widget_draw_slider(const char *label,
                        int x1, int y1, int x2, int y2,
                        double slider_min, double slider_max, double slider_value)
{
    int width  = x2-x1;
    int height = y2-y1;
    const int thickness = 5;

    int progress = width * (slider_value/slider_max);

    // draw bar
    boxColor(renderer,
             x1, y1+height/2-thickness,
             x2, y1+height/2+thickness,
             0xFFFFFFFF);

    // draw slider 'caret' (?)
    boxRGBA(renderer,
            x1+progress,             y1+height/2-thickness*5,
            x1+progress+thickness*2, y1+height/2+thickness*5,
            100, 100, 100, 255);

    SDL_Color color = {255, 255, 255};
    SDL_Surface *surface = TTF_RenderText_Solid(font, label, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect rect;
    rect.x = x1 + width/4; 
    rect.y = y1; 
    rect.w = abs(x1-x2) - width/2; 
    rect.h = height/2-thickness*2;

    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void draw_widget(Widget *widget)
{
    switch (widget->widget_type) {
    case WIDGET_BUTTON:
        widget_draw_button(widget->label,
                           widget->x1, widget->y1,
                           widget->x2, widget->y2);
        break;
    case WIDGET_SLIDER:
        widget_draw_slider(widget->label,
                           widget->x1, widget->y1,
                           widget->x2, widget->y2,
                           widget->slider_min, widget->slider_max, widget->slider_value);
    default:
        break;
    }
}

void widget_update_sliders(int x, int y)
{
    Scene *scene = SIM_STATE.sel_scene;

    for (int i = 0; i < CONFIG_MAX_WIDGETS; i++) {
        if (scene->widgets[i].widget_type != WIDGET_SLIDER)
            break;

        // I'm sorry (I promise this kinda works)
        if (x > scene->widgets[i].x1 && y > scene->widgets[i].y1 &&
            x < scene->widgets[i].x2 && y < scene->widgets[i].y2) {

            int value_range = scene->widgets[i].slider_max - scene->widgets[i].slider_min;
            int slider_width = scene->widgets[i].x2 - scene->widgets[i].x1;
            int cursor_offset = x - scene->widgets[i].x1;

            scene->widgets[i].slider_value = scene->widgets[i].slider_min +
                value_range * ((double)cursor_offset/slider_width);

            if (!scene->widgets[i].callback)
                continue;
            
            scene->widgets[i].callback(scene->widgets[i].callback_data);
        }
    }
}
