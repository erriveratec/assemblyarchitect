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

static int operation;
static run_result_action_t action;
static bool return_released;
static bool step_clicked;
static bool run_ended;
static bool satisfied;
static int reset_calls;
static int destroy_calls;
static int result_reset_calls;
static int unlock_calls;
static int win_condition_calls;
static int unlocked_player;
static int unlocked_level;
static int execution_calls;
static int electron_creates;
static int electron_destroys;
static int drawing_calls;
static bool probe_pointer;
static bool expect_background_pointer;
static bool reset_menu_active;
static int result_update_calls;
static int background_checks;

static bool check_background_pointer(void)
{
	if (probe_pointer) {
		assert((ms_left_pressed() || ms_left_released()) == expect_background_pointer);
		assert((ms_right_pressed() || ms_right_released()) == expect_background_pointer);
		assert((ms_chord_pressed() || ms_chord_released()) == expect_background_pointer);
		if (!expect_background_pointer) {
			assert(ms_get_mouse_scroll_x() == 0);
			assert(ms_get_mouse_scroll_y() == 0);
		}
		background_checks++;
	}
	return false;
}

static run_result_action_t update_result(int operation_id)
{
	(void)operation_id;
	result_update_calls++;
	if (probe_pointer) {
		assert(ms_left_pressed() || ms_left_released());
		assert(ms_right_pressed() || ms_right_released());
	}
	return action;
}

static void inject_pointer(bool pressed)
{
	ms_reset_mouse_values();
	SDL_Event event = {0};
	event.type = pressed ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
	event.button.state = pressed ? SDL_PRESSED : SDL_RELEASED;
	event.button.button = SDL_BUTTON_LEFT;
	ms_mouse_button_handler(event);
	event.button.button = SDL_BUTTON_RIGHT;
	ms_mouse_button_handler(event);
	event.type = SDL_MOUSEWHEEL;
	event.wheel.x = 1;
	event.wheel.y = 2;
	ms_mouse_wheel_handler(event);
}

static void test_reset_operation(void);
static void test_unlock(int player_id, int level_id);
static void test_forbid_save(int player_id, int level_id);

#define SDL_GetTicks64() ((Uint64)0)
#define fx_electron_create(renderer, width, height, path) (++electron_creates, (fx_electron_t *)NULL)
#define fx_electron_update(electron, delta) ((void)0)
#define fx_electron_render(electron, renderer) ((void)0)
#define aa_electron_fx_destroy(electron) ((void)++electron_destroys)
#define sb_chck_rel_ret_btn() (check_background_pointer(), return_released)
#define sb_chk_click_stage_btn() (check_background_pointer(), step_clicked)
#define identify_clicked_stage_button() STEP
#define sb_chk_hov_rst_ret_btns() false
#define sb_chk_rel_rst_btn() check_background_pointer()
#define ms_left_pressed() false
#define ms_left_released() false
#define ms_consume_left_press() ((void)0)
#define ms_reset_mouse_values() ((void)0)
#define rm_chk_rst_menu_state() reset_menu_active
#define rm_chk_rst_menu_btns(active) false
#define rm_set_rst_menu(active) ((void)0)
#define rm_render_rst_menu(active) ((void)0)
#define em_get_escape_state() false
#define em_render_escape_menu(active) ((void)0)
#define rr_update(operation_id) update_result(operation_id)
#define rr_reset_state() ((void)++result_reset_calls)
#define rr_render(operation_id) ((void)++drawing_calls)
#define mc_get_operation_flag() (operation)
#define mc_set_operation_flag(operation_id) ((void)(operation = (operation_id)))
#define mc_get_run_ended() (run_ended)
#define mc_set_run_ended(state) ((void)(run_ended = (state)))
#define mc_get_step_ended() true
#define mc_start_execution(state) ((void)0)
#define mc_run_code() ((void)++execution_calls)
#define mc_reset_invalid_operation_flag() test_reset_operation()
#define mc_reset_avatar() ((void)++reset_calls)
#define mc_destroy_avatar_textures() ((void)++destroy_calls)
#define mc_draw_avatar() ((void)0)
#define wc_is_satisfied() (satisfied)
#define wc_reset_expected_output() ((void)0)
#define wc_reset_condition() ((void)(satisfied = false))
#define wc_destroy_expected_output() ((void)0)
#define bf_set_win_condition() ((void)++win_condition_calls)
#define bf_reset_win_condition() ((void)0)
#define bf_reset_input_list() ((void)0)
#define bf_reset_output_list() ((void)0)
#define bf_reset_input_list_x_pos() ((void)0)
#define bf_destroy_buffer_lists() ((void)0)
#define bf_draw_buffers(limit) ((void)0)
#define rg_reset_register_values() ((void)0)
#define rg_reset_ibox() ((void)0)
#define rg_reset_obox() ((void)0)
#define rg_reset_rflags() ((void)0)
#define rg_destroy_register_list() ((void)0)
#define rg_destroy_value_boxes() ((void)0)
#define rg_draw_registers(limit) ((void)0)
#define oh_clear() ((void)0)
#define cw_reset_code_execution() ((void)0)
#define cw_destroy_code_window_assets() ((void)0)
#define cw_sort_code() ((void)0)
#define cw_is_operand_pending() false
#define cw_chk_click_code_op() check_background_pointer()
#define cw_chk_click_code() false
#define cw_chk_rclick_code() false
#define cw_draw_code_window() ((void)0)
#define cw_get_challenge_highlight_limit() 0.0f
#define iw_chk_click_ins() false
#define iw_draw_ins_box() ((void)0)
#define iw_destroy_instruction_list() ((void)0)
#define tr_clear() ((void)0)
#define lv_set_hold_line(line) ((void)0)
#define lv_prepare_level_frame(level_id, operation_id) ((void)check_background_pointer())
#define lv_level_drawings(level_id) ((void)0)
#define sb_draw_stage_btns(size, animation_limit) ((void)0)
#define sb_draw_ret_btn() ((void)0)
#define sb_draw_rst_btn() ((void)0)
#define im_draw_imm(limit) ((void)0)
#define ar_display_arrow(arrow, limit) ((void)0)
#define ar_hide_execution_arrow() ((void)0)
#define ar_reset_execution_arrow() ((void)0)
#define ax_set_fast_move_delta(fast) ((void)0)
#define ax_set_arrow_mdelta(fast) ((void)0)
#define fl_enable_next_level test_unlock
#define fl_save_level test_forbid_save
#include "../source/stages.c"

