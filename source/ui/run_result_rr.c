#include <stdbool.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include "ui/run_result_rr.h"
#include "ui/run_result_rr_internal.h"
#include "game_mechanics_mc.h"
#include "dimensions_dm.h"
#include "draw_dw.h"
#include "text_tx.h"
#include "ui/button_bt.h"
#include "ui/escape_menu_em.h"
#include "media/audio_au.h"
#include "aux.h"
#include "mouse_ms.h"

#define INPUT_BUFFER_EMPTY_TEXT "A value cannot be recovered if the "\
"Input Buffer [IB] is empty"
#define REG_VALUE_INVALID_TEXT "Register cannot be read if a value "\
"hasn't been stored first"
#define FLAG_VALUE_INVALID_TEXT "FLAG cannot be read if a value "\
"hasn't been stored first"
#define INVALID_OUTPUT_VALUE_TEXT "Incorrect value in the output buffer"
#define UNPROCESSED_IB_VALUES_TEXT "Output is correct but only works by"\
" that specific set of values"
#define EXCEEDS_CODE_LIMIT_TEXT "Correct output but exceeds code size"\
" limit"
#define OUTPUT_BUFFER_INCOMPLETE_TEXT "Not enough "\
"items in the Output Buffer [ob] after run"
#define WIN_TEXT "Execution produced the expected output"
#define RUN_COMPLETED_TEXT "RUN COMPLETED"
#define RUN_FAILED_TEXT "RUN FAILED"

static texture_array_t *ib_empty;
static texture_array_t *reg_val_bad;
static texture_array_t *flag_val_bad;
static texture_array_t *ob_val_bad;
static texture_array_t *ib_unproc_vals;
static texture_array_t *exc_code_size;
static texture_array_t *ob_incomplete;
static texture_array_t *g_win_text;
static texture_t *g_run_completed;
static texture_t *g_run_failed;
static iface_btn_t *g_result_back_button;
static iface_btn_t *g_result_continue_button;
static int g_presented_operation_id = NO_OPERATION;
static bool g_result_sound_played;
static bool g_initialized;

typedef enum run_result_box_size_t {
	RUN_RESULT_BOX_STANDARD = 0,
	RUN_RESULT_BOX_LARGE,
	RUN_RESULT_BOX_CUSTOM
} run_result_box_size_t;

typedef struct run_result_layout_config_t {
	run_result_box_size_t size;
	enum text_box_positions position;
	int custom_width;
	int custom_height;
	int vertical_offset;
	int button_gap;
	int message_button_gap;
	int bottom_padding;
} run_result_layout_config_t;

typedef struct run_result_layout_t {
	SDL_Rect box;
	SDL_Rect content;
	SDL_Rect message;
	SDL_Rect back_button;
	SDL_Rect continue_button;
	int text_height;
} run_result_layout_t;

/*
 * Resolve this configuration once during application initialization. Changing
 * modes requires restart; runtime changes would rebuild wrapped text textures
 * and reinitialize dependent arrows.
 */
static const run_result_layout_config_t g_run_result_layout_config = {
	.size = RUN_RESULT_BOX_LARGE,
	.position = TX_CENTER_BOX,
	.custom_width = 900,
	.custom_height = 520,
	.vertical_offset = -30,
	.button_gap = 30,
	.message_button_gap = 15,
	.bottom_padding = 24
};

static run_result_layout_t g_failure_layout;
static run_result_layout_t g_success_layout;
static bool g_layout_resolved;

static bool rr_rect_contains(SDL_Rect outer, SDL_Rect inner)
{
	if (outer.w <= 0 || outer.h <= 0 || inner.w <= 0 || inner.h <= 0) {
		return false;
	}
	int64_t outer_right = (int64_t)outer.x + outer.w;
	int64_t outer_bottom = (int64_t)outer.y + outer.h;
	int64_t inner_right = (int64_t)inner.x + inner.w;
	int64_t inner_bottom = (int64_t)inner.y + inner.h;
	return inner.x >= outer.x && inner.y >= outer.y &&
	       inner_right <= outer_right && inner_bottom <= outer_bottom;
}

