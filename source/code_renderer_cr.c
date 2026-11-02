#include "code_renderer_cr.h"

#include "aux.h"
#include "code_line_cl.h"
#include "code_layout_cly.h"
#include "gameplay/highlight_animation_ha.h"
#include "ui/ui_metrics_um.h"
#include "ui/typography_ty.h"
#include "draw_dw.h"
#include <math.h>

#include <stdlib.h>

typedef struct jump_target_label_buttons_t {
	btn_t *instruction_button;
	btn_t *operand_button;
} jump_target_label_buttons_t;

static bool cr_is_active_jump_replacement(
	const code_line_t *line, const code_line_t *pending_line)
{
	return line != NULL && line == pending_line &&
		line->state == CHANGING_OP1 && line->ins != NULL &&
		cl_is_ins_jmp_type(line->ins->id);
}

static bool cr_should_draw_replacement_operand(
	const code_line_t *line, int changing_state,
	const code_line_t *pending_line)
{
	bool active_slot = line != NULL && line == pending_line &&
		line->state == changing_state;
	return !active_slot || (changing_state == CHANGING_OP1 &&
		cr_is_active_jump_replacement(line, pending_line));
}

static jump_target_label_buttons_t cr_jump_target_buttons(
	const code_line_t *line)
{
	jump_target_label_buttons_t buttons = {0};
	if (line == NULL || line->ins == NULL ||
	    !cl_is_ins_jmp_type(line->ins->id) || line->op1 == NULL ||
	    line->op1->jptr == NULL || line->op1->jptr->ins == NULL ||
	    line->op1->jptr->ins->id != LABEL) {
		return buttons;
	}
	code_line_t *target = line->op1->jptr;
	buttons.instruction_button = target->ins->b;
	if (target->op1 != NULL) {
		buttons.operand_button = target->op1->b;
	}
	return buttons;
}

static bool cr_button_is_claimed(
	const btn_t *button, const cr_visual_state_t *visual_state)
{
	if (button == NULL || visual_state == NULL) {
		return false;
	}
	for (int index = 0; index < CR_JUMP_HIGHLIGHT_COMPONENT_COUNT; index++) {
		if (visual_state->jump_claim_buttons[index] == button) {
			return true;
		}
	}
	return false;
}

static bool cr_active_target_label_group(
	const code_line_t *line, code_line_t *pending_line,
	const cr_visual_state_t *visual_state,
	jump_target_label_buttons_t *buttons)
{
	if (buttons == NULL || visual_state == NULL) {
		return false;
	}
	*buttons = (jump_target_label_buttons_t){0};
	if (!cr_is_active_jump_replacement(pending_line, pending_line) ||
	    pending_line->op1 == NULL || pending_line->op1->b == NULL ||
	    pending_line->op1->jptr != line) {
		return false;
	}
	jump_target_label_buttons_t target_buttons =
		cr_jump_target_buttons(pending_line);
	if (target_buttons.instruction_button != NULL &&
	    visual_state->jump_claim_buttons[1] ==
		target_buttons.instruction_button) {
		buttons->instruction_button = target_buttons.instruction_button;
	}
	if (target_buttons.operand_button != NULL &&
	    visual_state->jump_claim_buttons[2] == target_buttons.operand_button) {
		buttons->operand_button = target_buttons.operand_button;
	}
	return buttons->instruction_button != NULL || buttons->operand_button != NULL;
}

static void cr_draw_button(
	btn_t *button, const cr_visual_state_t *visual_state, bool blocked)
{
	if (button == NULL) {
		return;
	}
	if (cr_button_is_claimed(button, visual_state)) {
		bt_draw_btn_with_highlight_progress(
			button, blocked, false, ha_get_progress());
	} else {
		bt_draw_btn(button, blocked, false);
	}
}

static bool cr_is_group_button(
	const btn_t *button, jump_target_label_buttons_t buttons)
{
	return button != NULL &&
		(button == buttons.instruction_button || button == buttons.operand_button);
}