static void test_reset_operation(void)
{
	operation = NO_OPERATION;
}

static void test_unlock(int player_id, int level_id)
{
	assert(level_id >= 0 && level_id < LV_LEVEL_MAX);
	unlock_calls++;
	unlocked_player = player_id;
	unlocked_level = level_id;
}

static void test_forbid_save(int player_id, int level_id)
{
	(void)player_id;
	(void)level_id;
	assert(false && "Navigation must not save level code");
}

static int expected_sector(int level_id)
{
	const int sectors[] = {
		LV_SECTOR_0, LV_SECTOR_1, LV_SECTOR_2, LV_SECTOR_3, LV_SECTOR_4
	};
	int sector_index = level_id / 8;
	return sectors[sector_index < 4 ? sector_index : 4];
}

static void reset_fixture(void)
{
	operation = NO_OPERATION;
	action = RUN_RESULT_ACTION_NONE;
	return_released = false;
	step_clicked = false;
	run_ended = false;
	satisfied = false;
	reset_calls = 0;
	destroy_calls = 0;
	result_reset_calls = 0;
	unlock_calls = 0;
	win_condition_calls = 0;
	unlocked_player = -1;
	unlocked_level = -1;
	execution_calls = 0;
	electron_creates = 0;
	electron_destroys = 0;
	drawing_calls = 0;
	probe_pointer = false;
	expect_background_pointer = false;
	reset_menu_active = false;
	result_update_calls = 0;
	background_checks = 0;
	g_player = 2;
}

static void assert_exit(int level_id, int destination)
{
	assert(stage_level(level_id) == destination);
	assert(reset_calls == 1);
	assert(destroy_calls == 1);
	assert(electron_creates == 1);
	assert(electron_destroys == 1);
	assert(operation == NO_OPERATION);
	assert(!run_ended);
}

static void test_destination_helpers(void)
{
	assert(LV_SECTOR_1_START == 8);
	assert(LV_SECTOR_2_START == 16);
	assert(LV_SECTOR_3_START == 24);
	assert(LV_SECTOR_4_START == 32);
	for (int level_id = 0; level_id < LV_LEVEL_MAX; level_id++) {
		int sector = expected_sector(level_id);
		bool boundary = level_id == 7 || level_id == 15 ||
		                level_id == 23 || level_id == 31 ||
		                level_id == LV_LEVEL_MAX - 1;
		assert(get_sector_id(level_id) == sector);
		assert(get_post_level_destination(level_id, false) == sector);
		assert(get_post_level_destination(level_id, true) ==
		       (boundary ? LV_SELECT_SECTOR : sector));
	}
}

