#include <stdbool.h>
#include <stdio.h>
#include "ui/run_result_rr.h"
#include "game_mechanics_mc.h"
#include "dimensions_dm.h"
#include "draw_dw.h"
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

static void rr_assign_button_rectangles(int operation_id)
{
	if (g_result_back_button == NULL || g_result_continue_button == NULL) {
		return;
	}
	g_result_back_button->r = rr_is_success(operation_id) ?
		dm_get_text_box_result_but1() : dm_get_text_box_result_but3();
	g_result_continue_button->r = dm_get_text_box_result_but2();
}

void rr_reset_state(void)
{
	g_presented_operation_id = NO_OPERATION;
	g_result_sound_played = false;
}

bool rr_is_visible(int operation_id)
{
	return g_initialized && rr_is_valid_operation(operation_id) &&
	       !em_get_escape_state();
}

bool rr_initialize(void)
{
	if (g_initialized) {
		return true;
	}

	int text_h = dm_get_h_msg();
	SDL_Rect result_box = dm_get_run_result_box();
	SDL_Rect message_box = dm_get_run_result_message_box();
	SDL_Rect content_box = dw_get_iface_content_box(result_box);
	int message_width = message_box.w > 0 ? message_box.w : content_box.w;

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
	g_result_back_button = bt_create_iface_btn(
		dm_get_text_box_result_but3(), back_label, true);
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
		dm_get_text_box_result_but2(), continue_label, true);
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

	rr_assign_button_rectangles(operation_id);
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

	texture_array_t *message = rr_get_message(operation_id);
	texture_t *header = rr_is_success(operation_id) ?
		g_run_completed : g_run_failed;
	if (message == NULL || header == NULL) {
		fprintf(stderr, "Run Result resources unavailable for operation ID: %d\n",
		        operation_id);
		return;
	}

	dw_draw_iface_box_with_status(dm_get_run_result_box(), header,
	                              rr_is_success(operation_id));
	dw_draw_wrapped_texture_by_h(dm_get_run_result_message_box(),
	                             dm_get_h_msg(), message);
	rr_assign_button_rectangles(operation_id);
	bt_draw_iface_btn(g_result_back_button, false, g_sfx_iface_hover);
	if (rr_is_success(operation_id)) {
		bt_draw_iface_btn(g_result_continue_button, false, g_sfx_iface_hover);
	}
}