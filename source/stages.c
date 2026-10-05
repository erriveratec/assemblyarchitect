#include <assert.h>
#include <stdbool.h>
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
#include "dbg.h"

typedef struct level_flags_t {
	bool play;
	bool stop;
	bool stop_enabled;
	bool fast;
	bool step;
	bool step_fst;
	bool non_stop;
} level_flags_t;

int g_player = FL_NO_PLAYER;

static code_line_t *g_edit_line;
static code_line_t *g_stage_hold_line;
static bool g_edit_hold_line;
static bool g_domain_step_execution_active;
static bool g_domain_teardown_active;
static bool g_domain_append_candidate;
static bool g_domain_existing_candidate;
static bool g_program_snapshot_valid;
static aa_instruction_id_t g_edit_authorized_id = AA_INSTRUCTION_ID_INVALID;
static bool g_edit_pickup_authorized;
static bool g_edit_may_move;
static bool g_edit_may_delete;
#ifdef STAGES_EDIT_TESTING
void (*stages_before_edit_for_test)(void);
static int g_edit_test_result = -1;
static unsigned g_edit_test_legacy_fallbacks;

stages_edit_test_state_t stages_edit_state_for_test(void)
{
	return (stages_edit_test_state_t){
		.line = g_edit_line,
		.authorized = g_edit_pickup_authorized,
		.snapshot_valid = g_program_snapshot_valid,
		.result = g_edit_test_result,
		.legacy_fallbacks = g_edit_test_legacy_fallbacks
	};
}
#endif
static bool g_control_flow_repair_failed;

void                stage_drawings(int level, int operation_id);
static code_line_t *pending_operand_handler(bool *semantic_changed);
static void         flag_handler(level_flags_t *flags, int clicked_button);
static code_line_t *edit_code(int level_id, aa_program_t *program,
							  bool step_execution_active,
							  bool teardown_active);
static void         reset_level(int level_id, level_flags_t *flags);
static void         destroy_level(level_flags_t *flags);
static void         init_stage_assets();
static void         code_updated_actions(int level_id, aa_program_t *program);
static bool         refresh_program_snapshot(aa_program_t *program);
static bool         reconcile_program_snapshot(aa_program_t *program);
static bool         append_domain_instruction(
	const aa_legacy_line_snapshot_t *snapshot,
	aa_instruction_id_t *created_id,
	void *context);
static bool         prepare_domain_append(size_t required_count, void *context);
static bool         edit_domain_instruction(
	cw_existing_edit_kind_t kind,
	size_t from,
	size_t to,
	const aa_legacy_line_snapshot_t *snapshot,
	void *context);
static bool         domain_edit_context_allowed(const aa_program_t *program,
												 const code_line_t *line);
static bool         pending_jump_allows_instruction_click(void);
static bool         pending_jump_label_selected(void);
static void         save_and_update_code(int level_id);
static void         cancel_edit_interaction(void);
static void         set_code_editable();
static void         reset_code_editable();
static void         rst_btn_hdl(int level_id, level_flags_t *f,
								aa_program_t *program);
static int          get_sector_id(int level_id);

/* Function: reset_level_flags
 * -------------------------------------
 * Arguments:
 * 	flags: the flag levels that will be reset
 *
 * Return:
 *	void.
 */
void reset_level_flags(level_flags_t *flags)
{
	flags->play         = false;
	flags->stop         = false;
	flags->stop_enabled = false;
	flags->fast         = false;
	flags->step         = false;
	flags->step_fst     = false;
	flags->non_stop     = false;
}

/* Function: init_stage_assets
 * ----------------------------------------------------------------------------
 * Initializes several aspects of the stages that were initialized
 * as the level was created. Independly of the level that player is playing.
 * the stages should be initialized. Code box and instruction box should not
 * be initialized here.
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	void.
 */
static void init_stage_assets()
{
	sb_init_stage_btns();

	SDL_Rect r0 = bf_get_input_buffer_bounds();
	bf_set_input_box(r0);

	SDL_Rect r1 = bf_get_output_buffer_bounds();
	bf_set_output_box(r1);

	SDL_Rect ib = bf_get_input_label_anchor();
	SDL_Rect r2 = {.x = r0.x, .y = ib.y, .w = r0.w, .h = ib.h + r0.h};
	bf_set_input_buffer_button(r2);

	SDL_Rect ob = bf_get_output_label_anchor();
	SDL_Rect r3 = {.x = r1.x, .y = r1.y, .w = r1.w, .h = ob.h + r1.h};
	bf_set_output_buffer_button(r3);

	bf_init_buf_ops();
}

/* Function: init_level
 * -------------------------------------
 * Arguments:
 * stage_id: the id for the specific stage that is going to be played
 *
 * Return:
 *	true: used for the caller of the function.
 */
void init_level(int level_id, aa_program_t *program)
{
	assert(level_id >= 0 && level_id < LV_LEVEL_MAX && "Invalid stage id");
	assert(program != NULL && "Program owner is NULL");

	init_stage_assets();
	bf_create_input_list();
	bf_create_output_list();
	bf_reset_win_condition();

	if (!tr_load_level(level_id)) {
		fprintf(stderr, "Unable to load tutorial for level %d\n", level_id);
	}

	wc_create_expected_output();
	lv_init_level_assets(level_id);

	// goes before the load level
	SDL_Rect code_box = cw_get_stage_code_box();
	SDL_Rect register_size = rg_get_panel_size();
	SDL_Rect register_bounds = {
		.x = code_box.x + code_box.w,
		.y = register_size.y,
		.w = register_size.w,
		.h = register_size.h
	};
	rg_set_register_box(register_bounds);

	fl_file_initialize_level(level_id);

	// must go after level loading
	rg_init_flag_and_vboxes();

	SDL_Rect r1 = cw_get_stage_code_box();
	cw_set_scroll_box(r1);
	cw_set_code_box(r1);

	cw_create_code_list();
	fl_load_save_file(g_player, level_id);
	refresh_program_snapshot(program);
	mc_init_avatar();

	ar_initialize_arrows();
	return;
}

