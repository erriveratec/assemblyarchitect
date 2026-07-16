#include "ui/screens/screen_common_sc.h"
#include "sdl_config.h"
#include "dimensions_dm.h"
#include "text_tx.h"

void sc_fx_init(sc_fx_t *fx, Uint64 cur_time)
{
	int W = dm_get_screen_width();
	int H = dm_get_screen_height();
    if (fx == NULL || fx->initialized) {
        return;
    }

    fx->electron = fx_electron_create(g_renderer, W, H, NULL);
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

void sc_typewriter_reset(sc_typewriter_t *tw)
{
    if (tw == NULL) {
        return;
    }

    tw->last_type_ms = 0;
    tw->type_index = 0;
    tw->done = false;
    tw->texture = NULL;
}

bool sc_typewriter_update(sc_typewriter_t *tw,
                          Uint64 cur_time,
                          Uint32 delay_ms,
                          SDL_Rect box,
                          const char *text,
                          SDL_Color color)
{
    if (tw == NULL || text == NULL) {
        return false;
    }

    if (tw->done) {
        return true;
    }

    if ((cur_time - tw->last_type_ms) >= delay_ms) {
        tw->last_type_ms = cur_time;
        bool complete = tx_draw_create_typewriter_text(&tw->texture,
                                                       box,
                                                       text,
                                                       &tw->type_index,
                                                       color);
        if (complete) {
            tw->done = true;
        }
    }
    return tw->done;
}

void sc_typewriter_free(sc_typewriter_t *tw)
{
    if (tw == NULL) {
        return;
    }

    if (tw->texture != NULL) {
        dw_free_texture(tw->texture);
        tw->texture = NULL;
    }

    tw->last_type_ms = 0;
    tw->type_index = 0;
    tw->done = false;
}


