#include <SDL2/SDL2.h>

void widget_button(SDL_Renderer *renderer, const char *label,
                   int x1, int y1, int x2, int y2)
{
    rectangleColor(renderer, x1, y1, x2, y2, 0xFFFFFFFF);
}
