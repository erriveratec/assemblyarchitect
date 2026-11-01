#ifndef SDL_CONFIG_INTERNAL_H
#define SDL_CONFIG_INTERNAL_H

#include <stdbool.h>
#include <SDL.h>

#include "aux.h"

#define SDL_PREFERRED_MIN_WINDOW_WIDTH 1280
#define SDL_PREFERRED_MIN_WINDOW_HEIGHT 720

typedef struct sdl_window_size_limits {
	int minimum_width;
	int minimum_height;
} sdl_window_size_limits_t;

bool sdl_calculate_adaptive_minimum_size(
	int preferred_width, int preferred_height, SDL_Rect usable_bounds,
	sdl_window_size_limits_t *result);

typedef struct sdl_fullscreen_transition_operations {
	void *context;
	int (*set_fullscreen)(void *context, bool enabled);
	int (*restore_windowed_rect)(void *context);
	int (*apply_display_configuration)(void *context);
	int (*refresh_display_metrics)(void *context);
	void (*log_rollback_failure)(void *context, const char *operation);
} sdl_fullscreen_transition_operations_t;

int sdl_run_fullscreen_transition(
	bool was_fullscreen, bool enabled,
	const sdl_fullscreen_transition_operations_t *operations);

#endif
