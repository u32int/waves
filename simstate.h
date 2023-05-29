#ifndef _SIMSTATE_H
#define _SIMSTATE_H

#include <stdbool.h>
#include "draw.h"

typedef struct SimState {
    bool mouse_down;
    Scene *sel_scene;
} SimState;

extern SimState SIM_STATE;

#endif /* _SIMSTATE_H */
