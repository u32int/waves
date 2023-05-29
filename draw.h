#ifndef _DRAW_H
#define _DRAW_H

#include "widget.h"

typedef struct Scene {
    void (*drawfn)(void *);
    Widget widgets[32];
} Scene;

void draw(void);

#endif /* _DRAW_H */
