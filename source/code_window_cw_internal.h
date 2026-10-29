#ifndef CODE_WINDOW_CW_INTERNAL_H
#define CODE_WINDOW_CW_INTERNAL_H

#include <math.h>
#include <stdbool.h>
#include <limits.h>
#include <stdint.h>
#include <SDL.h>
#include "code_line_cl.h"
#include "text_tx.h"

static inline bool cw_calculate_challenge_frame_rect(
	SDL_Rect text_bounds, SDL_Rect container, int allowance, SDL_Rect *frame)
{
	if (frame == NULL || text_bounds.w <= 0 || text_bounds.h <= 0 ||
	    container.w <= 0 || container.h <= 0 || allowance < 0) {
		return false;
	}
	int64_t left = (int64_t)text_bounds.x - allowance;
	int64_t top = (int64_t)text_bounds.y - allowance;
	int64_t right = (int64_t)text_bounds.x + text_bounds.w + allowance;
	int64_t bottom = (int64_t)text_bounds.y + text_bounds.h + allowance;
	int64_t container_right = (int64_t)container.x + container.w;
	int64_t container_bottom = (int64_t)container.y + container.h;
	if (left < container.x) left = container.x;
	if (top < container.y) top = container.y;
	if (right > container_right) right = container_right;
	if (bottom > container_bottom) bottom = container_bottom;
	if (right <= left || bottom <= top || left < INT_MIN || top < INT_MIN ||
	    right > INT_MAX || bottom > INT_MAX || right - left > INT_MAX ||
	    bottom - top > INT_MAX) {
		return false;
	}
	*frame = (SDL_Rect){
		.x = (int)left,
		.y = (int)top,
		.w = (int)(right - left),
		.h = (int)(bottom - top)
	};
	return true;
}

#ifdef CW_CHALLENGE_TEXT_TESTING
typedef tx_styled_text_t *(*cw_challenge_text_factory_t)(
	int width, int text_height, const char *text,
	const tx_styled_text_options_t *options);

/* Challenge-specific creation seam for lifecycle tests; NULL resets to the
 * production factory. Kept internal so gameplay callers cannot change it. */
void cw_set_challenge_text_factory(cw_challenge_text_factory_t factory);
void cw_reset_challenge_text_factory(void);
#endif

static inline bool cw_is_active_operand_replacement_slot(
	const code_line_t *line,
	int changing_state,
	const code_line_t *pending_line)
{
	return line != NULL && line == pending_line &&
		line->state == changing_state;
}

static inline bool cw_is_active_jump_operand_replacement(
	const code_line_t *line,
	const code_line_t *pending_line)
{
	return line != NULL && line == pending_line &&
		line->state == CHANGING_OP1 && line->ins != NULL &&
		cl_is_ins_jmp_type(line->ins->id);
}

static inline bool cw_should_draw_pending_operand_rect(
	const code_line_t *line,
	const code_line_t *pending_line)
{
	if (line == NULL || line != pending_line) {
		return false;
	}
	if (line->state == MISSING_BOTH || line->state == MISSING_OP1 ||
	    line->state == MISSING_OP2) {
		return true;
	}
	if (line->state == CHANGING_OP1 || line->state == CHANGING_OP2) {
		return !(cw_is_active_jump_operand_replacement(line, pending_line) &&
		         line->op1 != NULL && line->op1->b != NULL);
	}
	return false;
}

static inline bool cw_should_draw_replacement_operand(
	const code_line_t *line,
	int changing_state,
	const code_line_t *pending_line)
{
	return !cw_is_active_operand_replacement_slot(
		line, changing_state, pending_line) ||
		(changing_state == CHANGING_OP1 &&
		 cw_is_active_jump_operand_replacement(line, pending_line));
}

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