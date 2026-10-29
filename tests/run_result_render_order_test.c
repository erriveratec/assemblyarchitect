#include <assert.h>
#include <stdio.h>

#include "game_mechanics_mc.h"
#include "ui/run_result_rr.h"
#include "tutorial_tr.h"
#include "aux.h"
#include "ui/button_bt.h"
#include "ui/reset_menu_rm.h"
#include "ui/escape_menu_em.h"
#include "stages.h"
#include "code_window_cw.h"
#include "instruction_window_iw.h"
#include "instruction_layout_il.h"
#include "buffers_bf.h"
#include "registers_rg.h"
#include "file_fl.h"
#include "dimensions_dm.h"
#include "sdl_config.h"
#include "stage_buttons_sb.h"
#include "levels_lv.h"
#include "mouse_ms.h"
#include "arrow_ar.h"
#include "immediates_im.h"
#include "electron_fx.h"
#include "text_tx.h"
#include "gameplay/win_condition_wc.h"
#include "gameplay/operand_highlights_oh.h"
#include "gameplay/highlight_animation_ha.h"
#include "gameplay/level_presentation_lp.h"

enum draw_event {
	DRAW_FRAME,
	DRAW_INSTRUCTIONS,
	DRAW_CODE,
	DRAW_CONTROLS,
	DRAW_IMMEDIATES,
	DRAW_EXECUTION_ARROW,
	DRAW_BUFFERS,
	DRAW_REGISTERS,
	DRAW_AVATAR,
	DRAW_RETURN,
	DRAW_RESET,
	DRAW_LEVEL_UI,
	DRAW_FLAG_BOXES,
	DRAW_TUTORIAL,
	DRAW_RESULT_BOX,
	DRAW_RESULT_MESSAGE,
	DRAW_RESULT_BACK,
	DRAW_RESULT_CONTINUE
};

static enum draw_event events[32];
static size_t event_count;
static bool escape_active;
static void draw_level(int level);
static int draw_result_button(iface_btn_t *button);
static tx_styled_text_t *test_create_styled_text(
	SDL_Rect content, int text_height, const char *message,
	const tx_styled_text_options_t *options,
	tx_text_layout_info_t *layout_info);
static void test_free_styled_text(tx_styled_text_t *message);
static int styled_text_allocation_attempts;
static int styled_text_free_count;
static int fail_styled_text_after = 3;
static bool styled_options_valid = true;
static bool saw_lowercase_input_message;
static tx_styled_text_t fake_styled_text[8];
static texture_t fake_textures[4];
static iface_btn_t fake_buttons[2];
static int fake_texture_count;
static int fake_button_count;

static texture_t *test_create_text(char *text, SDL_Color color);
static iface_btn_t *test_create_button(SDL_Rect rect, texture_t *label,
										  bool enabled);

static void record_draw(enum draw_event event)
{
	assert(event_count < sizeof(events) / sizeof(events[0]));
	events[event_count++] = event;
}

static void record_animated_draw(enum draw_event event, float limit)
{
	assert(limit == dw_get_animation_limit());
	record_draw(event);
}

#define ha_begin_frame() record_draw(DRAW_FRAME)
#define iw_draw_ins_box() record_draw(DRAW_INSTRUCTIONS)
#define cw_draw_code_window() record_draw(DRAW_CODE)
#define cw_get_challenge_highlight_limit() ((float)dw_get_animation_limit())
#define cw_get_code_list_size() 0
#define sb_draw_stage_btns(size, limit) record_draw(DRAW_CONTROLS)
#define im_draw_imm(limit) record_draw(DRAW_IMMEDIATES)
#define ar_display_arrow(arrow, limit) record_draw(DRAW_EXECUTION_ARROW)
#define bf_draw_buffers(limit) record_draw(DRAW_BUFFERS)
#define rg_draw_registers(limit) record_animated_draw(DRAW_REGISTERS, limit)
#define mc_draw_avatar() record_draw(DRAW_AVATAR)
#define sb_draw_ret_btn() record_draw(DRAW_RETURN)
#define sb_draw_rst_btn() record_draw(DRAW_RESET)
#define lv_level_drawings(level) draw_level(level)
#define em_get_escape_state() escape_active
#include "../source/stages.c"
#undef lv_level_drawings

#define set_code_editable level_test_set_code_editable
#define sb_chk_rst_esc_menu_active() escape_active
#define rg_draw_flag_boxes(limit) record_animated_draw(DRAW_FLAG_BOXES, limit)
#define tr_render_current() record_draw(DRAW_TUTORIAL)
#include "../source/levels_lv.c"
#undef set_code_editable

