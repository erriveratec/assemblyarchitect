#ifndef HIGHLIGHT_ANIMATION_HA_H
#define HIGHLIGHT_ANIMATION_HA_H

#include <stdbool.h>

/* The owning primary gameplay render path must call ha_begin_frame() exactly
 * once per logical gameplay render frame, before drawing highlight consumers.
 * That call captures one immutable phase snapshot and advances the clock for
 * the next frame. Secondary render passes must only sample the captured phase
 * and must never call ha_begin_frame(). All highlight consumers and mapped
 * arrows sample the same normalized progress; each target applies its own
 * visual amplitude. Getters are side-effect free.
 */
void ha_reset(void);
void ha_begin_frame(void);
float ha_get_progress(void);
float ha_get_value(float animation_limit);
bool ha_is_descending(void);

#endif