static void test_continue_and_return(void)
{
	for (int level_id = 0; level_id < LV_LEVEL_MAX; level_id++) {
		reset_fixture();
		operation = MC_WIN;
		action = RUN_RESULT_ACTION_CONTINUE;
		bool boundary = level_id == 7 || level_id == 15 ||
		                level_id == 23 || level_id == 31 ||
		                level_id == LV_LEVEL_MAX - 1;
		assert_exit(level_id, boundary ? LV_SELECT_SECTOR : expected_sector(level_id));
		assert(result_reset_calls == 3);
		assert(drawing_calls == 0);
		assert(unlock_calls == 0);
		for (int won = 0; won < 2; won++) {
			reset_fixture();
			operation = won ? MC_WIN : NO_OPERATION;
			return_released = true;
			assert_exit(level_id, expected_sector(level_id));
			assert(result_reset_calls == 2);
			assert(drawing_calls == 1);
			assert(unlock_calls == 0);
		}
	}
}

static void test_back_and_nonwinning_continue(void)
{
	const int operations[] = {NO_OPERATION, MC_WIN, OUTPUT_BUFFER_INCOMPLETE};
	for (int level_id = 0; level_id < LV_LEVEL_MAX; level_id++) {
		for (int index = 0; index < 3; index++) {
			reset_fixture();
			operation = operations[index];
			action = RUN_RESULT_ACTION_BACK;
			assert(stage_level(level_id) == LV_PLAY_LEVEL);
			assert(reset_calls == 1 && destroy_calls == 0);
			assert(operation == NO_OPERATION);
			assert(result_reset_calls == 1);
			assert(electron_destroys == 0 && unlock_calls == 0);
			action = RUN_RESULT_ACTION_NONE;
			return_released = true;
			reset_calls = 0;
			assert_exit(level_id, expected_sector(level_id));
		}
		for (int index = 0; index < 3; index++) {
			if (operations[index] == MC_WIN) continue;
			reset_fixture();
			operation = operations[index];
			action = RUN_RESULT_ACTION_CONTINUE;
			assert(stage_level(level_id) == LV_PLAY_LEVEL);
			assert(reset_calls == 0 && destroy_calls == 0);
			assert(result_reset_calls == 0);
			assert(operation == operations[index]);
			assert(electron_destroys == 0 && unlock_calls == 0);
			return_released = true;
			assert_exit(level_id, expected_sector(level_id));
		}
	}
}

static void test_back_clears_execution_flags(void)
{
	const int levels[] = {7, 15, 23, 31, LV_LEVEL_MAX - 1};
	for (int index = 0; index < 5; index++) {
		int level_id = levels[index];
		reset_fixture();
		step_clicked = true;
		assert(stage_level(level_id) == LV_PLAY_LEVEL);
		assert(execution_calls == 1);
		step_clicked = false;
		operation = MC_WIN;
		run_ended = true;
		satisfied = true;
		action = RUN_RESULT_ACTION_BACK;
		assert(stage_level(level_id) == LV_PLAY_LEVEL);
		assert(reset_calls == 1 && destroy_calls == 0);
		assert(operation == NO_OPERATION && !run_ended);
		assert(execution_calls == 1 && unlock_calls == 0);
		action = RUN_RESULT_ACTION_NONE;
		run_ended = true;
		satisfied = true;
		assert(stage_level(level_id) == LV_PLAY_LEVEL);
		assert(operation == NO_OPERATION);
		assert(execution_calls == 1 && unlock_calls == 0);
		reset_calls = 0;
		return_released = true;
		assert_exit(level_id, expected_sector(level_id));
		assert(unlock_calls == 0);
	}
}

static void test_unlock_requires_completed_execution(void)
{
	for (int level_id = 0; level_id < LV_LEVEL_MAX; level_id++) {
		for (int missing_condition = 0; missing_condition < 3; missing_condition++) {
			reset_fixture();
			step_clicked = missing_condition != 0;
			run_ended = missing_condition != 1;
			satisfied = missing_condition != 2;
			assert(stage_level(level_id) == LV_PLAY_LEVEL);
			assert(operation == NO_OPERATION);
			assert(unlock_calls == 0);
			assert(execution_calls == (step_clicked ? 1 : 0));
			step_clicked = false;
			run_ended = false;
			return_released = true;
			assert_exit(level_id, expected_sector(level_id));
			assert(unlock_calls == 0);
		}
	}
}

static void test_execution_unlock_and_replay(void)
{
	for (int level_id = 0; level_id < LV_LEVEL_MAX; level_id++) {
		for (int replay = 0; replay < 2; replay++) {
			reset_fixture();
			step_clicked = true;
			run_ended = true;
			satisfied = true;
			assert(stage_level(level_id) == LV_PLAY_LEVEL);
			assert(execution_calls == 1);
			assert(operation == MC_WIN);
			assert(reset_calls == 0 && destroy_calls == 0);
			assert(unlock_calls == (level_id + 1 < LV_LEVEL_MAX ? 1 : 0));
			if (unlock_calls != 0) {
				assert(unlocked_player == g_player);
				assert(unlocked_level == level_id + 1);
			}
			step_clicked = false;
			run_ended = false;
			action = RUN_RESULT_ACTION_CONTINUE;
			bool boundary = level_id == 7 || level_id == 15 ||
			                level_id == 23 || level_id == 31 ||
			                level_id == LV_LEVEL_MAX - 1;
			assert_exit(level_id, boundary ? LV_SELECT_SECTOR : expected_sector(level_id));
			assert(unlock_calls == (level_id + 1 < LV_LEVEL_MAX ? 1 : 0));
		}
	}
}