/* Function: destroy_level
 * -----------------------------------------------------------------------------
 * This function should be called each time the player leaves a level as it
 * should clean everything for the next invocation of the level.
 *
 * Arguments:
 * stage_id: the id for the specific stage that is going to be played
 *
 * Return:
 *	void
 */
static void destroy_level(level_flags_t *flags)
{
	cancel_edit_interaction();
	g_program_snapshot_valid = false;
	oh_clear();
	bf_destroy_buffer_lists();
	wc_destroy_expected_output();
	cw_destroy_code_window_assets();
	iw_destroy_instruction_list();
	tr_clear();
	rg_destroy_register_list();
	rg_destroy_value_boxes();
	reset_level_flags(flags);
	bf_reset_input_list_x_pos();
	mc_destroy_avatar_textures();
	return;
}

/* Function: stages_drawings
 * -----------------------------------------------------------------------------
 * Does all the drawings regarding a level
 *
 * Arguments:
 *	level: id of the level that is being rendered
 *
 * Return:
 *	Void.
 */
void stage_drawings(int level, int operation_id)
{
	iw_draw_ins_box();
	cw_draw_code_window();
	float animation_limit = cw_get_challenge_highlight_limit();
	sb_draw_stage_btns(cw_get_code_list_size());
	im_draw_imm(animation_limit);
	ar_display_arrow(AR_EXEC, animation_limit);
	bf_draw_buffers(animation_limit);
	rg_draw_registers(animation_limit);
	mc_draw_avatar();
	sb_draw_ret_btn();
	sb_draw_rst_btn();
	/*
	 * Draw the result before tutorial overlays so Level 0 can explain the
	 * first incomplete run while the result remains visible. Tutorial layouts
	 * must not cover result buttons.
	 */
	rr_render(operation_id);
	lv_level_drawings(level);
}

/* Function: stage_button_handler
 * ----------------------------------------------------------------------------
 * Arguments:
 * 	None.
 *
 * Return:
 *	void.
 */
void stage_button_handler()
{
	int clicked_button = identify_clicked_stage_button();

	switch (clicked_button) {
	case STOP:

		break;

	case STEP:

		break;

	case PLAY:

		break;

	case FAST:

		break;

	case INVALID:

		break;
	}
}

/* Function: flag_handler
 * ----------------------------------------------------------------------------
 * Sets the value of flags of the state of the game
 *
 * Arguments:
 * 	flags: level flags.
 *	clicked_button: ide of the stage button clicked by the player
 *
 * Return:
 *	void.
 */
static void flag_handler(level_flags_t *flags, int clicked_button)
{
	assert(clicked_button != INVALID && "Invalid clicked button");
	assert(flags != NULL && "The flags pointer is NULL");

	switch (clicked_button) {
	case PLAY:
		if (flags->play == false && flags->step_fst == false) {
			ar_reset_execution_arrow();
		}
		flags->play         = true;
		flags->non_stop     = true;
		flags->stop         = false;
		flags->stop_enabled = true;
		flags->fast         = false;
		flags->step         = false;
		flags->step_fst     = true;
		ax_set_fast_move_delta(false);
		ax_set_arrow_mdelta(false);
		break;
	case STOP:
		flags->stop     = true;
		flags->non_stop = false;
		flags->play     = false;
		flags->fast     = false;
		flags->step     = false;
		flags->step_fst = false;
		mc_reset_invalid_operation_flag();
		ax_set_fast_move_delta(false);
		ax_set_arrow_mdelta(false);
		break;
	case FAST:
		if (flags->play == false && flags->step_fst == false) {
			ar_reset_execution_arrow();
		}
		flags->fast         = true;
		flags->play         = true;
		flags->stop         = false;
		flags->non_stop     = true;
		flags->stop_enabled = true;
		flags->step         = false;
		flags->step_fst     = true;
		ax_set_fast_move_delta(true);
		ax_set_arrow_mdelta(true);
		break;
	case STEP:
		if (flags->step_fst == false) {
			ar_reset_execution_arrow();
		}
		flags->step_fst     = true;
		flags->step         = true;
		flags->stop         = false;
		flags->non_stop     = true;
		flags->stop_enabled = true;
		flags->play         = false;
		flags->fast         = false;
		ax_set_fast_move_delta(false);
		ax_set_arrow_mdelta(false);
		break;
	}
}
/* Function: code_updated_actions
 * ----------------------------------------------------------------------------
 * Performs all the actions that need to be done in the game logic when
 * the code has been updated with a new line or deleted line
 * Saves the state of the game.
 *
 * Arguments:
 * 	Void.
 *
 * Return:
 *	Void.
 */
/* Structural or unbound legacy changes use full import, then rebuild bindings
 * positionally from the resulting domain program. */
