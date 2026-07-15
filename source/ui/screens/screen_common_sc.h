#ifndef SCREEN_COMMON_SC_H
#define SCREEN_COMMON_SC_H

#include <stdbool.h>
#include <SDL.h>
#include "electron_fx.h"

typedef struct sc_fx_t {
    fx_electron_t *electron;
    Uint64 anim_prev_ms;
    bool initialized;
} sc_fx_t;

void sc_fx_init(sc_fx_t *fx, Uint64 cur_time, int width, int height);
void sc_fx_update_render(sc_fx_t *fx, Uint64 cur_time);
void sc_fx_destroy(sc_fx_t *fx);

#endif



