#ifndef _DRAW_H
#define _DRAW_H

#include "widgets.h"
#include "config.h"

typedef struct Scene {
    void (*drawfn)();
    Widget widgets[CONFIG_MAX_WIDGETS];
} Scene;

typedef enum SceneEnum {
    SCENE_MENU,
    SCENE_BASIC_WAVE_FUNC,
    SCENE_INTERFERENCE,
    SCENE_DOPPLER,
    SCENE_END
} SceneEnum;

extern Scene SCENES[];

void draw_scene(Scene *scene);
void draw_text(const char *label, int x1, int y1, int x2, int y2);

#endif /* _DRAW_H */