static bool refresh_program_snapshot(aa_program_t *program)
{
	aa_legacy_program_reader_t reader;
	aa_legacy_import_report_t report;
	g_program_snapshot_valid = false;
	cw_clear_domain_bindings();
	if (program == NULL) {
		return false;
	}
	if (!cw_get_legacy_program_reader(&reader)) {
		fprintf(stderr, "Could not access the active legacy code list\n");
		return false;
	}
	aa_result_t result = aa_legacy_program_import(program, &reader, &report);
	if (result != AA_RESULT_OK) {
		fprintf(stderr, "Could not refresh level program snapshot "
				"(result=%d, issue=%d, position=%zu, operand=%zu)\n",
				(int)result, (int)report.issue,
				report.position, report.operand_position);
		return false;
	}
	if (!cw_rebuild_domain_bindings(program)) {
		fprintf(stderr, "Could not rebuild legacy/domain instruction bindings\n");
		return false;
	}
	g_program_snapshot_valid = true;
	return true;
}

/* Bound same-count semantic edits reconcile against the live line identities. */
static bool reconcile_program_snapshot(aa_program_t *program)
{
	aa_legacy_program_reader_t reader;
	aa_legacy_import_report_t report;
	if (!g_program_snapshot_valid || !cw_domain_bindings_valid(program) ||
		!cw_get_legacy_program_reader(&reader)) {
		g_program_snapshot_valid = false;
		cw_clear_domain_bindings();
		fprintf(stderr, "Legacy/domain identity reconciliation is unavailable\n");
		return false;
	}
	aa_result_t result = aa_legacy_program_reconcile(program, &reader, &report);
	if (result != AA_RESULT_OK) {
		g_program_snapshot_valid = false;
		cw_clear_domain_bindings();
		fprintf(stderr, "Could not reconcile legacy program edit "
				"(result=%d, issue=%d, position=%zu, operand=%zu)\n",
				(int)result, (int)report.issue,
				report.position, report.operand_position);
		return false;
	}
	return true;
}

bool stages_refresh_program_snapshot(aa_program_t *program)
{
	return refresh_program_snapshot(program);
}

bool stages_reconcile_program_snapshot(aa_program_t *program)
{
	return reconcile_program_snapshot(program);
}

static bool append_domain_instruction(
	const aa_legacy_line_snapshot_t *snapshot,
	aa_instruction_id_t *created_id,
	void *context)
{
	aa_program_t *program = context;
	aa_instruction_t instruction;
	aa_legacy_import_report_t report;
	if (snapshot == NULL || created_id == NULL ||
		!domain_edit_context_allowed(program,
									(const code_line_t *)snapshot->identity) ||
		snapshot->bound_instruction_id != AA_INSTRUCTION_ID_INVALID) {
		return false;
	}
	if (snapshot->opcode == LABEL) {
		if (snapshot->has_operand_1 || snapshot->has_operand_2 ||
			snapshot->jump_target_identity != NULL ||
			snapshot->line_state != MISSING_OP1) {
			return false;
		}
		instruction = aa_instruction_create(AA_OPCODE_LABEL);
	} else if (cl_is_ins_jmp_type(snapshot->opcode)) {
		if (snapshot->has_operand_1 || snapshot->has_operand_2 ||
			snapshot->jump_target_identity != NULL ||
			snapshot->line_state != MISSING_OP1) {
			return false;
		}
		aa_opcode_t opcode = snapshot->opcode == JMP ? AA_OPCODE_JMP :
			snapshot->opcode == JE ? AA_OPCODE_JE : AA_OPCODE_JNE;
		instruction = aa_instruction_create(opcode);
	} else if (aa_legacy_instruction_from_snapshot(
			snapshot, &instruction, &report) != AA_RESULT_OK) {
		return false;
	}
	return aa_program_append(program, &instruction, created_id) == AA_RESULT_OK;
}

static bool prepare_domain_append(size_t required_count, void *context)
{
	aa_program_t *program = context;
	return required_count <= MAX_CODE_LINES &&
		aa_program_count(program) + 1 == required_count &&
		g_program_snapshot_valid && cw_domain_bindings_valid(program) &&
		aa_program_reserve(program, required_count) == AA_RESULT_OK;
}

static bool domain_edit_context_allowed(const aa_program_t *program,
											 const code_line_t *line)
{
	aa_instruction_id_t bound_id = AA_INSTRUCTION_ID_INVALID;
	bool authorized = g_domain_existing_candidate ?
		g_edit_pickup_authorized &&
		g_edit_authorized_id != AA_INSTRUCTION_ID_INVALID &&
		cw_check_if_in_code_list((code_line_t *)line) &&
		cw_get_domain_instruction_id(line, &bound_id) &&
		bound_id == g_edit_authorized_id :
		g_domain_append_candidate ? g_edit_pickup_authorized :
		lv_is_code_editable();
	return g_program_snapshot_valid && authorized &&
		cw_domain_bindings_valid(program) &&
		program != NULL && line != NULL &&
		g_edit_line == line && !mc_is_executing() &&
		!g_domain_step_execution_active && !g_domain_teardown_active &&
		aa_program_count(program) == (size_t)cw_get_code_list_size() &&
		(g_stage_hold_line == NULL || g_stage_hold_line == line);
}

#ifdef STAGES_EDIT_TESTING
static void trace_edit_context(const char *phase, aa_program_t *program)
{
	cs_context_t context = cs_capture_context();
	const tutorial_step_t *step = tr_get_matching_step(&context);
	fprintf(stderr, "drag %s step=%s editable=%d arrange=%d line=%p candidate=%d hold=%d stage_hold=%p allowed=%d revision=%llu\n",
		phase, step == NULL ? "none" : step->name, lv_is_code_editable(),
		lv_is_arrange_enabled(), (void *)g_edit_line,
		g_domain_existing_candidate, g_edit_hold_line,
		(void *)g_stage_hold_line, domain_edit_context_allowed(program, g_edit_line),
		(unsigned long long)aa_program_revision(program));
}
#endif

