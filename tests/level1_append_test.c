#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "aux.h"
#include "buffers_bf.h"
#include "code_line_cl.h"
#include "code_window_cw.h"
#include "dimensions_dm.h"
#include "file_fl.h"
#include "game_mechanics_mc.h"
#include "gameplay/code_state_cs.h"
#include "gameplay/interaction_rules_ir.h"
#include "gameplay/win_condition_wc.h"
#include "immediates_im.h"
#include "instruction_window_iw.h"
#include "levels_lv.h"
#include "mouse_ms.h"
#include "registers_rg.h"
#include "stages.h"
#include "sdl_config.h"
#include "stage_buttons_sb.h"
#include "text_tx.h"
#include "tutorial_tr.h"
#include "ui/escape_menu_em.h"
#include "ui/reset_menu_rm.h"
#include "ui/run_result_rr.h"

static SDL_Surface *test_surface;
static SDL_Renderer *test_renderer;

static void initialize_game_assets(void)
{
	assert(SDL_Init(0) == 0);
	assert((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != 0);
	assert(TTF_Init() == 0);
	dm_set_screen_resolution(DM_RESOLUTION_1920X1080);
	test_surface = SDL_CreateRGBSurfaceWithFormat(
		0, dm_get_screen_width(), dm_get_screen_height(), 32,
		SDL_PIXELFORMAT_RGBA32);
	assert(test_surface != NULL);
	test_renderer = SDL_CreateSoftwareRenderer(test_surface);
	assert(test_renderer != NULL);
	g_screen = test_surface;
	g_renderer = test_renderer;
	g_width = dm_get_screen_width();
	g_height = dm_get_screen_height();
	ms_init_mouse();
	assert(load_media() == SUCCESS);
	em_init_escape_menu();
	rm_init_rst_menu();
	sb_init_ret_btn();
	sb_init_rst_btn();
	tx_init_global_msgs();
	iw_init_ins_box();
	rg_init_reg_texture();
	bf_init_buffer_assets();
	assert(rr_initialize());
	cw_init_code_window_texture();
	im_init_imm_assets();
}

static void send_left_button(bool pressed, int x, int y)
{
	ms_clear_mouse_values();
	SDL_Event event = {0};
	event.type = pressed ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
	event.button.button = SDL_BUTTON_LEFT;
	event.button.state = pressed ? SDL_PRESSED : SDL_RELEASED;
	event.button.x = x;
	event.button.y = y;
	ms_mouse_button_handler(event);
}

static void assert_interaction_cleared(void)
{
	stages_edit_test_state_t state = stages_edit_state_for_test();
	assert(state.line == NULL);
	assert(!state.authorized);
	assert(get_hold_line() == NULL);
}

static void settle_code(void)
{
	for (int frame = 0; frame < 1000 && !cw_check_code_sorted(); frame++) {
		cw_sort_code();
	}
	assert(cw_check_code_sorted());
}

static void establish_reorder(aa_program_t *program)
{
	stages_cancel_pending_edit();
	ms_reset_mouse_values();
	cw_clear_code_list();
	char first[] = "MOV [ob], rax";
	char last[] = "MOV rax, [ib]";
	cw_add_saved_line(first);
	cw_add_saved_line(last);
	assert(stages_refresh_program_snapshot(program));
	assert(tr_load_level(1));
	tr_deactivate("review_instruction_order");
	cs_context_t context = cs_capture_context();
	const tutorial_step_t *step = tr_update(&context);
	assert(step != NULL && strcmp(step->name, "reorder_instructions") == 0);
	assert(step->effect_code_editable_exception ==
		TUTORIAL_EDIT_EXCEPTION_LAST_LINE);
	assert(!ir_get_current_rules()->code_editable);
	assert(ir_get_current_rules()->code_editable_exception == 2);
	assert(ir_is_arrange_enabled());
	settle_code();
}

static int instruction_x(void)
{
	SDL_Rect instruction = cw_get_code_line_at_pos(0)->ins->b->r;
	return instruction.x + instruction.w / 2;
}

static void pick_up_last(aa_program_t *program)
{
	settle_code();
	send_left_button(true, instruction_x(), cw_get_code_line_at_pos(1)->ins->b->r.y + 8);
	stage_level(1, program);
	assert(get_hold_line() == cw_get_code_line_at_pos(1));
	assert(stages_edit_state_for_test().authorized);
	send_left_button(true, instruction_x(), cw_get_code_line_y(0) + 8);
	stage_level(1, program);
	cs_context_t context = cs_capture_context();
	const tutorial_step_t *step = tr_get_matching_step(&context);
	assert(step != NULL && strcmp(step->name, "reorder_instructions_holding") == 0);
	assert(!ir_get_current_rules()->code_editable);
	assert(ir_is_arrange_enabled());
}

static void release_at_first(aa_program_t *program)
{
	send_left_button(false, instruction_x(), cw_get_code_line_y(0) + 8);
	stage_level(1, program);
	stage_level(1, program);
}

static void disable_arrange_before_edit(void)
{
	ir_set_arrange_enabled(false);
	stages_before_edit_for_test = NULL;
}

static void invalidate_bindings_before_edit(void)
{
	cw_clear_domain_bindings();
	stages_before_edit_for_test = NULL;
}

static void start_execution_before_edit(void)
{
	mc_start_execution(true);
	stages_before_edit_for_test = NULL;
}

static void assert_unchanged(aa_program_t *program, code_line_t *first,
						   code_line_t *last, uint64_t revision)
{
	assert(cw_get_code_line_at_pos(0) == first);
	assert(cw_get_code_line_at_pos(1) == last);
	assert(aa_program_revision(program) == revision);
	assert(stages_edit_state_for_test().legacy_fallbacks == 0);
	assert_interaction_cleared();
}

static void test_reorder_rejections(aa_program_t *program)
{
	establish_reorder(program);
	code_line_t *first = cw_get_code_line_at_pos(0);
	code_line_t *last = cw_get_code_line_at_pos(1);
	uint64_t revision = aa_program_revision(program);
	send_left_button(true, instruction_x(), cw_get_code_line_y(0) + 8);
	stage_level(1, program);
	assert_interaction_cleared();
	release_at_first(program);
	assert_unchanged(program, first, last, revision);

	pick_up_last(program);
	stages_before_edit_for_test = disable_arrange_before_edit;
	release_at_first(program);
	assert_unchanged(program, first, last, revision);
	assert(cw_domain_bindings_valid(program));
	assert(stages_edit_state_for_test().snapshot_valid);

	pick_up_last(program);
	stages_before_edit_for_test = invalidate_bindings_before_edit;
	release_at_first(program);
	assert_unchanged(program, first, last, revision);
	assert(stages_edit_state_for_test().result == CW_EXISTING_EDIT_FAILED);
	assert(stages_refresh_program_snapshot(program));

	pick_up_last(program);
	stages_cancel_pending_edit();
	revision = aa_program_revision(program);
	assert_interaction_cleared();
	assert(!stages_edit_state_for_test().snapshot_valid);
	assert(stages_refresh_program_snapshot(program));
	release_at_first(program);
	assert_unchanged(program, first, last, revision);

	pick_up_last(program);
	ms_reset_mouse_values();
	assert_interaction_cleared();
	release_at_first(program);
	assert_unchanged(program, first, last, revision);

	pick_up_last(program);
	stages_before_edit_for_test = start_execution_before_edit;
	send_left_button(false, instruction_x(), cw_get_code_line_y(0) + 8);
	stage_level(1, program);
	assert_interaction_cleared();
	mc_start_execution(false);
	assert_unchanged(program, first, last, revision);

	pick_up_last(program);
	cw_clear_code_list();
	assert_interaction_cleared();
	establish_reorder(program);
}

static void test_delete_exception(aa_program_t *program)
{
	stages_cancel_pending_edit();
	ms_reset_mouse_values();
	cw_clear_code_list();
	char first[] = "MOV [ob], rax";
	char middle[] = "MOV rax, rax";
	char last[] = "MOV rax, rax";
	cw_add_saved_line(first);
	cw_add_saved_line(middle);
	cw_add_saved_line(last);
	assert(stages_refresh_program_snapshot(program));
	assert(tr_load_level(1));
	tr_deactivate("welcome");
	tr_deactivate("review_challenge");
	cs_context_t context = cs_capture_context();
	const tutorial_step_t *step = tr_update(&context);
	assert(step != NULL && strcmp(step->name, "select_last_instruction") == 0);
	assert(!ir_get_current_rules()->code_editable);
	assert(ir_get_current_rules()->code_editable_exception == 3);
	assert(!ir_is_arrange_enabled() && ir_is_delete_enabled());
	settle_code();
	uint64_t revision = aa_program_revision(program);
	send_left_button(true, instruction_x(), cw_get_code_line_at_pos(2)->ins->b->r.y + 8);
	stage_level(1, program);
	assert(get_hold_line() == cw_get_code_line_at_pos(2));
	send_left_button(true, 1, 1);
	stage_level(1, program);
	send_left_button(false, 1, 1);
	stage_level(1, program);
	stage_level(1, program);
	assert(cw_get_code_list_size() == 2 && aa_program_count(program) == 2);
	assert(aa_program_revision(program) == revision + 1);
	assert(cw_domain_bindings_valid(program));
	assert(stages_edit_state_for_test().snapshot_valid);
	assert(stages_edit_state_for_test().result == CW_EXISTING_EDIT_COMMITTED);
	assert(stages_edit_state_for_test().legacy_fallbacks == 0);
	assert_interaction_cleared();
}

static void test_level1_palette_append(void)
{
	const int level_id = 1;
	aa_program_t *program = aa_program_create();
	assert(program != NULL);
	g_player = FL_PLAYER_1;
	init_level(level_id, program);
	assert(iw_get_instruction_list_size() == 1);
	test_delete_exception(program);
	test_reorder_rejections(program);
	code_line_t *first = cw_get_code_line_at_pos(0);
	code_line_t *last = cw_get_code_line_at_pos(1);
	aa_instruction_id_t first_id, last_id;
	assert(cw_get_domain_instruction_id(first, &first_id));
	assert(cw_get_domain_instruction_id(last, &last_id));
	uint64_t move_revision = aa_program_revision(program);
	int move_x = instruction_x();
	fprintf(stderr, "drag before from=1 to=0 legacy=%p,%p domain=%llu,%llu\n",
		(void *)first, (void *)last, (unsigned long long)first_id,
		(unsigned long long)last_id);
	send_left_button(true, move_x, last->ins->b->r.y + 8);
	stage_level(level_id, program);
	assert(get_hold_line() == last);
	send_left_button(true, move_x, cw_get_code_line_y(0) + 8);
	stage_level(level_id, program);
	send_left_button(false, move_x, cw_get_code_line_y(0) + 8);
	stage_level(level_id, program);
	stage_level(level_id, program);
	fprintf(stderr, "drag after legacy=%p,%p domain=%llu,%llu revision=%llu\n",
		(void *)cw_get_code_line_at_pos(0), (void *)cw_get_code_line_at_pos(1),
		(unsigned long long)aa_program_instruction_at(program, 0)->id,
		(unsigned long long)aa_program_instruction_at(program, 1)->id,
		(unsigned long long)aa_program_revision(program));
	assert(cw_get_code_line_at_pos(0) == last);
	assert(cw_get_code_line_at_pos(1) == first);
	assert(aa_program_revision(program) == move_revision + 1);
	assert(aa_program_instruction_at(program, 0)->id == last_id);
	assert(aa_program_instruction_at(program, 1)->id == first_id);
	aa_instruction_id_t moved_id;
	assert(cw_get_domain_instruction_id(last, &moved_id));
	assert(moved_id == last_id);
	assert(cw_domain_bindings_valid(program));
	assert(stages_edit_state_for_test().snapshot_valid);
	assert(stages_edit_state_for_test().result == CW_EXISTING_EDIT_COMMITTED);
	assert(stages_edit_state_for_test().legacy_fallbacks == 0);
	assert_interaction_cleared();
	cs_context_t context = cs_capture_context();
	const tutorial_step_t *step = tr_update(&context);
	assert(step != NULL && strcmp(step->name, "run_program") == 0);
	send_left_button(false, move_x, cw_get_code_line_y(0) + cw_get_code_line_spacing() + 8);
	stage_level(level_id, program);
	stage_level(level_id, program);
	assert(cw_get_code_line_at_pos(0) == last);
	assert(aa_program_revision(program) == move_revision + 1);
	assert_interaction_cleared();
	tr_deactivate("run_program");
	tr_deactivate("reorder_instructions_holding_first");
	ir_rules_t base_rules = *ir_get_base_rules();
	ir_rules_t restricted_rules = base_rules;
	restricted_rules.code_editable = false;
	restricted_rules.code_editable_exception = IR_INSTRUCTION_EXCEPTION;
	ir_set_base_rules(&restricted_rules);

	SDL_Rect palette = iw_get_instruction_rect_by_pos(0);
	int palette_x = palette.x + palette.w / 2;
	int palette_y = palette.y + palette.h / 2;
	send_left_button(true, palette_x, palette_y);
	stage_level(level_id, program);
	code_line_t *held_line = get_hold_line();
	assert(held_line != NULL);
	assert(held_line->ins->id == MOV);
	assert(held_line->op1 == NULL && held_line->op2 == NULL);
	assert(held_line->state == MISSING_BOTH);

	uint64_t revision = aa_program_revision(program);
	SDL_Rect code_box = cw_get_stage_code_box();
	int drop_x = code_box.x + code_box.w / 2;
	int drop_y = code_box.y + code_box.h - 8;
	send_left_button(false, drop_x, drop_y);
	stage_level(level_id, program);

	assert(aa_program_count(program) == 3);
	assert(cw_get_code_list_size() == 3);
	assert(aa_program_revision(program) == revision + 1);
	code_line_t *appended = cw_get_code_line_at_pos(2);
	assert(appended == held_line);
	assert(cw_check_if_in_code_list(appended));
	assert(cw_domain_bindings_valid(program));
	aa_instruction_id_t appended_id = AA_INSTRUCTION_ID_INVALID;
	assert(cw_get_domain_instruction_id(appended, &appended_id));
	assert(appended_id != AA_INSTRUCTION_ID_INVALID);
	assert(aa_program_find_by_id(program, appended_id) != NULL);
	assert(get_hold_line() == NULL);
	assert(!cs_capture_context().holding_instruction);
	assert_interaction_cleared();
	ir_set_base_rules(&base_rules);
	uint64_t committed_revision = aa_program_revision(program);
	assert(stages_reconcile_program_snapshot(program));
	assert(aa_program_revision(program) == committed_revision);
	assert(stages_refresh_program_snapshot(program));
	assert(cw_domain_bindings_valid(program));

	cw_clear_code_list();
	aa_program_clear(program);
	fl_load_save_file(g_player, level_id);
	assert(cw_get_code_list_size() == 3);
	code_line_t *reloaded = cw_get_code_line_at_pos(2);
	assert(reloaded->ins->id == MOV);
	assert(reloaded->state == MISSING_BOTH);
	assert(reloaded->op1 == NULL && reloaded->op2 == NULL);
	assert(stages_refresh_program_snapshot(program));
	const aa_instruction_t *reloaded_instruction =
		aa_program_instruction_at(program, 2);
	assert(reloaded_instruction != NULL);
	assert(reloaded_instruction->operands[0].kind == AA_OPERAND_NONE);
	assert(reloaded_instruction->operands[1].kind == AA_OPERAND_NONE);

	cw_clear_code_list();
	aa_program_clear(program);
	char saved_label[] = "line _";
	char saved_jmp[] = "JMP _";
	char saved_je[] = "JE _";
	char saved_jne[] = "JNE _";
	char saved_mov_both_missing[] = "MOV _, _";
	char saved_mov_second_missing[] = "MOV rax, _";
	cw_add_saved_line(saved_label);
	cw_add_saved_line(saved_jmp);
	cw_add_saved_line(saved_je);
	cw_add_saved_line(saved_jne);
	cw_add_saved_line(saved_mov_both_missing);
	cw_add_saved_line(saved_mov_second_missing);
	assert(cw_update_saved_jump_instructions());
	assert(stages_refresh_program_snapshot(program));
	fl_save_level(g_player, level_id);

	cw_clear_code_list();
	aa_program_clear(program);
	fl_load_save_file(g_player, level_id);
	assert(cw_get_code_list_size() == 6);
	assert(cw_get_code_line_at_pos(0)->state == COMPLETE);
	assert(cw_get_code_line_at_pos(1)->state == MISSING_OP1);
	assert(cw_get_code_line_at_pos(2)->state == MISSING_OP1);
	assert(cw_get_code_line_at_pos(3)->state == MISSING_OP1);
	assert(cw_get_code_line_at_pos(4)->state == MISSING_BOTH);
	assert(cw_get_code_line_at_pos(5)->state == MISSING_OP2);
	assert(stages_refresh_program_snapshot(program));
	assert(cw_domain_bindings_valid(program));

	stages_cancel_pending_edit();
	cw_destroy_code_window_assets();
	iw_destroy_instruction_list();
	tr_clear();
	bf_destroy_buffer_lists();
	wc_destroy_expected_output();
	rg_destroy_register_list();
	rg_destroy_value_boxes();
	mc_destroy_avatar_textures();
	rr_destroy();
	IMG_Quit();
	close_sdl();
	SDL_FreeSurface(test_surface);
	aa_program_destroy(program);
}

int main(void)
{
	initialize_game_assets();
	test_level1_palette_append();
	puts("Level 1 arrange, authorization, deletion and incomplete-save tests passed");
	return 0;
}