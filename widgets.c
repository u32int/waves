#include "widgets.h"
#include "simstate.h"
#include "config.h"
#include "draw.h"
#include "utils.h"

#include <assert.h>

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern TTF_Font *font;

void callback_switch_scene(void *data)
{
    SIM_STATE.sel_scene = data;
}

void callback_slider_setvar_double(void *data)
{
    Widget *slider = (Widget *)data;
    assert(slider && slider->widget_type == WIDGET_SLIDER);

    *(double *)slider->slider_var = slider->slider_value;
}

void callback_slider_setvar_int(void *data)
{
    Widget *slider = (Widget *)data;
    assert(slider && slider->widget_type == WIDGET_SLIDER);

    *(int *)slider->slider_var = (int)slider->slider_value;
}

void widget_draw_button(const char *label,
                   int x1, int y1, int x2, int y2)
{
    rectangleColor(renderer, x1, y1, x2, y2, 0xFFFFFFFF);

    int button_width = x2-x1;
    int button_height = y2-y1;

    int text_width = strlen(label)*22;

    render_text(label, x1 + button_width/2 - text_width/2, y1 + button_height/2 - button_height/3, font);
}

void widget_draw_slider(const char *label,
                        int x1, int y1, int x2, int y2,
                        double slider_min, double slider_max, double slider_value)
{
    int slider_width  = x2-x1;
    int slider_height = y2-y1;
    const int thickness = 5;

    int progress = slider_width * fabs((slider_value-slider_min)/(slider_max-slider_min));

    // draw bar
    boxColor(renderer,
             x1, y1+slider_height/2-thickness,
             x2, y1+slider_height/2+thickness,
             0xFFFFFFFF);

    // draw slider 'caret' (?)
    boxRGBA(renderer,
            x1+progress,             y1+slider_height/2-thickness*5,
            x1+progress+thickness*2, y1+slider_height/2+thickness*5,
            100, 100, 100, 255);

    render_text(label, x1, y1-10, font);

    char buff[32];
    snprintf(buff, 32, "%.2lf", slider_value);
    render_text(buff, x1, y2-10, font);
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

#define PAD 10
        if (x > scene->widgets[i].x1 - PAD && y > scene->widgets[i].y1 &&
            x < scene->widgets[i].x2 + PAD && y < scene->widgets[i].y2) {

            double value_range = scene->widgets[i].slider_max - scene->widgets[i].slider_min;
            double slider_width = scene->widgets[i].x2 - scene->widgets[i].x1;

            int cursor_offset = clamp_int(x - scene->widgets[i].x1,
                                          0, slider_width);

            scene->widgets[i].slider_value = scene->widgets[i].slider_min +
                value_range * ((double)cursor_offset/slider_width);

            if (!scene->widgets[i].callback)
                continue;
            
            scene->widgets[i].callback(scene->widgets[i].callback_data);
        }
    }
}


void widget_trigger(int x, int y)
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