static bool pending_jump_allows_instruction_click(void)
{
	if (!iw_chk_click_ins()) {
		return false;
	}
	code_line_t *pending_line = cw_get_code_line_pending_operand();
	return pending_line != NULL && pending_line->ins != NULL &&
		pending_line->state == MISSING_OP1 &&
		cl_is_ins_jmp_type(pending_line->ins->id);
}

static bool pending_jump_label_selected(void)
{
	code_line_t *pending_line = cw_get_code_line_pending_operand();
	return pending_line != NULL && pending_line->ins != NULL &&
		pending_line->state == MISSING_OP1 &&
		cl_is_ins_jmp_type(pending_line->ins->id) && cw_ms_rel_in_label();
}

static bool domain_append_opcode(int opcode)
{
	switch (opcode) {
	case MOV:
	case ADD:
	case CMP:
	case LABEL:
	case JMP:
	case JE:
	case JNE:
		return true;
	default:
		return false;
	}
}

static bool domain_move_opcode(int opcode)
{
	switch (opcode) {
	case MOV:
	case ADD:
	case CMP:
	case LABEL:
	case JMP:
	case JE:
	case JNE:
		return true;
	default:
		return false;
	}
}

static bool edit_domain_instruction(
	cw_existing_edit_kind_t kind,
	size_t from,
	size_t to,
	const aa_legacy_line_snapshot_t *snapshot,
	void *context)
{
	aa_program_t *program = context;
	if (snapshot == NULL ||
		!domain_edit_context_allowed(program,
									(const code_line_t *)snapshot->identity) ||
		(kind == CW_EXISTING_EDIT_MOVE &&
		 (!g_edit_may_move || !lv_is_arrange_enabled())) ||
		(kind == CW_EXISTING_EDIT_REMOVE &&
		 (!g_edit_may_delete || !lv_is_del_enabled())) ||
		aa_program_count(program) != (size_t)cw_get_code_list_size() ||
		from >= aa_program_count(program) ||
		snapshot->bound_instruction_id == AA_INSTRUCTION_ID_INVALID) {
		return false;
	}
	const aa_instruction_t *existing =
		aa_program_instruction_at(program, from);
	if (existing == NULL || existing->id != snapshot->bound_instruction_id) {
		return false;
	}
	bool control_flow = snapshot->opcode == LABEL ||
		cl_is_ins_jmp_type(snapshot->opcode);
	if (control_flow) {
		if (kind != CW_EXISTING_EDIT_MOVE || snapshot->has_operand_2) {
			return false;
		}
		if (snapshot->opcode == LABEL) {
			if (existing->opcode != AA_OPCODE_LABEL ||
				existing->operand_count != 0 || !snapshot->has_operand_1 ||
			snapshot->line_state != COMPLETE ||
			snapshot->jump_target_identity != NULL) {
				return false;
			}
		} else {
			aa_opcode_t expected_opcode = snapshot->opcode == JMP ? AA_OPCODE_JMP :
				snapshot->opcode == JE ? AA_OPCODE_JE : AA_OPCODE_JNE;
			if (existing->opcode != expected_opcode ||
				existing->operand_count != 1) {
				return false;
			}
			if (!snapshot->has_operand_1) {
				if (snapshot->line_state != MISSING_OP1 ||
					snapshot->jump_target_identity != NULL ||
					existing->operands[0].kind != AA_OPERAND_NONE) {
					return false;
				}
			} else {
				aa_instruction_id_t target_id;
				code_line_t *target = (code_line_t *)
					snapshot->jump_target_identity;
				const aa_instruction_t *domain_target;
				if (snapshot->line_state != COMPLETE || target == NULL ||
					!cw_get_domain_instruction_id(target, &target_id) ||
					existing->operands[0].kind != AA_OPERAND_LABEL_REFERENCE ||
					existing->operands[0].value.label_instruction_id != target_id) {
					return false;
				}
				domain_target = aa_program_find_by_id(program, target_id);
				if (domain_target == NULL ||
					domain_target->opcode != AA_OPCODE_LABEL) {
					return false;
				}
			}
		}
	} else {
		aa_instruction_t legacy;
		aa_legacy_import_report_t report;
		if (aa_legacy_instruction_from_snapshot(snapshot, &legacy, &report) !=
			AA_RESULT_OK || existing->opcode != legacy.opcode ||
			existing->operand_count != legacy.operand_count) {
				return false;
			}
		for (size_t operand_position = 0;
			 operand_position < existing->operand_count; operand_position++) {
			const aa_operand_t *domain_operand =
				&existing->operands[operand_position];
			const aa_operand_t *legacy_operand =
				&legacy.operands[operand_position];
			if (domain_operand->kind != legacy_operand->kind) {
				return false;
			}
			switch (domain_operand->kind) {
			case AA_OPERAND_NONE:
				break;
			case AA_OPERAND_REGISTER:
				if (domain_operand->value.reg != legacy_operand->value.reg) {
					return false;
				}
				break;
			case AA_OPERAND_BUFFER:
				if (domain_operand->value.buffer != legacy_operand->value.buffer) {
					return false;
				}
				break;
			case AA_OPERAND_IMMEDIATE:
				if (domain_operand->value.immediate !=
					legacy_operand->value.immediate) {
					return false;
				}
				break;
			case AA_OPERAND_LABEL_REFERENCE:
				return false;
			default:
				return false;
			}
		}
	}
	if (kind == CW_EXISTING_EDIT_REMOVE) {
		if (control_flow) {
			return false;
		}
		return aa_program_remove(program, from) == AA_RESULT_OK;
	}
	if (kind == CW_EXISTING_EDIT_MOVE) {
		return aa_program_move(program, from, to) == AA_RESULT_OK;
	}
	return false;
}

