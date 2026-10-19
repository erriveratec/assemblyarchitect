#ifndef CODE_WINDOW_CW_INTERNAL_H
#define CODE_WINDOW_CW_INTERNAL_H

#include <math.h>
#include <stdbool.h>
#include <SDL.h>

/* Internal pure geometry helper, shared with synchronization tests. */
static inline bool cw_calculate_pending_operand_rect(
	SDL_Rect base_operand_rect,
	float normalized_progress,
	int animation_limit,
	SDL_Rect *animated_operand_rect)
{
	if (animated_operand_rect == NULL ||
	    !isfinite(normalized_progress) || base_operand_rect.w <= 0 ||
	    base_operand_rect.h <= 0) {
		return false;
	}

	if (animation_limit <= 0) {
		*animated_operand_rect = base_operand_rect;
		return true;
	}

	/* Phase 0 is the minimum square; phase 1 is the maximum square. */
	float progress = fmaxf(0.0f, fminf(1.0f, normalized_progress));
	float maximum_inset = fminf((float)animation_limit,
	                            fminf((float)(base_operand_rect.w - 1),
	                                  (float)(base_operand_rect.h - 1)));
	float current_inset = maximum_inset * (1.0f - progress);
	SDL_FRect rect = {
		.x = (float)base_operand_rect.x + current_inset / 2.0f,
		.y = (float)base_operand_rect.y + current_inset / 2.0f,
		.w = fmaxf(1.0f, (float)base_operand_rect.w - current_inset),
		.h = fmaxf(1.0f, (float)base_operand_rect.h - current_inset)
	};
	*animated_operand_rect = (SDL_Rect){
		.x = (int)lroundf(rect.x),
		.y = (int)lroundf(rect.y),
		.w = (int)lroundf(rect.w),
		.h = (int)lroundf(rect.h)
	};
	if (animated_operand_rect->w < 1) animated_operand_rect->w = 1;
	if (animated_operand_rect->h < 1) animated_operand_rect->h = 1;
	return true;
}

#endif