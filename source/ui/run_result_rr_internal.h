#ifndef RUN_RESULT_RR_INTERNAL_H
#define RUN_RESULT_RR_INTERNAL_H

#include <SDL.h>
#include <stdbool.h>

/*
 * Internal API for the arrow subsystem to position the Level 0 error arrow
 * next to the dynamically positioned failure Back button. Valid only after
 * rr_initialize() succeeds. Run Result layout is immutable at runtime; any
 * future runtime layout setter must reinitialize the error arrow.
 */
bool rr_get_failure_back_button_rect(SDL_Rect *button_rect);

#endif
