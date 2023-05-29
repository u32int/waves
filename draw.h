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
    SCENE_END
} SceneEnum;

extern Scene SCENES[];

void draw_scene(Scene *scene);
void trigger_widget(int x, int y);

#endif /* _DRAW_H */
