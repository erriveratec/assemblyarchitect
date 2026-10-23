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

static void record_draw(enum draw_event event)
{
	assert(event_count < sizeof(events) / sizeof(events[0]));
	events[event_count++] = event;
}

#define ha_begin_frame() record_draw(DRAW_FRAME)
#define iw_draw_ins_box() record_draw(DRAW_INSTRUCTIONS)
#define cw_draw_code_window() record_draw(DRAW_CODE)
#define cw_get_challenge_highlight_limit() 0.0f
#define cw_get_code_list_size() 0
#define sb_draw_stage_btns(size, limit) record_draw(DRAW_CONTROLS)
#define im_draw_imm(limit) record_draw(DRAW_IMMEDIATES)
#define ar_display_arrow(arrow, limit) record_draw(DRAW_EXECUTION_ARROW)
#define bf_draw_buffers(limit) record_draw(DRAW_BUFFERS)
#define rg_draw_registers(limit) record_draw(DRAW_REGISTERS)
#define mc_draw_avatar() record_draw(DRAW_AVATAR)
#define sb_draw_ret_btn() record_draw(DRAW_RETURN)
#define sb_draw_rst_btn() record_draw(DRAW_RESET)
#define lv_level_drawings(level) draw_level(level)
#define em_get_escape_state() escape_active
#include "../source/stages.c"
#undef lv_level_drawings

#define set_code_editable level_test_set_code_editable
#define sb_chk_rst_esc_menu_active() escape_active
#define rg_draw_flag_boxes(limit) record_draw(DRAW_FLAG_BOXES)
#define tr_render_current() record_draw(DRAW_TUTORIAL)
#include "../source/levels_lv.c"
#undef set_code_editable

#define dw_draw_iface_box_with_status(box, header, success) record_draw(DRAW_RESULT_BOX)
#define dw_draw_wrapped_texture_by_h(rect, height, message) record_draw(DRAW_RESULT_MESSAGE)
#define bt_draw_iface_btn(button, blocked, sound) draw_result_button(button)
#include "../source/ui/run_result_rr.c"

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

static void initialize_result_fixture(void)
{
	static texture_t header;
	static texture_array_t message;
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
	for (int event = DRAW_FRAME; event <= DRAW_LEVEL_UI; event++) {
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
	puts("Run-result render order and button tests passed");
	return 0;
}