#ifndef _WIDGETS_H
#define _WIDGETS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

typedef enum WidgetEnum {
    WIDGET_BUTTON,
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
            int slider_min, slider_max, slider_step;
        };
    };
} Widget;

void callback_switch_scene(void *data);

void draw_widget(Widget *widget);
void widget_draw_button(const char *label, int x1, int y1, int x2, int y2);

#endif /* _WIDGETS_H */