static void save_and_update_code(int level_id)
{
	fl_save_level(g_player, level_id);
	lv_upd_level_assets(level_id);
}

static void cancel_edit_interaction(void)
{
	g_edit_pickup_authorized = false;
	g_edit_authorized_id = AA_INSTRUCTION_ID_INVALID;
	g_edit_may_move = false;
	g_edit_may_delete = false;
	if (g_edit_line != NULL && !cw_check_if_in_code_list(g_edit_line)) {
		cl_destroy_code_line(g_edit_line);
	}
	g_edit_line = NULL;
	g_stage_hold_line = NULL;
	g_edit_hold_line = false;
	g_domain_append_candidate = false;
	g_domain_existing_candidate = false;
	cw_clear_held_instruction();
	lv_set_hold_line(NULL);
}

void stages_forget_destroyed_line(const code_line_t *line)
{
	if (line != g_edit_line) {
		return;
	}
	g_edit_line = NULL;
	cancel_edit_interaction();
}

void stages_cancel_edit_interaction(void)
{
	cancel_edit_interaction();
}

void stages_cancel_pending_edit(void)
{
	cancel_edit_interaction();
	g_program_snapshot_valid = false;
	cw_clear_domain_bindings();
}

static void code_updated_actions(int level_id, aa_program_t *program)
{
	refresh_program_snapshot(program);
	save_and_update_code(level_id);
}

/* Function: pending_operand_handler
 * ----------------------------------------------------------------------------
 * This function is called when an instruction is pending an operand.
 * In case that the instruction that is pending an operand is a jump, it
 * generates the label operand to be put on the code
 *
 * Arguments:
 * 	Void.
 *
 * Return:
 *	NULL, if the peding operand is not a line, LABEL if the operand is a line.
 */
static code_line_t *pending_operand_handler(bool *semantic_changed)
{
	if (semantic_changed != NULL) {
		*semantic_changed = false;
	}
	bool reg_sel   = rg_chk_rel_in_reg();
	bool buf_sel   = bf_ms_rel_in_buf();
	bool label_sel = cw_ms_rel_in_label();
	bool imm_sel   = im_ms_rel_in_upimm();
	bool rel       = ms_left_released();

	code_line_t *l = cw_get_code_line_pending_operand();
	code_line_t *r = NULL;
	cw_highlight_code_pending_operand();

	if (cl_is_ins_jmp_type(l->ins->id) == true && l->state == MISSING_OP1) {
		code_line_t *target = NULL;
		if (label_sel) {
			target = cw_get_released_label_code_line();
		} else {
			r = cw_create_label_code_line();
			target = r;
			if (!cw_player_holding_instruction(r, false, true)) {
				g_control_flow_repair_failed = true;
				g_program_snapshot_valid = false;
			}
		}
		operand_t *a = cw_create_jmp_op(target);
		if (a != NULL) {
			cw_assign_op_to_line(a, l);
			*semantic_changed = true;
		} else {
			g_control_flow_repair_failed = true;
			g_program_snapshot_valid = false;
		}
	} else if (reg_sel == true && lv_is_reg_selectable() == true) {
		operand_t *r = rg_create_sel_reg_op();
		cw_assign_op_to_line(r, l);
		*semantic_changed = true;
	} else if (buf_sel == true && lv_is_buf_selectable() == true) {
		operand_t *b = bf_create_sel_buf_op();
		if (cl_is_op_compatible(b, l) == true) {
			cw_assign_op_to_line(b, l);
			*semantic_changed = true;
		} else {
			cl_destroy_operand(b);
		}
	} else if (imm_sel == true) {
		operand_t *i = im_create_sel_imm_op();
		if (cl_is_op_compatible(i, l) == true) {
			cw_assign_op_to_line(i, l);
			*semantic_changed = true;
		}
	} else if (rel == true && reg_sel == false && buf_sel == false &&
	           imm_sel == false) {
		if (l->state == CHANGING_OP1 || l->state == CHANGING_OP2) {
			int qty  = cl_get_instruction_operand_quantity(l->ins->id);
			bool missing_op2 = l->state == CHANGING_OP1 &&
								qty == TWO_OPERANDS && l->op2 == NULL;
			l->state = missing_op2 ? MISSING_OP2 : COMPLETE;
			if (qty == ONE_OPERAND || qty == TWO_OPERANDS) {
				l->op1->b->animated   = false;
				l->op1->b->anim_dir   = false;
				l->op1->b->anim_state = 0;
				if (cl_is_ins_jmp_type(l->ins->id) == true) {
					l->op1->jptr->op1->b->animated   = false;
					l->op1->jptr->op1->b->anim_dir   = false;
					l->op1->jptr->op1->b->anim_state = 0;
				}
			}
			if (qty == TWO_OPERANDS && l->op2 != NULL) {
				l->op2->b->animated   = false;
				l->op2->b->anim_dir   = false;
				l->op2->b->anim_state = 0;
			}
		}
	}
	return r;
}

