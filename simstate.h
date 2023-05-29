#ifndef _SIMSTATE_H
#define _SIMSTATE_H

#include "draw.h"

typedef struct SimState {
    Scene *sel_scene;
} SimState;

extern SimState SIM_STATE;

#endif /* _SIMSTATE_H */
