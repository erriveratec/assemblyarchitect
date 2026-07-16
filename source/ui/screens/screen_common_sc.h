#ifndef SCREEN_COMMON_SC_H
#define SCREEN_COMMON_SC_H

#include <stdbool.h>
#include <SDL.h>
#include "electron_fx.h"

#include "draw_dw.h"
typedef struct sc_fx_t {
    fx_electron_t *electron;
    Uint64 anim_prev_ms;
    bool initialized;
} sc_fx_t;

void sc_fx_init(sc_fx_t *fx, Uint64 cur_time);
void sc_fx_update_render(sc_fx_t *fx, Uint64 cur_time);
void sc_fx_destroy(sc_fx_t *fx);

typedef struct sc_typewriter_t {
    Uint64 last_type_ms;
    size_t type_index;
    bool done;
    texture_t *texture;
} sc_typewriter_t;

void sc_typewriter_reset(sc_typewriter_t *tw);

bool sc_typewriter_update(sc_typewriter_t *tw,
                          Uint64 cur_time,
                          Uint32 delay_ms,
                          SDL_Rect box,
                          const char *text,
                          SDL_Color color);

void sc_typewriter_free(sc_typewriter_t *tw);

#endif

