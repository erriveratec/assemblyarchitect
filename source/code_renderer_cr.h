#ifndef CODE_RENDERER_CR_H
#define CODE_RENDERER_CR_H

#include <stdbool.h>
#include <stddef.h>

#include "code_program_cp.h"
#include "draw_dw.h"
#include "text_tx.h"
#include "ui/button_bt.h"

#define CR_JUMP_HIGHLIGHT_COMPONENT_COUNT 3

typedef struct cr_assets_t {
	texture_t *comma;
	texture_t **line_numbers;
	size_t line_number_count;
} cr_assets_t;

typedef struct cr_visual_state_t {
	btn_t *jump_claim_buttons[CR_JUMP_HIGHLIGHT_COMPONENT_COUNT];
} cr_visual_state_t;

bool cr_assets_init(cr_assets_t *assets, size_t line_number_count);
void cr_assets_destroy(cr_assets_t *assets);
void cr_draw_code_line(
	code_line_t *line, code_line_t *pending_line,
	const cr_visual_state_t *visual_state, bool blocked);
void cr_draw_code_lines(
	const code_program_t *program, const cr_assets_t *assets,
	code_line_t *pending_line, const cr_visual_state_t *visual_state,
	int comma_offset, bool blocked);
void cr_draw_line_numbers(
	const code_program_t *program, const cr_assets_t *assets,
	int x, int first_y, int spacing, int text_height);
void cr_draw_code_window_presentation(
	const code_program_t *program, const cr_assets_t *assets,
	const cr_visual_state_t *visual_state, code_line_t *pending_line,
	int comma_offset, bool blocked, SDL_Rect code_box, SDL_Rect scroll_box,
	SDL_Rect text_box, texture_t *stage_name,
	tx_styled_text_t *challenge_text, bool code_box_highlight,
	bool challenge_highlight, float animation_limit, float *code_box_value,
	float *challenge_value, int code_line_x, int first_code_line_y,
	int code_line_spacing, int code_text_height, int message_text_height,
	int emphasis_text_height, int border_width, int horizontal_padding,
	int content_x_adjustment);

#endif