#define dw_draw_iface_box_with_status(box, header, success) record_draw(DRAW_RESULT_BOX)
#define tx_draw_styled_text(rect, height, message, offset) record_draw(DRAW_RESULT_MESSAGE)
#define tx_create_styled_text test_create_styled_text
#define tx_free_styled_text test_free_styled_text
#define dw_create_text_tex test_create_text
#define dw_free_texture(texture) ((void)(texture))
#define bt_create_iface_btn test_create_button
#define bt_destroy_iface_btn(button) ((void)(button))
#define bt_draw_iface_btn(button, blocked, sound) draw_result_button(button)
#include "../source/ui/run_result_rr.c"
#undef tx_create_styled_text
#undef tx_free_styled_text
#undef dw_create_text_tex
#undef dw_free_texture
#undef bt_create_iface_btn
#undef bt_destroy_iface_btn

static void draw_level(int level)
{
	record_draw(DRAW_LEVEL_UI);
	lv_level_drawings(level);
}

static int draw_result_button(iface_btn_t *button)
{
	assert(button == g_result_back_button || button == g_result_continue_button);
	record_draw(button == g_result_back_button ? DRAW_RESULT_BACK : DRAW_RESULT_CONTINUE);
	return BTN_NONE;
}

static tx_styled_text_t *test_create_styled_text(
	SDL_Rect content, int text_height, const char *message,
	const tx_styled_text_options_t *options,
	tx_text_layout_info_t *layout_info)
{
	(void)content;
	(void)text_height;
	(void)message;
	(void)layout_info;
	styled_text_allocation_attempts++;
	if (options == NULL || !options->highlight_arch_tokens ||
	    options->parse_authoring_prefixes ||
	    options->body_color.r != C_WHITE.r ||
	    options->body_color.g != C_WHITE.g ||
	    options->body_color.b != C_WHITE.b) {
		styled_options_valid = false;
	}
	if (strcmp(message, "A value cannot be recovered if the Input Buffer [ib] is empty") == 0) {
		saw_lowercase_input_message = true;
	}
	if ((fail_styled_text_after > 0 &&
	     styled_text_allocation_attempts > fail_styled_text_after) ||
	    styled_text_allocation_attempts >
	        (int)(sizeof(fake_styled_text) / sizeof(fake_styled_text[0]))) {
		return NULL;
	}
	return &fake_styled_text[styled_text_allocation_attempts - 1];
}

static texture_t *test_create_text(char *text, SDL_Color color)
{
	(void)text;
	(void)color;
	assert(fake_texture_count < (int)(sizeof(fake_textures) / sizeof(fake_textures[0])));
	return &fake_textures[fake_texture_count++];
}

static iface_btn_t *test_create_button(SDL_Rect rect, texture_t *label,
										 bool enabled)
{
	assert(fake_button_count < (int)(sizeof(fake_buttons) / sizeof(fake_buttons[0])));
	iface_btn_t *button = &fake_buttons[fake_button_count++];
	button->r = rect;
	button->t = label;
	button->enabled = enabled;
	return button;
}

static void test_free_styled_text(tx_styled_text_t *message)
{
	if (message != NULL) {
		styled_text_free_count++;
	}
}

static void initialize_result_fixture(void)
{
	static texture_t header;
	static tx_styled_text_t message;
	static iface_btn_t back = {.enabled = true};
	static iface_btn_t next = {.enabled = true};
	dm_set_screen_resolution(DM_RESOLUTION_1600X900);
	assert(rr_resolve_layout());
	g_result_back_button = &back;
	g_result_continue_button = &next;
	g_run_failed = g_run_completed = &header;
	ib_empty = reg_val_bad = flag_val_bad = ob_val_bad = &message;
	ib_unproc_vals = exc_code_size = ob_incomplete = g_win_text = &message;
	g_initialized = true;
}

static void test_render_order(int level, int operation, bool flags_visible)
{
	lp_configure(false, false, false, flags_visible, true);
	event_count = 0;
	stage_drawings(level, operation);
	size_t cursor = 0;
	for (enum draw_event event = DRAW_FRAME; event <= DRAW_LEVEL_UI; event++) {
		assert(events[cursor++] == event);
	}
	if (flags_visible) {
		assert(events[cursor++] == DRAW_FLAG_BOXES);
	}
	assert(events[cursor++] == DRAW_TUTORIAL);
	if (operation != NO_OPERATION) {
		assert(rr_is_valid_operation(operation));
		assert(events[cursor++] == DRAW_RESULT_BOX);
		assert(events[cursor++] == DRAW_RESULT_MESSAGE);
		assert(events[cursor++] == DRAW_RESULT_BACK);
		if (operation == MC_WIN) {
			assert(events[cursor++] == DRAW_RESULT_CONTINUE);
		}
	}
	assert(cursor == event_count);
}