static void test_win_result_repeated_frames(void)
{
	const int levels[] = {1, LV_SECTOR_1_START - 1, LV_LEVEL_MAX - 1};
	const int destinations[] = {LV_SECTOR_0, LV_SELECT_SECTOR, LV_SELECT_SECTOR};
	const int expected_unlocks[] = {1, 1, 0};
	for (int index = 0; index < 3; index++) {
		int level_id = levels[index];
		reset_fixture();
		step_clicked = true;
		run_ended = true;
		satisfied = true;
		assert(stage_level(level_id) == LV_PLAY_LEVEL);
		assert(operation == MC_WIN);
		assert(unlock_calls == expected_unlocks[index]);
		assert(win_condition_calls == 1);
		step_clicked = false;
		for (int frame = 0; frame < 5; frame++) {
			assert(action == RUN_RESULT_ACTION_NONE);
			assert(stage_level(level_id) == LV_PLAY_LEVEL);
			assert(operation == MC_WIN);
			assert(run_ended && satisfied);
			assert(unlock_calls == expected_unlocks[index]);
			assert(win_condition_calls == 1);
		}
		action = RUN_RESULT_ACTION_CONTINUE;
		assert_exit(level_id, destinations[index]);
		assert(unlock_calls == expected_unlocks[index]);
		assert(win_condition_calls == 1);
		printf("Repeated win frames: level %d, unlocks %d, win-condition calls %d\n",
		       level_id, unlock_calls, win_condition_calls);
	}
}

static void test_result_pointer_priority(void)
{
	const int operations[] = {NO_OPERATION, OUTPUT_BUFFER_INCOMPLETE, MC_WIN};
	for (size_t index = 0; index < sizeof(operations) / sizeof(operations[0]); index++) {
		for (int pressed = 0; pressed <= 1; pressed++) {
			reset_fixture();
			operation = operations[index];
			probe_pointer = true;
			expect_background_pointer = operation == NO_OPERATION;
			inject_pointer(pressed);
			assert(stage_level(LV_LEVEL_13) == LV_PLAY_LEVEL);
			assert(result_update_calls == 1);
			assert(background_checks >= 5);
			assert(execution_calls == 0 && reset_calls == 0);
			probe_pointer = false;
			return_released = true;
			assert_exit(LV_LEVEL_13, LV_SECTOR_1);
		}
	}
	reset_fixture();
	operation = MC_WIN;
	reset_menu_active = true;
	probe_pointer = true;
	expect_background_pointer = true;
	inject_pointer(false);
	assert(stage_level(LV_LEVEL_13) == LV_PLAY_LEVEL);
	assert(result_update_calls == 0);
	assert(background_checks >= 5);
	probe_pointer = false;
	reset_menu_active = false;
	return_released = true;
	assert_exit(LV_LEVEL_13, LV_SECTOR_1);
	for (size_t index = 1; index < sizeof(operations) / sizeof(operations[0]); index++) {
		reset_fixture();
		operation = operations[index];
		action = RUN_RESULT_ACTION_BACK;
		probe_pointer = true;
		inject_pointer(false);
		assert(stage_level(LV_LEVEL_13) == LV_PLAY_LEVEL);
		assert(operation == NO_OPERATION && reset_calls == 1);
		assert(background_checks >= 5 && execution_calls == 0);
		probe_pointer = false;
		action = RUN_RESULT_ACTION_NONE;
		return_released = true;
		reset_calls = 0;
		assert_exit(LV_LEVEL_13, LV_SECTOR_1);
	}
	reset_fixture();
	operation = MC_WIN;
	action = RUN_RESULT_ACTION_CONTINUE;
	probe_pointer = true;
	inject_pointer(false);
	assert_exit(LV_LEVEL_13, LV_SECTOR_1);
	assert(background_checks >= 5 && execution_calls == 0);
	probe_pointer = false;
}

int main(void)
{
	test_destination_helpers();
	test_continue_and_return();
	test_back_and_nonwinning_continue();
	test_back_clears_execution_flags();
	test_unlock_requires_completed_execution();
	test_execution_unlock_and_replay();
	test_win_result_repeated_frames();
	test_result_pointer_priority();
	puts("Sector completion navigation tests passed (all levels, boundaries, exits, replay, repeated frames, unlock guard)");
	return 0;
}