/* Function: edit_code
 * ----------------------------------------------------------------------------
 * This function is called when the player is able to edit or alter the code
 * that is present in the code window.
 *
 * Arguments:
 * 	level_id: Required for the save file when the code is saved.
 *
 * Return:
 *	true if the player is holding a line, false if otherwise
 */
static code_line_t *edit_code(int level_id,
							  aa_program_t *program,
							  bool step_execution_active,
							  bool teardown_active)
{
	assert(level_id >= 0 && level_id <= LV_LEVEL_QUANTITY &&
	       "Incorrect level_id value");
	g_domain_step_execution_active = step_execution_active;
	g_domain_teardown_active = teardown_active;
#ifdef STAGES_EDIT_TESTING
	if (stages_before_edit_for_test != NULL) {
		stages_before_edit_for_test();
	}
#endif
	if (g_domain_existing_candidate) {
		g_edit_may_move = g_edit_may_move && lv_is_arrange_enabled();
		g_edit_may_delete = g_edit_may_delete && lv_is_del_enabled();
	}
	if (mc_is_executing() || step_execution_active || teardown_active ||
		rm_chk_rst_menu_state() || em_get_escape_state() ||
		(g_domain_existing_candidate &&
		 (!g_edit_may_move && !g_edit_may_delete))) {
		cancel_edit_interaction();
		return NULL;
	}

	bool                left_pressed  = ms_left_pressed();
	bool                left_released = ms_left_released();
	if (g_edit_line == NULL && left_pressed) {
		g_control_flow_repair_failed = false;
	}

	if (cw_is_operand_pending() == true && g_edit_line == NULL &&
	    cw_check_code_sorted() == true &&
	    ((cw_chk_click_code() == false && cw_chk_click_code_op() == false) ||
		 pending_jump_label_selected()) &&
	    !pending_jump_allows_instruction_click()) {
		bool semantic_changed = false;
		int line_count_before = cw_get_code_list_size();
		g_edit_line = pending_operand_handler(&semantic_changed);
		if (semantic_changed && !g_control_flow_repair_failed) {
			if (line_count_before == cw_get_code_list_size()) {
				if (reconcile_program_snapshot(program) &&
					cw_is_operand_pending() == false) {
					save_and_update_code(level_id);
				}
			} else if (cw_is_operand_pending() == false) {
				code_updated_actions(level_id, program);
			} else {
				refresh_program_snapshot(program);
			}
		} else if (cw_is_operand_pending() == false &&
				   !g_control_flow_repair_failed) {
			code_updated_actions(level_id, program);
		}
		if (g_edit_line != NULL) {
			g_edit_hold_line = true;
		}
	} else if (cw_chk_click_code_op() == true && g_edit_line == NULL &&
	           lv_is_code_editable() == true) {
		cw_change_clicked_code_line_state();
	} else if (iw_chk_click_ins() == true && g_edit_line == NULL &&
	           lv_is_code_editable() == true) {
		g_edit_line = cl_new_code_line(iw_get_clicked_instruction());
		/* Control-flow append is authoritative only in its initial incomplete form. */
		g_domain_append_candidate = g_edit_line != NULL &&
			domain_append_opcode(g_edit_line->ins->id) &&
			(!cl_is_ins_jmp_type(g_edit_line->ins->id) ||
			 (g_edit_line->op1 == NULL &&
			  g_edit_line->state == MISSING_OP1));
		g_edit_pickup_authorized = g_domain_append_candidate;
	} else if (cw_chk_click_code() == true && g_edit_line == NULL &&
	           lv_is_code_editable() == true) {
		g_edit_line = cw_get_clicked_code();
		g_domain_existing_candidate = g_edit_line != NULL &&
			domain_move_opcode(g_edit_line->ins->id);
		if (g_domain_existing_candidate &&
			cw_get_domain_instruction_id(g_edit_line, &g_edit_authorized_id)) {
			g_edit_pickup_authorized = true;
			g_edit_may_move = lv_is_arrange_enabled();
			g_edit_may_delete = lv_is_del_enabled();
		}
#ifdef STAGES_EDIT_TESTING
		g_edit_test_result = -1;
		g_edit_test_legacy_fallbacks = 0;
		trace_edit_context("pickup", program);
#endif
	} else if (cw_chk_rclick_code() == true && g_edit_line == NULL) {
		g_edit_line = cw_clone_rclicked_line(cw_get_rclicked_code());
		g_edit_hold_line = true;
	} else if ((left_pressed == true || g_edit_hold_line == true) &&
		   g_edit_line != NULL) {
		if ((g_domain_append_candidate || g_domain_existing_candidate) &&
			!cw_check_if_in_code_list(g_edit_line)) {
			cw_draw_held_instruction(g_edit_line);
		} else if (g_domain_existing_candidate) {
			cw_draw_held_instruction(g_edit_line);
		} else {
			bool arrange = lv_is_arrange_enabled();
			bool delete  = lv_is_del_enabled();
			if (!cw_player_holding_instruction(g_edit_line, arrange, delete)) {
				g_control_flow_repair_failed = true;
				g_program_snapshot_valid = false;
			}
		}
		g_edit_hold_line = (left_released == true) ? false : true;
	} else if (left_pressed == false && g_edit_line != NULL) {
		bool domain_append_committed = false;
		bool domain_append_failed = false;
		bool domain_existing_committed = false;
		bool domain_existing_failed = false;
		if (g_domain_existing_candidate &&
			cw_check_if_in_code_list(g_edit_line)) {
#ifdef STAGES_EDIT_TESTING
			trace_edit_context("release", program);
#endif
			cw_existing_edit_result_t edit_result =
				cw_edit_existing_line_authoritatively(
					g_edit_line, g_edit_may_move && lv_is_arrange_enabled(),
					g_edit_may_delete && lv_is_del_enabled(),
					domain_edit_context_allowed(program, g_edit_line),
					edit_domain_instruction, program);
#ifdef STAGES_EDIT_TESTING
			g_edit_test_result = edit_result;
			fprintf(stderr, "drag authoritative result=%d revision=%llu\n",
				edit_result, (unsigned long long)aa_program_revision(program));
#endif
			if (edit_result == CW_EXISTING_EDIT_COMMITTED) {
				domain_existing_committed = true;
				if (!cw_check_if_in_code_list(g_edit_line)) {
					cl_destroy_code_line(g_edit_line);
					g_edit_line = NULL;
					lv_set_hold_line(NULL);
				}
				save_and_update_code(level_id);
			} else if (edit_result == CW_EXISTING_EDIT_FAILED) {
				log_err("Domain-authoritative code edit failed");
				domain_existing_failed = true;
			} else if (edit_result == CW_EXISTING_EDIT_NOT_APPLICABLE &&
				g_edit_may_delete && lv_is_del_enabled() &&
				domain_edit_context_allowed(program, g_edit_line) &&
				(cl_is_ins_jmp_type(g_edit_line->ins->id) ||
				 g_edit_line->ins->id == LABEL)) {
#ifdef STAGES_EDIT_TESTING
				g_edit_test_legacy_fallbacks++;
#endif
				if (!cw_player_holding_instruction(
						g_edit_line, lv_is_arrange_enabled(), lv_is_del_enabled())) {
					g_control_flow_repair_failed = true;
					g_program_snapshot_valid = false;
				}
			} else {
				domain_existing_failed = true;
			}
		}
		if (g_edit_line != NULL && g_domain_append_candidate &&
			!cw_check_if_in_code_list(g_edit_line)) {
			cw_append_result_t append_result =
				cw_append_new_line_authoritatively(
					g_edit_line, lv_is_arrange_enabled(), lv_is_del_enabled(),
					domain_edit_context_allowed(program, g_edit_line),
					append_domain_instruction, prepare_domain_append, program);
			if (append_result == CW_APPEND_COMMITTED) {
				domain_append_committed = true;
				save_and_update_code(level_id);
			} else if (append_result == CW_APPEND_FAILED) {
				log_err("Domain-authoritative instruction append failed");
				domain_append_failed = true;
			} else {
				if (!cw_player_holding_instruction(
						g_edit_line, lv_is_arrange_enabled(), lv_is_del_enabled())) {
					g_control_flow_repair_failed = true;
					g_program_snapshot_valid = false;
				}
			}
		}
		if (g_edit_line != NULL && !g_domain_append_candidate &&
			!g_domain_existing_candidate &&
			!cw_check_if_in_code_list(g_edit_line)) {
			if (!cw_player_holding_instruction(
					g_edit_line, lv_is_arrange_enabled(), lv_is_del_enabled())) {
				g_control_flow_repair_failed = true;
				g_program_snapshot_valid = false;
			}
		}
		if (g_edit_line != NULL && !cw_check_if_in_code_list(g_edit_line)) {
			cl_destroy_code_line(g_edit_line);
		}
		cw_clear_held_instruction();
		if (!g_control_flow_repair_failed &&
			!domain_append_committed && !domain_append_failed &&
			!domain_existing_committed && !domain_existing_failed &&
			cw_is_operand_pending() == false) {
			code_updated_actions(level_id, program);
		} else if (!g_control_flow_repair_failed &&
			   !domain_append_committed && !domain_append_failed &&
			   !domain_existing_committed && !domain_existing_failed) {
			refresh_program_snapshot(program);
		} else if (g_control_flow_repair_failed) {
			g_program_snapshot_valid = false;
		}
		g_edit_line = NULL;
		g_edit_hold_line = false;
		g_domain_append_candidate = false;
		g_domain_existing_candidate = false;
		g_edit_authorized_id = AA_INSTRUCTION_ID_INVALID;
		g_edit_pickup_authorized = false;
		g_edit_may_move = false;
		g_edit_may_delete = false;
	}
	return g_edit_line;
}