void cr_draw_code_line(
	code_line_t *line, code_line_t *pending_line,
	const cr_visual_state_t *visual_state, bool blocked)
{
	if (line == NULL) {
		return;
	}
	jump_target_label_buttons_t label_buttons;
	bool grouped_label = cr_active_target_label_group(
		line, pending_line, visual_state, &label_buttons);
	if (grouped_label) {
		btn_t *reference_button = visual_state->jump_claim_buttons[0];
		float reference_scale;
		bt_group_draw_item_t items[] = {
			{label_buttons.instruction_button, blocked, false},
			{label_buttons.operand_button, blocked, false}
		};
		if (bt_calculate_button_highlight_scale(
			reference_button, ha_get_progress(), &reference_scale) &&
		    bt_draw_btn_group_with_highlight_scale(
				items, sizeof(items) / sizeof(items[0]), reference_scale,
				BT_HIGHLIGHT_ANCHOR_LEFT)) {
			/* The grouped helper drew both label buttons. */
		} else {
			cr_draw_button(label_buttons.instruction_button, visual_state, blocked);
			cr_draw_button(label_buttons.operand_button, visual_state, blocked);
		}
	}

	btn_t *instruction_button = line->ins != NULL ? line->ins->b : NULL;
	if (!grouped_label || !cr_is_group_button(instruction_button, label_buttons)) {
		cr_draw_button(instruction_button, visual_state, blocked);
	}
	if (line->op1 != NULL &&
	    (!grouped_label || !cr_is_group_button(line->op1->b, label_buttons)) &&
	    cr_should_draw_replacement_operand(line, CHANGING_OP1, pending_line)) {
		cr_draw_button(line->op1->b, visual_state, blocked);
	}
	if (line->op2 != NULL &&
	    cr_should_draw_replacement_operand(line, CHANGING_OP2, pending_line)) {
		cr_draw_button(line->op2->b, visual_state, blocked);
	}
}

void cr_draw_code_lines(
	const code_program_t *program, const cr_assets_t *assets,
	code_line_t *pending_line, const cr_visual_state_t *visual_state,
	int comma_offset, bool blocked)
{
	if (program == NULL) {
		return;
	}
	SDL_Rect button_size = cl_get_code_button_size();
	for (int index = 0; index < cp_count(program); index++) {
		code_line_t *line = cp_get(program, index);
		if (line == NULL) {
			continue;
		}
		cr_draw_code_line(line, pending_line, visual_state, blocked);
		if (assets != NULL && assets->comma != NULL && line->ins != NULL &&
		    line->ins->b != NULL &&
		    cl_get_instruction_operand_quantity(line->ins->id) == TWO_OPERANDS &&
		    line->state != MISSING_BOTH && line->state != MISSING_OP1) {
			SDL_Rect comma_rect = {
				.x = line->ins->b->r.x + comma_offset,
				.y = line->ins->b->r.y,
				.h = button_size.h
			};
			dw_draw_texture_fit_h(comma_rect, assets->comma);
		}
	}
}

