#include "ui/screens/screen_common_sc.h"
#include "sdl_config.h"
//#include "aux.h"

void sc_fx_init(sc_fx_t *fx, Uint64 cur_time, int width, int height)
{
    if (fx == NULL || fx->initialized) {
        return;
    }

    fx->electron = fx_electron_create(g_renderer, width, height, NULL);
    fx->anim_prev_ms = cur_time;
    fx->initialized = true;
}

void sc_fx_update_render(sc_fx_t *fx, Uint64 cur_time)
{
    if (fx == NULL || fx->initialized == false || fx->electron == NULL) {
        return;
    }

    float dt = (cur_time - fx->anim_prev_ms) / 1000.0f;
    fx->anim_prev_ms = cur_time;

    fx_electron_update(fx->electron, dt);
    fx_electron_render(fx->electron, g_renderer);
}

void sc_fx_destroy(sc_fx_t *fx)
{
    if (fx == NULL) {
        return;
    }

    if (fx->electron != NULL) {
        aa_electron_fx_destroy(fx->electron);
        fx->electron = NULL;
    }

    fx->anim_prev_ms = 0;
    fx->initialized = false;
}