/* Function: rst_btn_hdl
 * ----------------------------------------------------------------------------
 * Comprirses all the functions related to the rst button
 *
 * Arguments:
 * 	level_id: the level number that is going to be reset.
 *	flags: the flags of the level that will be reset.
 *
 * Return:
 *	void.
 */
static void rst_btn_hdl(int level_id, level_flags_t *flags,
					aa_program_t *program)
{
	bool menu_was_active = rm_chk_rst_menu_state();

	bool reset_button_released = sb_chk_rel_rst_btn();

	if (reset_button_released) {
		rm_set_rst_menu(true);
	}

	bool reset_confirmed = rm_chk_rst_menu_btns(rm_chk_rst_menu_state());

	bool menu_is_active = rm_chk_rst_menu_state();

	bool menu_closed = menu_was_active && !menu_is_active;

	if (reset_confirmed) {
		reset_level(level_id, flags);

		cw_clear_code_list();
		lv_init_stage_code(level_id);
		code_updated_actions(level_id, program);
		lv_init_level_assets(level_id);
		tr_load_level(level_id);
	}

	if (reset_button_released || menu_closed) {
		ms_reset_mouse_values();
	}
}

/* Function: reset_level
 * ----------------------------------------------------------------------------
 * Arguments:
 * 	level_id: the level number that is going to be reset.
 *	flags: the flags of the level that will be reset.
 *
 * Return:
 *	void.
 */