static bool rr_rects_overlap(SDL_Rect first, SDL_Rect second)
{
	if (first.w <= 0 || first.h <= 0 || second.w <= 0 || second.h <= 0) {
		return false;
	}
	return (int64_t)first.x < (int64_t)second.x + second.w &&
	       (int64_t)second.x < (int64_t)first.x + first.w &&
	       (int64_t)first.y < (int64_t)second.y + second.h &&
	       (int64_t)second.y < (int64_t)first.y + first.h;
}

static bool rr_get_layout_for_size(run_result_box_size_t size,
								   bool success,
								   run_result_layout_t *layout,
								   const char **failure_reason)
{
	if (failure_reason != NULL) {
		*failure_reason = "unable to resolve the configured text-box position";
	}
	if (layout == NULL) {
		return false;
	}

	tx_text_box_options_t options = {
		.position = g_run_result_layout_config.position,
		.large_box = size == RUN_RESULT_BOX_LARGE,
		.large_text = false
	};
	SDL_Rect base_box;
	int text_height = 0;
	if (!tx_get_text_box_rects(&options, &base_box, NULL,
	                           &text_height)) {
		return false;
	}

	SDL_Rect box = base_box;
	if (size == RUN_RESULT_BOX_CUSTOM) {
		if (g_run_result_layout_config.custom_width <= 0) {
			if (failure_reason != NULL) *failure_reason = "width must be positive";
			return false;
		}
		if (g_run_result_layout_config.custom_height <= 0) {
			if (failure_reason != NULL) *failure_reason = "height must be positive";
			return false;
		}
		int width = dm_scale_to_res(g_run_result_layout_config.custom_width);
		int height = dm_scale_to_res(g_run_result_layout_config.custom_height);
		int screen_width = dm_get_screen_width();
		int screen_height = dm_get_screen_height();
		if (width <= 0) {
			if (failure_reason != NULL) *failure_reason = "scaled width must be positive";
			return false;
		}
		if (height <= 0) {
			if (failure_reason != NULL) *failure_reason = "scaled height must be positive";
			return false;
		}
		int margin = dm_scale_to_res(16);
		if (screen_width <= 2 * margin || width > screen_width - 2 * margin) {
			if (failure_reason != NULL) *failure_reason = "width exceeds the usable screen width";
			return false;
		}
		if (screen_height <= 2 * margin || height > screen_height - 2 * margin) {
			if (failure_reason != NULL) *failure_reason = "height exceeds the usable screen height";
			return false;
		}
		int center_x = base_box.x + base_box.w / 2;
		int center_y = base_box.y + base_box.h / 2;
		box.w = width;
		box.h = height;
		box.x = center_x - width / 2;
		box.y = center_y - height / 2;
	}
	int64_t offset_y = (int64_t)box.y +
	                   dm_scale_to_res(g_run_result_layout_config.vertical_offset);
	if (offset_y < INT_MIN || offset_y > INT_MAX) {
		if (failure_reason != NULL) *failure_reason =
			"vertical offset exceeds supported coordinates";
		return false;
	}
	box.y = (int)offset_y;
	int safe_margin = dm_scale_to_res(16);
	int64_t box_right = (int64_t)box.x + box.w;
	int64_t box_bottom = (int64_t)box.y + box.h;
	if (box.x < safe_margin ||
	    box_right > (int64_t)dm_get_screen_width() - safe_margin) {
		if (failure_reason != NULL) *failure_reason =
			"final box exceeds the screen safe width";
		return false;
	}
	if (box.y < safe_margin ||
	    box_bottom > (int64_t)dm_get_screen_height() - safe_margin) {
		if (failure_reason != NULL) *failure_reason =
			"vertical offset moves the final box outside the screen safe area";
		return false;
	}

	SDL_Rect content = dw_get_iface_content_box(box);
	if (content.w <= 0 || content.h <= 0 ||
	    !rr_rect_contains(box, content)) {
		if (failure_reason != NULL) *failure_reason =
			"content rectangle is outside the box or has no usable area";
		return false;
	}
	SDL_Rect button_size = dm_get_modal_button_wh();
	int gap = dm_scale_to_res(g_run_result_layout_config.button_gap);
	int message_gap = dm_scale_to_res(
		g_run_result_layout_config.message_button_gap);
	int bottom_padding = dm_scale_to_res(
		g_run_result_layout_config.bottom_padding);
	if (button_size.w <= 0 || button_size.h <= 0) {
		if (failure_reason != NULL) *failure_reason =
			"modal button dimensions must be positive";
		return false;
	}
	if (gap < 0) {
		if (failure_reason != NULL) *failure_reason =
			"success button gap must not be negative";
		return false;
	}
	if (bottom_padding < 0) {
		if (failure_reason != NULL) *failure_reason =
			"bottom padding must not be negative";
		return false;
	}
	int button_width = button_size.w;
	if (success) {
		int max_button_width = (content.w - gap) / 2;
		if (size == RUN_RESULT_BOX_CUSTOM && max_button_width < button_width) {
			if (failure_reason != NULL) *failure_reason =
				"success button group exceeds content width";
			return false;
		}
		if (max_button_width < button_width) {
			button_width = max_button_width;
		}
	} else if (content.w < button_width) {
		button_width = content.w;
	}

	int64_t reserved_height = (int64_t)button_size.h + message_gap +
	                          bottom_padding;
	int64_t message_height = (int64_t)content.h - reserved_height;
	if (button_width <= 0) {
		if (failure_reason != NULL) *failure_reason =
			"button row has no usable width";
		return false;
	}
	if (message_gap < 0 || message_height <= 0 ||
	    message_height < text_height) {
		if (failure_reason != NULL) *failure_reason =
			"message area cannot fit one line above the button row";
		return false;
	}

	if (message_height > INT_MAX) {
		if (failure_reason != NULL) *failure_reason =
			"message rectangle height exceeds supported dimensions";
		return false;
	}
	SDL_Rect back_button = {
		.x = content.x + (content.w - button_width) / 2,
		.y = content.y + content.h - button_size.h - bottom_padding,
		.w = button_width,
		.h = button_size.h
	};
	SDL_Rect continue_button = back_button;
	if (success) {
		int group_width = 2 * button_width + gap;
		int group_x = content.x + (content.w - group_width) / 2;
		back_button.x = group_x;
		continue_button.x = group_x + button_width + gap;
	}

	run_result_layout_t computed_layout = {
		.box = box,
		.content = content,
		.message = {
			.x = content.x,
			.y = content.y,
			.w = content.w,
			.h = (int)message_height
		},
		.back_button = back_button,
		.continue_button = continue_button,
		.text_height = text_height
	};
	if (!rr_rect_contains(computed_layout.content, computed_layout.message)) {
		if (failure_reason != NULL) *failure_reason =
			"message rectangle is outside the content area";
		return false;
	}
	if (!rr_rect_contains(computed_layout.content,
	                      computed_layout.back_button)) {
		if (failure_reason != NULL) *failure_reason = success ?
			"success Back button is outside the content area" :
			"failure Back button is outside the content area";
		return false;
	}
	int expected_button_y = content.y + content.h - button_size.h - bottom_padding;
	if (computed_layout.back_button.y != expected_button_y) {
		if (failure_reason != NULL) *failure_reason =
			"Back button does not respect bottom padding";
		return false;
	}
	if (rr_rects_overlap(computed_layout.message, computed_layout.back_button)) {
		if (failure_reason != NULL) *failure_reason =
			success ? "success Back button overlaps message area" :
			"failure Back button overlaps message area";
		return false;
	}
	if (!success) {
		int centered_x = content.x + (content.w - computed_layout.back_button.w) / 2;
		if (computed_layout.back_button.x != centered_x) {
			if (failure_reason != NULL) *failure_reason =
				"failure Back button is not horizontally centered";
			return false;
		}
	} else {
		if (!rr_rect_contains(computed_layout.content,
		                      computed_layout.continue_button)) {
			if (failure_reason != NULL) *failure_reason =
				"success Continue button is outside the content area";
			return false;
		}
		int group_width = computed_layout.back_button.w + gap +
		                  computed_layout.continue_button.w;
		int group_x = content.x + (content.w - group_width) / 2;
		if (group_width > content.w || computed_layout.back_button.x != group_x ||
		    computed_layout.continue_button.x !=
		        computed_layout.back_button.x + computed_layout.back_button.w + gap) {
			if (failure_reason != NULL) *failure_reason =
				"success button group exceeds content width or is not centered";
			return false;
		}
		if (computed_layout.back_button.w != computed_layout.continue_button.w ||
		    computed_layout.back_button.h != computed_layout.continue_button.h ||
		    computed_layout.back_button.y != computed_layout.continue_button.y ||
		    computed_layout.back_button.x >= computed_layout.continue_button.x) {
			if (failure_reason != NULL) *failure_reason =
				"success buttons are not aligned as a two-button group";
			return false;
		}
		if (rr_rects_overlap(computed_layout.back_button,
		                     computed_layout.continue_button)) {
			if (failure_reason != NULL) *failure_reason =
				"success Back and Continue buttons overlap";
			return false;
		}
		if (computed_layout.continue_button.y != expected_button_y) {
			if (failure_reason != NULL) *failure_reason =
				"Continue button does not respect bottom padding";
			return false;
		}
		if (rr_rects_overlap(computed_layout.message,
		                     computed_layout.continue_button)) {
			if (failure_reason != NULL) *failure_reason =
				"success Continue button overlaps message area";
			return false;
		}
	}
	*layout = computed_layout;
	return true;
}