static void release_at(SDL_Rect rect)
{
	ms_reset_mouse_values();
	SDL_Event event = {0};
	event.type = SDL_MOUSEBUTTONUP;
	event.button.button = SDL_BUTTON_LEFT;
	event.button.x = rect.x + rect.w / 2;
	event.button.y = rect.y + rect.h / 2;
	ms_mouse_button_handler(event);
}

static void test_result_buttons(void)
{
	const int operations[] = {OUTPUT_BUFFER_INCOMPLETE, MC_WIN};
	for (size_t index = 0; index < sizeof(operations) / sizeof(operations[0]); index++) {
		int operation = operations[index];
		run_result_layout_t layout;
		assert(rr_get_layout(operation == MC_WIN, &layout));
		release_at(layout.back_button);
		assert(rr_update(operation) == RUN_RESULT_ACTION_BACK);
		assert(!ms_left_released());
		if (operation == MC_WIN) {
			release_at(layout.continue_button);
			assert(rr_update(operation) == RUN_RESULT_ACTION_CONTINUE);
			assert(!ms_left_released());
		}
		escape_active = true;
		release_at(layout.back_button);
		assert(rr_update(operation) == RUN_RESULT_ACTION_NONE);
		event_count = 0;
		rr_render(operation);
		assert(event_count == 0);
		escape_active = false;
	}
	assert(!rr_is_valid_operation(NO_OPERATION));
	assert(!rr_is_valid_operation(-1));
	assert(!rr_is_valid_operation(MC_WIN + 1));
}

static void test_partial_initialization_cleanup(void)
{
	ib_empty = reg_val_bad = flag_val_bad = ob_val_bad = NULL;
	ib_unproc_vals = exc_code_size = ob_incomplete = g_win_text = NULL;
	g_run_completed = NULL;
	g_run_failed = NULL;
	g_result_back_button = NULL;
	g_result_continue_button = NULL;
	g_initialized = false;
	styled_text_allocation_attempts = 0;
	styled_text_free_count = 0;
	fail_styled_text_after = 3;
	styled_options_valid = true;
	saw_lowercase_input_message = false;

	assert(!rr_initialize());
	assert(styled_text_allocation_attempts == 4);
	assert(styled_text_free_count == 3);
	assert(styled_options_valid);
	assert(saw_lowercase_input_message);
	assert(ib_empty == NULL && reg_val_bad == NULL && flag_val_bad == NULL);
	assert(ob_val_bad == NULL && ib_unproc_vals == NULL && exc_code_size == NULL);
	assert(ob_incomplete == NULL && g_win_text == NULL);
	assert(g_run_completed == NULL && g_run_failed == NULL);
	assert(!g_initialized && !g_layout_resolved);
	run_result_layout_t layout;
	assert(!rr_get_layout(false, &layout));

	rr_destroy();
	assert(styled_text_free_count == 3);

	styled_text_allocation_attempts = 0;
	fail_styled_text_after = 0;
	fake_texture_count = 0;
	fake_button_count = 0;
	assert(rr_initialize());
	assert(g_initialized);
	assert(styled_text_allocation_attempts == 8);
	assert(g_win_text != NULL && ob_incomplete != NULL && ib_empty != NULL);
	rr_destroy();
	assert(!g_initialized);
	assert(styled_text_free_count == 11);
	rr_destroy();
	assert(styled_text_free_count == 11);
}

int main(void)
{
	initialize_result_fixture();
	const int levels[] = {LV_LEVEL_13, LV_LEVEL_0};
	const int operations[] = {
		NO_OPERATION, INPUT_BUFFER_EMPTY, REG_VALUE_INVALID, FLAG_VALUE_INVALID,
		INVALID_OUTPUT_VALUE, UNPROCESSED_IB_VALUES, EXCEEDS_CODE_LIMIT,
		OUTPUT_BUFFER_INCOMPLETE, MC_WIN
	};
	for (size_t level_index = 0; level_index < sizeof(levels) / sizeof(levels[0]); level_index++) {
		for (size_t operation_index = 0; operation_index < sizeof(operations) / sizeof(operations[0]); operation_index++) {
			test_render_order(levels[level_index], operations[operation_index], true);
			test_render_order(levels[level_index], operations[operation_index], false);
		}
	}
	test_result_buttons();
	test_partial_initialization_cleanup();
	puts("Run-result render order and button tests passed");
	return 0;
}