static void reset_level(int level_id, level_flags_t *flags)
{
	cancel_edit_interaction();
	oh_clear();
	mc_reset_avatar();
	reset_level_flags(flags);
	rg_reset_register_values();
	bf_reset_input_list();
	bf_reset_output_list();
	bf_reset_win_condition();
	wc_reset_expected_output();
	wc_reset_condition();
	cw_reset_code_execution();
	ar_hide_execution_arrow();
	mc_reset_invalid_operation_flag();
	mc_set_run_ended(false);
	ms_reset_mouse_values();
	rg_reset_ibox();
	rg_reset_obox();
	rg_reset_rflags();
	rr_reset_state();
}

/* Function: get_sector_id
 * ----------------------------------------------------------------------------
 * Returns the sector select id based on which level the player is in
 *
 * Arguments:
 * 	level_id: the level that the player is playing
 *
 * Return:
 *	void.
 */
static int get_sector_id(int level_id)
{
	int ret_screen = LV_SELECT_SECTOR;
	if (level_id < LV_SECTOR_1_START) {
		ret_screen = LV_SECTOR_0;
	} else if (level_id < LV_SECTOR_2_START && level_id >= LV_SECTOR_1_START) {
		ret_screen = LV_SECTOR_1;
	} else if (level_id < LV_SECTOR_2_START && level_id >= LV_SECTOR_1_START) {
		ret_screen = LV_SECTOR_1;
	} else if (level_id < LV_SECTOR_4_START && level_id >= LV_SECTOR_3_START) {
		ret_screen = LV_SECTOR_3;
	} else if (level_id >= LV_SECTOR_4_START) {
		ret_screen = LV_SECTOR_4;
	}
	return ret_screen;
}

int stage_level(int level_id, aa_program_t *program)
{
	assert(program != NULL && "Program owner is NULL");
	// Electron animation
	int                   W = dm_get_screen_width();
	int                   H = dm_get_screen_height();
	static fx_electron_t *fx;
	static Uint64         last_type_ms;
	static Uint64         anim_prev_ms;
	Uint64                cur_time      = SDL_GetTicks64();
	static bool           electron_init = false;

	if (electron_init == false) {
		electron_init = true;
		last_type_ms  = cur_time;
		fx            = fx_electron_create(g_renderer, W, H, NULL);
	}

	float dt     = (cur_time - anim_prev_ms) / 1000.0f;
	anim_prev_ms = cur_time;
	fx_electron_update(fx, dt);
	fx_electron_render(fx, g_renderer);

	// Electron animation

	int                  ret_val   = LV_PLAY_LEVEL;
	static level_flags_t flags;
	bool                 back_to_level_selection = sb_chck_rel_ret_btn();

	lv_set_hold_line(g_stage_hold_line);

	if (ms_left_pressed() && sb_chk_hov_rst_ret_btns()) {
    	ms_consume_left_press();
	}

	rst_btn_hdl(level_id, &flags, program);
	int operation_id = mc_get_operation_flag();
	run_result_action_t result_action = rr_update(operation_id);
	if (result_action == RUN_RESULT_ACTION_BACK) {
		reset_level(level_id, &flags);
		flags.play = false;
	} else if (operation_id == MC_WIN &&
	           result_action == RUN_RESULT_ACTION_CONTINUE) {
		rr_reset_state();
		back_to_level_selection = true;
	}
	operation_id = mc_get_operation_flag();
	lv_prepare_level_frame(level_id, operation_id);
	if (result_action != RUN_RESULT_ACTION_CONTINUE) {
		stage_drawings(level_id, operation_id);
	}
	cw_sort_code();

	if (sb_chk_click_stage_btn() == true && cw_is_operand_pending() == false) {
		flag_handler(&flags, identify_clicked_stage_button());
	}

	mc_start_execution(flags.play);
	if (flags.play || flags.step || back_to_level_selection ||
		rm_chk_rst_menu_state() || em_get_escape_state()) {
		cancel_edit_interaction();
	}

	if (flags.stop == true && flags.stop_enabled == true) {
		reset_level(level_id, &flags);
	} else if (flags.non_stop == false || cw_is_operand_pending() == true) {
			g_stage_hold_line = edit_code(level_id, program, flags.step,
									  back_to_level_selection);
		lv_set_hold_line(g_stage_hold_line);
	} else if (flags.play == true && cw_is_operand_pending() == false) {
		mc_run_code();
	} else if (flags.step == true && cw_is_operand_pending() == false) {
		mc_run_code();
		flags.step = !mc_get_step_ended();
	}
	int op_flag = mc_get_operation_flag();
	if (op_flag != NO_OPERATION && op_flag != MC_WIN) {
		flags.play = false;
	} else if (mc_get_run_ended() == true && flags.step_fst == true &&
	           wc_is_satisfied() == true) {
		mc_set_operation_flag(MC_WIN);
		bf_set_win_condition();
		flags.play          = false;
		fl_enable_next_level(g_player, level_id + 1);
	}

	if (back_to_level_selection == true) {
		rr_reset_state();
		ret_val = get_sector_id(level_id);
		reset_level(level_id, &flags);
		destroy_level(&flags);
		aa_program_clear(program);
		electron_init = false;
		aa_electron_fx_destroy(fx);
	}

	rm_render_rst_menu(rm_chk_rst_menu_state());
	em_render_escape_menu(em_get_escape_state());

	// sb_display_escape_menu(em_get_escape_state());
	return ret_val;
}