static bool rr_validate_layout_mode(run_result_box_size_t size,
									run_result_layout_t *failure_layout,
									run_result_layout_t *success_layout,
									const char **failure_reason)
{
	if (!rr_get_layout_for_size(size, false, failure_layout, failure_reason) ||
	    !rr_get_layout_for_size(size, true, success_layout, failure_reason)) {
		return false;
	}
	if (failure_layout->message.w != success_layout->message.w ||
	    failure_layout->text_height != success_layout->text_height) {
		if (failure_reason != NULL) *failure_reason =
			"failure and success message widths or text heights do not match";
		return false;
	}
	return true;
}

static bool rr_resolve_layout(void)
{
	const char *failure_reason = NULL;
	run_result_box_size_t requested_size = g_run_result_layout_config.size;
	g_layout_resolved = false;
	if (rr_validate_layout_mode(requested_size, &g_failure_layout,
	                            &g_success_layout, &failure_reason)) {
		g_layout_resolved = true;
		return true;
	}
	if (requested_size == RUN_RESULT_BOX_LARGE) {
		fprintf(stderr, "Large Run Result layout rejected: %s\n",
		        failure_reason != NULL ? failure_reason : "unknown geometry error");
		return false;
	}

	fprintf(stderr, "Run Result %s layout rejected: %s\n",
	        requested_size == RUN_RESULT_BOX_CUSTOM ? "custom" : "standard",
	        failure_reason != NULL ? failure_reason : "unknown geometry error");
	fputs("Falling back to the large Run Result layout\n", stderr);
	if (!rr_validate_layout_mode(RUN_RESULT_BOX_LARGE, &g_failure_layout,
	                             &g_success_layout, &failure_reason)) {
		fprintf(stderr, "Large Run Result layout rejected: %s\n",
		        failure_reason != NULL ? failure_reason : "unknown geometry error");
		return false;
	}
	g_layout_resolved = true;
	return true;
}

