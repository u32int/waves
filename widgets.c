#include "widgets.h"
#include "simstate.h"
#include "draw.h"

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern TTF_Font *font;

void callback_switch_scene(void *data)
{
    SIM_STATE.sel_scene = data;
}

void widget_button(const char *label,
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

void draw_widget(Widget *widget)
{
    switch (widget->widget_type) {
    case WIDGET_BUTTON:
        widget_button(widget->label,
                      widget->x1, widget->y1,
                      widget->x2, widget->y2);
        break;
    default:
        break;
    }
}