void cr_draw_line_numbers(
	const code_program_t *program, const cr_assets_t *assets,
	int x, int first_y, int spacing, int text_height)
{
	if (program == NULL || assets == NULL || assets->line_numbers == NULL) {
		return;
	}
	int line_number = 1;
	int y = first_y;
	for (int index = 0; index < cp_count(program); index++) {
		code_line_t *line = cp_get(program, index);
		if (line != NULL && line->ins != NULL && line->ins->id != LABEL &&
		    (size_t)line_number < assets->line_number_count) {
			SDL_Rect rect = {.x = x, .y = y, .h = text_height};
			dw_draw_texture_fit_h(rect, assets->line_numbers[line_number]);
			line_number++;
		}
		y += spacing;
	}
}

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
	int content_x_adjustment)
{
	float value = code_box_highlight ? ha_get_value(animation_limit) : 0.0f;
	if (code_box_value != NULL) {
		*code_box_value = value;
	}
	float growth = value * 12.0f;
	float code_box_top = code_box.y - growth / 2.0f;
	float code_box_bottom = code_box.y + code_box.h + growth / 2.0f;
	if (code_box_highlight) {
		SDL_FRect outer = {
			.x = code_box.x, .y = code_box.y,
			.w = code_box.w, .h = code_box.h
		};
		SDL_Rect inner_rect = {
			.x = code_box.x + border_width,
			.y = code_box.y + border_width,
			.w = code_box.w - 2 * border_width,
			.h = code_box.h - 2 * border_width
		};
		SDL_FRect inner = {
			.x = inner_rect.x, .y = inner_rect.y,
			.w = inner_rect.w, .h = inner_rect.h
		};
		dw_draw_filled_rectangle_f(dw_grow_rect_height(outer, growth), C_GREY, C_GREY);
		dw_draw_filled_rectangle_f(dw_grow_rect_height(inner, growth), C_BLACK, C_BLACK);
	} else {
		dw_draw_thick_rect(code_box, border_width, C_GREY);
	}

	cr_draw_code_lines(program, assets, pending_line, visual_state,
	                   comma_offset, blocked);
	int number_x = code_line_x + horizontal_padding + content_x_adjustment;
	cr_draw_line_numbers(program, assets, number_x, first_code_line_y,
	                     code_line_spacing, code_text_height);

	if (challenge_text != NULL && !challenge_highlight) {
		tx_draw_styled_text(text_box, message_text_height, challenge_text, 0);
		dw_draw_rectangle(text_box, C_GREY);
	} else if (challenge_text != NULL) {
		float pulse = ha_get_value(animation_limit);
		if (challenge_value != NULL) {
			*challenge_value = pulse;
		}
		SDL_Rect challenge_bounds;
		if (tx_get_styled_text_pulsed_bounds(
				text_box, message_text_height, pulse, challenge_text,
				&challenge_bounds)) {
			SDL_Rect frame;
			tx_draw_styled_text_pulsed(text_box, message_text_height, pulse,
			                           challenge_text);
			if (cly_calculate_challenge_frame_rect(
					challenge_bounds, code_box, border_width, &frame)) {
				dw_draw_rectangle(frame, C_GREY);
			}
		}
	}
	if (!challenge_highlight && challenge_value != NULL) {
		*challenge_value = 0.0f;
	}

	SDL_Rect stage_title = {
		.x = scroll_box.x + horizontal_padding,
		.y = scroll_box.y + horizontal_padding,
		.h = emphasis_text_height
	};
	dw_draw_texture_fit_h(stage_title, stage_name);
	SDL_Rect upper_mask = {
		.x = code_box.x, .y = 0, .w = code_box.w,
		.h = code_box_top > 0.0f ? (int)code_box_top : 0
	};
	SDL_Rect lower_mask = {
		.x = code_box.x, .y = (int)ceilf(code_box_bottom),
		.w = code_box.w, .h = code_box.h
	};
	dw_draw_filled_rectangle(upper_mask, C_BLACK, C_BLACK);
	dw_draw_filled_rectangle(lower_mask, C_BLACK, C_BLACK);
}

bool cr_assets_init(cr_assets_t *assets, size_t line_number_count)
{
	if (assets == NULL || line_number_count == 0) {
		return false;
	}
	cr_assets_destroy(assets);
	assets->comma = dw_create_text_tex(",", C_WHITE);
	assets->line_numbers = calloc(line_number_count, sizeof(*assets->line_numbers));
	assets->line_number_count = line_number_count;
	if (assets->comma == NULL || assets->line_numbers == NULL) {
		cr_assets_destroy(assets);
		return false;
	}
	for (size_t index = 0; index < line_number_count; index++) {
		char *number = ax_number_to_string_two_digits((int)index);
		if (number == NULL) {
			cr_assets_destroy(assets);
			return false;
		}
		assets->line_numbers[index] = dw_create_text_tex(number, C_GREY);
		free(number);
		if (assets->line_numbers[index] == NULL) {
			cr_assets_destroy(assets);
			return false;
		}
	}
	return true;
}

void cr_assets_destroy(cr_assets_t *assets)
{
	if (assets == NULL) {
		return;
	}
	if (assets->line_numbers != NULL) {
		for (size_t index = 0; index < assets->line_number_count; index++) {
			dw_free_texture(assets->line_numbers[index]);
		}
		free(assets->line_numbers);
	}
	dw_free_texture(assets->comma);
	*assets = (cr_assets_t){0};
}