static bool rr_get_layout(bool success, run_result_layout_t *layout)
{
	if (!g_layout_resolved || layout == NULL) {
		return false;
	}
	*layout = success ? g_success_layout : g_failure_layout;
	return true;
}

static bool rr_is_success(int operation_id)
{
	return operation_id == MC_WIN;
}

static bool rr_is_failure(int operation_id)
{
	return operation_id > NO_OPERATION && operation_id < MC_WIN;
}

static bool rr_is_valid_operation(int operation_id)
{
	return rr_is_success(operation_id) || rr_is_failure(operation_id);
}

static texture_array_t *rr_get_message(int operation_id)
{
	switch (operation_id) {
	case INPUT_BUFFER_EMPTY: return ib_empty;
	case REG_VALUE_INVALID: return reg_val_bad;
	case FLAG_VALUE_INVALID: return flag_val_bad;
	case INVALID_OUTPUT_VALUE: return ob_val_bad;
	case UNPROCESSED_IB_VALUES: return ib_unproc_vals;
	case EXCEEDS_CODE_LIMIT: return exc_code_size;
	case OUTPUT_BUFFER_INCOMPLETE: return ob_incomplete;
	case MC_WIN: return g_win_text;
	default: return NULL;
	}
}

static bool rr_assign_button_rectangles(const run_result_layout_t *layout)
{
	if (layout == NULL || g_result_back_button == NULL ||
	    g_result_continue_button == NULL) {
		return false;
	}
	g_result_back_button->r = layout->back_button;
	g_result_continue_button->r = layout->continue_button;
	return true;
}

