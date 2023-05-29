#ifndef _WIDGETS_H
#define _WIDGETS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

typedef struct SliderSetVar {
    void *var;
    void *value;
} SliderSetVar;

typedef enum WidgetEnum {
    WIDGET_BUTTON,
    WIDGET_SLIDER,
    WIDGET_END,
} WidgetEnum;

typedef struct Widget {
    WidgetEnum widget_type;
    int x1, y1, x2, y2;
    const char *label;

    void (*callback)(void *);
    void *callback_data;

    union {
        // slider widget
        struct {
            double slider_min, slider_max, slider_step;
            double slider_value, *slider_var;
        };
    };
} Widget;

void callback_switch_scene(void *data);
void callback_slider_setvar(void *data);

void draw_widget(Widget *widget);

void widget_update_sliders(int x, int y);

#endif /* _WIDGETS_H */
