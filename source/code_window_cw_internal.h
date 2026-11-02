#ifndef CODE_WINDOW_CW_INTERNAL_H
#define CODE_WINDOW_CW_INTERNAL_H

#include <stdbool.h>
#include <SDL.h>
#include "code_line_cl.h"
#include "code_layout_cly.h"
#include "code_program_cp.h"
#include "text_tx.h"

bool cw_calculate_challenge_frame_rect(
	SDL_Rect text_bounds, SDL_Rect container, int allowance, SDL_Rect *frame);

#ifdef CW_PROGRAM_TESTING
code_program_t *cw_get_program_for_testing(void);
#endif

#ifdef CW_CHALLENGE_TEXT_TESTING
typedef tx_styled_text_t *(*cw_challenge_text_factory_t)(
	int width, int text_height, const char *text,
	const tx_styled_text_options_t *options);

/* Challenge-specific creation seam for lifecycle tests; NULL resets to the
 * production factory. Kept internal so gameplay callers cannot change it. */
void cw_set_challenge_text_factory(cw_challenge_text_factory_t factory);
void cw_reset_challenge_text_factory(void);
tx_styled_text_t *cw_get_challenge_text_for_testing(void);
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

/* Stage 3 compatibility adapter; remove with the remaining Stage 11 wrappers. */
static inline bool cw_calculate_pending_operand_rect(
	SDL_Rect base_operand_rect,
	float normalized_progress,
	int animation_limit,
	SDL_Rect *animated_operand_rect)
{
	return cly_calculate_pending_operand_rect(
		base_operand_rect, normalized_progress, animation_limit,
		animated_operand_rect);
}

#endif