bool rr_get_failure_back_button_rect(SDL_Rect *button_rect)
{
	if (button_rect == NULL) {
		return false;
	}
	run_result_layout_t layout;
	if (!rr_get_layout(false, &layout)) {
		return false;
	}
	*button_rect = layout.back_button;
	return true;
}

void rr_reset_state(void)
{
	g_presented_operation_id = NO_OPERATION;
	g_result_sound_played = false;
}

bool rr_initialize(void)
{
	if (g_initialized) {
		return true;
	}

	if (!rr_resolve_layout()) {
		return false;
	}
	int message_width = g_failure_layout.message.w;
	int text_h = g_failure_layout.text_height;

#define RR_CHECK_RESOURCE(resource, expression) \
	do { \
		resource = expression; \
		if (resource == NULL) { \
			fprintf(stderr, "Run Result initialization failed: %s\n", #resource); \
			goto error; \
		} \
	} while (0)

	RR_CHECK_RESOURCE(ib_empty, dw_create_text_tex_array_by_h(
		message_width, text_h, C_WHITE, INPUT_BUFFER_EMPTY_TEXT));
	RR_CHECK_RESOURCE(reg_val_bad, dw_create_text_tex_array_by_h(
		message_width, text_h, C_WHITE, REG_VALUE_INVALID_TEXT));
	RR_CHECK_RESOURCE(flag_val_bad, dw_create_text_tex_array_by_h(
		message_width, text_h, C_WHITE, FLAG_VALUE_INVALID_TEXT));
	RR_CHECK_RESOURCE(ob_val_bad, dw_create_text_tex_array_by_h(
		message_width, text_h, C_WHITE, INVALID_OUTPUT_VALUE_TEXT));
	RR_CHECK_RESOURCE(ib_unproc_vals, dw_create_text_tex_array_by_h(
		message_width, text_h, C_WHITE, UNPROCESSED_IB_VALUES_TEXT));
	RR_CHECK_RESOURCE(exc_code_size, dw_create_text_tex_array_by_h(
		message_width, text_h, C_WHITE, EXCEEDS_CODE_LIMIT_TEXT));
	RR_CHECK_RESOURCE(ob_incomplete, dw_create_text_tex_array_by_h(
		message_width, text_h, C_WHITE, OUTPUT_BUFFER_INCOMPLETE_TEXT));
	RR_CHECK_RESOURCE(g_win_text, dw_create_text_tex_array_by_h(
		message_width, text_h, C_WHITE, WIN_TEXT));
	RR_CHECK_RESOURCE(g_run_completed, dw_create_text_tex(
		RUN_COMPLETED_TEXT, C_WHITE));
	RR_CHECK_RESOURCE(g_run_failed, dw_create_text_tex(RUN_FAILED_TEXT, C_WHITE));

	texture_t *back_label = dw_create_text_tex(AX_STR_BACK, C_WHITE);
	if (back_label == NULL) {
		fprintf(stderr, "Run Result initialization failed: Back button label\n");
		goto error;
	}
	g_result_back_button = bt_create_iface_btn(g_failure_layout.back_button,
	                                          back_label, true);
	if (g_result_back_button == NULL) {
		dw_free_texture(back_label);
		fprintf(stderr, "Run Result initialization failed: Back button\n");
		goto error;
	}

	texture_t *continue_label = dw_create_text_tex(AX_STR_CONT, C_WHITE);
	if (continue_label == NULL) {
		fprintf(stderr, "Run Result initialization failed: Continue button label\n");
		goto error;
	}
	g_result_continue_button = bt_create_iface_btn(
	                                              g_success_layout.continue_button,
	                                              continue_label, true);
	if (g_result_continue_button == NULL) {
		dw_free_texture(continue_label);
		fprintf(stderr, "Run Result initialization failed: Continue button\n");
		goto error;
	}

	g_initialized = true;
	rr_reset_state();
#undef RR_CHECK_RESOURCE
	return true;

error:
#undef RR_CHECK_RESOURCE
	rr_destroy();
	return false;
}

void rr_destroy(void)
{
	if (g_result_back_button != NULL) {
		bt_destroy_iface_btn(g_result_back_button);
		g_result_back_button = NULL;
	}
	if (g_result_continue_button != NULL) {
		bt_destroy_iface_btn(g_result_continue_button);
		g_result_continue_button = NULL;
	}

	dw_free_texture(g_run_completed);
	g_run_completed = NULL;
	dw_free_texture(g_run_failed);
	g_run_failed = NULL;
	dw_free_texture_array(ib_empty);
	ib_empty = NULL;
	dw_free_texture_array(reg_val_bad);
	reg_val_bad = NULL;
	dw_free_texture_array(flag_val_bad);
	flag_val_bad = NULL;
	dw_free_texture_array(ob_val_bad);
	ob_val_bad = NULL;
	dw_free_texture_array(ib_unproc_vals);
	ib_unproc_vals = NULL;
	dw_free_texture_array(exc_code_size);
	exc_code_size = NULL;
	dw_free_texture_array(ob_incomplete);
	ob_incomplete = NULL;
	dw_free_texture_array(g_win_text);
	g_win_text = NULL;
	g_initialized = false;
	g_layout_resolved = false;
	g_failure_layout = (run_result_layout_t){0};
	g_success_layout = (run_result_layout_t){0};
	rr_reset_state();
}

run_result_action_t rr_update(int operation_id)
{
	if (operation_id == NO_OPERATION) {
		rr_reset_state();
		return RUN_RESULT_ACTION_NONE;
	}
	if (!rr_is_valid_operation(operation_id)) {
		fprintf(stderr, "Run Result received invalid operation ID: %d\n",
		        operation_id);
		return RUN_RESULT_ACTION_NONE;
	}
	if (!g_initialized || em_get_escape_state()) {
		return RUN_RESULT_ACTION_NONE;
	}

	run_result_layout_t layout;
	if (!rr_get_layout(rr_is_success(operation_id), &layout) ||
	    !rr_assign_button_rectangles(&layout)) {
		return RUN_RESULT_ACTION_NONE;
	}
	if (operation_id != g_presented_operation_id) {
		g_presented_operation_id = operation_id;
		g_result_sound_played = false;
	}
	if (!g_result_sound_played) {
		Mix_Chunk *sound = rr_is_success(operation_id) ?
			g_sfx_run_win : g_sfx_run_error;
		if (sound != NULL) {
			Mix_PlayChannel(-1, sound, 0);
		}
		g_result_sound_played = true;
	}

	if (bt_chk_rel_iface_btn(g_result_back_button,
	                         g_sfx_iface_back_cancel)) {
		ms_consume_left_release();
		return RUN_RESULT_ACTION_BACK;
	}
	if (rr_is_success(operation_id) &&
	    bt_chk_rel_iface_btn(g_result_continue_button, g_sfx_select)) {
		ms_consume_left_release();
		return RUN_RESULT_ACTION_CONTINUE;
	}
	return RUN_RESULT_ACTION_NONE;
}

void rr_render(int operation_id)
{
	if (operation_id == NO_OPERATION || !g_initialized) {
		return;
	}
	if (!rr_is_valid_operation(operation_id)) {
		fprintf(stderr, "Run Result cannot render invalid operation ID: %d\n",
		        operation_id);
		return;
	}
	if (em_get_escape_state()) {
		return;
	}

	run_result_layout_t layout;
	if (!rr_get_layout(rr_is_success(operation_id), &layout) ||
	    !rr_assign_button_rectangles(&layout)) {
		return;
	}
	texture_array_t *message = rr_get_message(operation_id);
	texture_t *header = rr_is_success(operation_id) ?
		g_run_completed : g_run_failed;
	if (message == NULL || header == NULL) {
		fprintf(stderr, "Run Result resources unavailable for operation ID: %d\n",
		        operation_id);
		return;
	}

	dw_draw_iface_box_with_status(layout.box, header,
	                              rr_is_success(operation_id));
	dw_draw_wrapped_texture_by_h(layout.message, layout.text_height, message);
	bt_draw_iface_btn(g_result_back_button, false, g_sfx_iface_hover);
	if (rr_is_success(operation_id)) {
		bt_draw_iface_btn(g_result_continue_button, false, g_sfx_iface_hover);
	}
}