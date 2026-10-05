#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "code_window_cw.h"
#include "code_window_cw_internal.h"
#include "dimensions_dm.h"
#include "draw_dw.h"
#include "domain/program.h"
#include "legacy_code_ids.h"
#include "migration/legacy_program_adapter.h"
#include "list.h"
#include "mouse_ms.h"
#include "sdl_config.h"
#include "stages.h"
#include "ui/button_bt.h"

static SDL_Surface *test_surface;
static SDL_Renderer *test_renderer;
static TTF_Font *test_font;

typedef struct reader_fixture {
	aa_legacy_line_snapshot_t line;
	unsigned char identity;
	bool present;
} reader_fixture_t;

typedef struct array_reader_fixture {
	aa_legacy_line_snapshot_t *lines;
	size_t count;
} array_reader_fixture_t;

static size_t reader_count(const void *context)
{
	const reader_fixture_t *fixture = context;
	return fixture->present ? 1 : 0;
}

static bool reader_line(const void *context, size_t position,
						 aa_legacy_line_snapshot_t *line)
{
	const reader_fixture_t *fixture = context;
	if (!fixture->present || position != 0 || line == NULL) {
		return false;
	}
	*line = fixture->line;
	return true;
}

static size_t array_reader_count(const void *context)
{
	const array_reader_fixture_t *fixture = context;
	return fixture->count;
}

static bool array_reader_line(const void *context, size_t position,
							  aa_legacy_line_snapshot_t *line)
{
	const array_reader_fixture_t *fixture = context;
	if (line == NULL || position >= fixture->count) {
		return false;
	}
	*line = fixture->lines[position];
	return true;
}

static aa_legacy_program_reader_t make_reader(const reader_fixture_t *fixture)
{
	aa_legacy_program_reader_t reader = {
		.count = reader_count,
		.read_line = reader_line,
		.context = fixture
	};
	return reader;
}

static bool append_from_legacy_snapshot(
	const aa_legacy_line_snapshot_t *snapshot,
	aa_instruction_id_t *created_id,
	void *context)
{
	aa_program_t *program = context;
	aa_instruction_t instruction;
	aa_legacy_import_report_t report;
	if (aa_legacy_instruction_from_snapshot(snapshot, &instruction, &report) !=
		AA_RESULT_OK) {
		return false;
	}
	if (aa_program_reserve(program, aa_program_count(program) + 1) !=
		AA_RESULT_OK) {
		return false;
	}
	return aa_program_append(program, &instruction, created_id) == AA_RESULT_OK;
}

static bool reject_domain_append(const aa_legacy_line_snapshot_t *snapshot,
								 aa_instruction_id_t *created_id,
								 void *context)
{
	(void)snapshot;
	(void)created_id;
	(void)context;
	return false;
}

static code_line_t *create_line(int opcode)
{
	texture_t *texture = cl_create_instruction_texture(opcode);
	assert(texture != NULL);
	SDL_Rect bounds = cl_get_code_button_size();
	btn_t *button = bt_create_btn(bounds, texture);
	assert(button != NULL);
	instruction_t *instruction = cl_create_instruction(opcode, button);
	assert(instruction != NULL);
	code_line_t *line = cl_create_code_line(instruction);
	assert(line != NULL);
	return line;
}

static operand_t *create_repair_test_operand(const char *text,
											 int id,
											 code_line_t *target,
											 bool label)
{
	char mutable_text[64];
	assert(strlen(text) < sizeof(mutable_text));
	strcpy(mutable_text, text);
	texture_t *texture = dw_create_text_tex(mutable_text, C_WHITE);
	assert(texture != NULL);
	SDL_Rect bounds = cl_get_code_button_size();
	if (!label) {
		bounds.w *= 2;
	}
	btn_t *button = bt_create_btn(bounds, texture);
	assert(button != NULL);
	operand_t *operand = malloc(sizeof(*operand));
	assert(operand != NULL);
	operand->b = button;
	operand->id = id;
	operand->jptr = target;
	return operand;
}

static code_line_t *create_label_line(const char *text, int id)
{
	code_line_t *line = create_line(LABEL);
	line->op1 = create_repair_test_operand(text, id, NULL, true);
	line->state = COMPLETE;
	return line;
}

static code_line_t *create_jump_line(int opcode,
									 const char *text,
									 int id,
									 code_line_t *target)
{
	code_line_t *line = create_line(opcode);
	if (target != NULL) {
		line->op1 = create_repair_test_operand(text, id, target, false);
		line->state = COMPLETE;
	} else {
		line->state = MISSING_OP1;
	}
	return line;
}

static void assert_repair_snapshot_unchanged(code_line_t *const *lines,
											 size_t count,
											 operand_t *const *operands,
											 const int *ids,
											 code_line_t *const *targets,
											 const int *states)
{
	for (size_t index = 0; index < count; index++) {
		assert(lines[index]->op1 == operands[index]);
		assert(lines[index]->state == states[index]);
		if (operands[index] != NULL) {
			assert(operands[index]->id == ids[index]);
			assert(operands[index]->jptr == targets[index]);
		}
	}
}

static void test_control_flow_repair_empty_and_ordinary(void)
{
	cw_control_flow_repair_plan_t *plan = NULL;
	assert(cw_prepare_control_flow_repair(NULL, 0, false, &plan) ==
		CW_REPAIR_OK);
	cw_commit_control_flow_repair(plan);
	cw_discard_control_flow_repair(plan);

	code_line_t *ordinary = create_line(MOV);
	code_line_t *order[] = {ordinary};
	assert(cw_prepare_control_flow_repair(order, 1, false, &plan) ==
		CW_REPAIR_OK);
	cw_commit_control_flow_repair(plan);
	cw_discard_control_flow_repair(plan);
	cl_destroy_code_line(ordinary);
}

static void test_control_flow_repair_positions_and_identity(void)
{
	code_line_t *jump_forward = create_jump_line(JMP, "line 00", 0, NULL);
	code_line_t *label_a = create_label_line("01:", 1);
	code_line_t *ordinary = create_line(MOV);
	code_line_t *label_b = create_label_line("02:", 2);
	code_line_t *jump_backward = create_jump_line(JE, "line 01", 1, label_a);
	code_line_t *jump_same_target = create_jump_line(JMP, "line 00", 0, NULL);
	code_line_t *jump_same_target_2 = create_jump_line(JNE, "line 00", 0, NULL);
	code_line_t *incomplete = create_jump_line(JMP, "", 0, NULL);
	jump_forward->op1 = create_repair_test_operand("line 03", 0, label_b, false);
	jump_forward->state = COMPLETE;
	jump_same_target->op1 = create_repair_test_operand("line 03", 0,
														 label_b, false);
	jump_same_target->state = COMPLETE;
	jump_same_target_2->op1 = create_repair_test_operand("line 03", 0,
														 label_b, false);
	jump_same_target_2->state = COMPLETE;
	code_line_t *order[] = {jump_forward, label_a, ordinary, label_b,
		jump_backward, jump_same_target, jump_same_target_2, incomplete};
	operand_t *old_operands[] = {
		jump_forward->op1, label_a->op1, NULL, label_b->op1,
		jump_backward->op1, jump_same_target->op1,
		jump_same_target_2->op1, NULL
	};
	int old_ids[] = {0, 1, 0, 2, 1, 0, 0, 0};
	code_line_t *old_targets[] = {
		label_b, NULL, NULL, NULL, label_a, label_b, label_b, NULL
	};
	int old_states[] = {
		COMPLETE, COMPLETE, MISSING_BOTH, COMPLETE,
		COMPLETE, COMPLETE, COMPLETE, MISSING_OP1
	};
	cw_control_flow_repair_plan_t *plan = NULL;
	assert(cw_prepare_control_flow_repair(order, 8, false, &plan) ==
		CW_REPAIR_OK);
	assert_repair_snapshot_unchanged(order, 8, old_operands, old_ids,
										 old_targets, old_states);
	cw_commit_control_flow_repair(plan);
	operand_t *committed_label_operand = label_a->op1;
	cw_commit_control_flow_repair(plan);
	assert(label_a->op1 == committed_label_operand);
	cw_discard_control_flow_repair(plan);

	assert(label_a->op1->id == 2);
	assert(label_b->op1->id == 3);
	assert(jump_forward->op1->id == 3);
	assert(jump_backward->op1->id == 1);
	assert(jump_same_target->op1->id == 3);
	assert(jump_same_target_2->op1->id == 3);
	assert(jump_forward->op1->jptr == label_b);
	assert(jump_backward->op1->jptr == label_a);
	assert(jump_same_target->op1->jptr == label_b);
	assert(jump_same_target_2->op1->jptr == label_b);
	assert(incomplete->op1 == NULL && incomplete->state == MISSING_OP1);
	assert(order[1] == label_a && order[3] == label_b);

	for (int iteration = 0; iteration < 5; iteration++) {
		assert(cw_prepare_control_flow_repair(order, 8, false, &plan) ==
			CW_REPAIR_OK);
		cw_commit_control_flow_repair(plan);
		cw_discard_control_flow_repair(plan);
	}
	cl_destroy_code_line(jump_forward);
	cl_destroy_code_line(label_a);
	cl_destroy_code_line(ordinary);
	cl_destroy_code_line(label_b);
	cl_destroy_code_line(jump_backward);
	cl_destroy_code_line(jump_same_target);
	cl_destroy_code_line(jump_same_target_2);
	cl_destroy_code_line(incomplete);
}

static void test_control_flow_repair_consecutive_labels(void)
{
	code_line_t *label_a = create_label_line("00:", 0);
	code_line_t *label_b = create_label_line("00:", 0);
	code_line_t *jump = create_jump_line(JMP, "line 00", 0, label_b);
	code_line_t *order[] = {label_a, label_b, jump};
	cw_control_flow_repair_plan_t *plan = NULL;
	assert(cw_prepare_control_flow_repair(order, 3, false, &plan) ==
		CW_REPAIR_OK);
	cw_commit_control_flow_repair(plan);
	cw_discard_control_flow_repair(plan);
	assert(label_a->op1->id == 1);
	assert(label_b->op1->id == 1);
	assert(jump->op1->id == 1);
	assert(jump->op1->jptr == label_b);
	cl_destroy_code_line(label_a);
	cl_destroy_code_line(label_b);
	cl_destroy_code_line(jump);
}

static void test_proposed_order_repair(void)
{
	code_line_t *jump = create_jump_line(JMP, "line 00", 0, NULL);
	code_line_t *label_a = create_label_line("00:", 0);
	code_line_t *ordinary = create_line(MOV);
	code_line_t *label_b = create_label_line("00:", 0);
	code_line_t *target_jump = create_jump_line(JNE, "line 00", 0, label_b);
	jump->op1 = create_repair_test_operand("line 00", 0, label_b, false);
	jump->state = COMPLETE;
	cw_control_flow_repair_plan_t *plan = NULL;

	code_line_t *appended = create_line(CMP);
	code_line_t *append_order[] = {jump, label_a, ordinary, label_b,
		target_jump, appended};
	operand_t *label_a_before = label_a->op1;
	operand_t *label_b_before = label_b->op1;
	operand_t *jump_before = jump->op1;
	assert(cw_prepare_control_flow_repair(append_order, 6, false, &plan) ==
		CW_REPAIR_OK);
	assert(label_a->op1 == label_a_before && label_b->op1 == label_b_before);
	assert(jump->op1 == jump_before);
	cw_commit_control_flow_repair(plan);
	cw_discard_control_flow_repair(plan);
	assert(label_a->op1->id == 2 && label_b->op1->id == 3);
	assert(jump->op1->id == 3 && jump->op1->jptr == label_b);

	code_line_t *move_order[] = {jump, label_a, label_b, ordinary, target_jump};
	assert(cw_prepare_control_flow_repair(move_order, 5, false, &plan) ==
		CW_REPAIR_OK);
	cw_commit_control_flow_repair(plan);
	cw_discard_control_flow_repair(plan);
	assert(label_b->op1->id == 2 && jump->op1->id == 2);
	assert(jump->op1->jptr == label_b);

	code_line_t *remove_order[] = {jump, label_a, label_b, target_jump};
	assert(cw_prepare_control_flow_repair(remove_order, 4, false, &plan) ==
		CW_REPAIR_OK);
	cw_commit_control_flow_repair(plan);
	cw_discard_control_flow_repair(plan);
	assert(label_b->op1->id == 2 && jump->op1->id == 2);
	assert(jump->op1->jptr == label_b);

	cl_destroy_code_line(jump);
	cl_destroy_code_line(label_a);
	cl_destroy_code_line(ordinary);
	cl_destroy_code_line(label_b);
	cl_destroy_code_line(target_jump);
	cl_destroy_code_line(appended);
}

static void test_control_flow_repair_rejects_invalid_targets(void)
{
	code_line_t *label = create_label_line("00:", 0);
	code_line_t *other = create_line(MOV);
	code_line_t *jump = create_jump_line(JMP, "line 00", 0, label);
	code_line_t *without_target[] = {jump};
	operand_t *old_operand = jump->op1;
	cw_control_flow_repair_plan_t *plan = NULL;
	assert(cw_prepare_control_flow_repair(without_target, 1, false, &plan) ==
		CW_REPAIR_INVALID_TARGET);
	assert(plan == NULL && jump->op1 == old_operand && jump->op1->jptr == label);

	code_line_t *non_label_target[] = {other, jump};
	jump->op1->jptr = other;
	assert(cw_prepare_control_flow_repair(non_label_target, 2, false, &plan) ==
		CW_REPAIR_TARGET_NOT_LABEL);
	assert(plan == NULL && jump->op1 == old_operand && jump->op1->jptr == other);
	jump->op1->jptr = label;

	code_line_t *saved_jump = create_jump_line(JMP, "01", 1, NULL);
	saved_jump->op1 = create_repair_test_operand("01", 1, NULL, false);
	saved_jump->state = COMPLETE;
	code_line_t *null_saved_target[] = {saved_jump, NULL};
	operand_t *saved_operand = saved_jump->op1;
	assert(cw_prepare_control_flow_repair(null_saved_target, 2, true, &plan) ==
		CW_REPAIR_INVALID_TARGET);
	assert(plan == NULL && saved_jump->op1 == saved_operand &&
		saved_jump->op1->jptr == NULL && saved_jump->op1->id == 1);
	cl_destroy_code_line(saved_jump);
	cl_destroy_code_line(label);
	cl_destroy_code_line(other);
	cl_destroy_code_line(jump);
}

static void test_saved_jump_reconstruction_is_atomic(void)
{
	cw_create_code_list();
	char saved_label_a[] = "line 00";
	char saved_label_b[] = "line 00";
	char saved_forward_jump[] = "JMP 01";
	cw_add_saved_line(saved_label_a);
	cw_add_saved_line(saved_label_b);
	cw_add_saved_line(saved_forward_jump);
	code_line_t *label_a = cw_get_code_line_at_pos(0);
	code_line_t *label_b = cw_get_code_line_at_pos(1);
	code_line_t *saved_jump = cw_get_code_line_at_pos(2);
	operand_t *saved_placeholder = saved_jump->op1;
	assert(cw_update_saved_jump_instructions());
	assert(saved_jump->op1 != saved_placeholder);
	assert(label_a->op1->id == 1 && label_b->op1->id == 1);
	assert(saved_jump->op1->id == 1);
	assert(saved_jump->op1->jptr == label_b);
	cw_destroy_code_window_assets();

	cw_create_code_list();
	char invalid_label[] = "line 00";
	char invalid_jump[] = "JMP 01";
	cw_add_saved_line(invalid_label);
	cw_add_saved_line(invalid_jump);
	label_a = cw_get_code_line_at_pos(0);
	saved_jump = cw_get_code_line_at_pos(1);
	operand_t *invalid_placeholder = saved_jump->op1;
	operand_t *old_label_operand = label_a->op1;
	assert(!cw_update_saved_jump_instructions());
	assert(saved_jump->op1 == invalid_placeholder);
	assert(saved_jump->op1->jptr == NULL && saved_jump->op1->id == 1);
	assert(label_a->op1 == old_label_operand && label_a->op1->id == 0);
	cw_destroy_code_window_assets();
}

static void test_live_repair_rejects_malformed_incomplete_jump(void)
{
	cw_create_code_list();
	char saved_jump[] = "JMP 00";
	cw_add_saved_line(saved_jump);
	code_line_t *jump = cw_get_code_line_at_pos(0);
	cl_destroy_operand(jump->op1);
	jump->op1 = NULL;
	jump->state = COMPLETE;
	assert(!cw_refresh_label_and_jump_presentation());
	assert(jump->op1 == NULL && jump->state == COMPLETE);

	jump->state = MISSING_OP1;
	assert(cw_refresh_label_and_jump_presentation());
	assert(jump->op1 == NULL && jump->state == MISSING_OP1);
	cw_destroy_code_window_assets();
}

static void test_control_flow_repair_failure_atomicity(void)
{
	code_line_t *label_a = create_label_line("00:", 0);
	code_line_t *jump_a = create_jump_line(JMP, "line 00", 0, label_a);
	code_line_t *label_b = create_label_line("00:", 0);
	code_line_t *jump_b = create_jump_line(JE, "line 00", 0, label_b);
	code_line_t *jump_c = create_jump_line(JNE, "line 00", 0, label_a);
	code_line_t *order[] = {label_a, jump_a, label_b, jump_b, jump_c};
	operand_t *operands[] = {label_a->op1, jump_a->op1, label_b->op1,
		jump_b->op1, jump_c->op1};
	int ids[] = {0, 0, 0, 0, 0};
	code_line_t *targets[] = {NULL, label_a, NULL, label_b, label_a};
	int states[] = {COMPLETE, COMPLETE, COMPLETE, COMPLETE, COMPLETE};
	struct failure_case {
		cw_repair_test_stage_t stage;
		int opcode;
		size_t skip;
	} cases[] = {
		{CW_REPAIR_TEST_BEFORE_ENTRY, INVALID_INSTRUCTION, 0},
		{CW_REPAIR_TEST_BEFORE_ENTRY, INVALID_INSTRUCTION, 1},
		{CW_REPAIR_TEST_BEFORE_ENTRY, INVALID_INSTRUCTION, 3},
		{CW_REPAIR_TEST_TEXTURE, LABEL, 0},
		{CW_REPAIR_TEST_BUTTON, LABEL, 0},
		{CW_REPAIR_TEST_OPERAND, LABEL, 0},
		{CW_REPAIR_TEST_TEXTURE, JMP, 0},
		{CW_REPAIR_TEST_BUTTON, JMP, 0},
		{CW_REPAIR_TEST_OPERAND, JMP, 0}
	};
	for (size_t index = 0; index < sizeof(cases) / sizeof(cases[0]); index++) {
		cw_control_flow_repair_plan_t *plan = NULL;
		cw_repair_fail_for_test(cases[index].stage, cases[index].opcode,
							cases[index].skip);
		assert(cw_prepare_control_flow_repair(order, 5, false, &plan) !=
			CW_REPAIR_OK);
		assert(plan == NULL);
		assert_repair_snapshot_unchanged(order, 5, operands, ids, targets,
										 states);
	}
	for (int iteration = 0; iteration < 10; iteration++) {
		cw_control_flow_repair_plan_t *plan = NULL;
		assert(cw_prepare_control_flow_repair(order, 5, false, &plan) ==
			CW_REPAIR_OK);
		cw_discard_control_flow_repair(plan);
	}
	cl_destroy_code_line(label_a);
	cl_destroy_code_line(jump_a);
	cl_destroy_code_line(label_b);
	cl_destroy_code_line(jump_b);
	cl_destroy_code_line(jump_c);
}

static bool edit_domain_from_window(cw_existing_edit_kind_t kind,
									size_t from,
									size_t to,
									const aa_legacy_line_snapshot_t *snapshot,
									void *context)
{
	aa_program_t *program = context;
	if (program == NULL || snapshot == NULL ||
		from >= aa_program_count(program)) {
		return false;
	}
	aa_instruction_t converted;
	aa_legacy_import_report_t report;
	const aa_instruction_t *current = aa_program_instruction_at(program, from);
	if (current == NULL ||
		aa_legacy_instruction_from_snapshot(snapshot, &converted, &report) !=
		AA_RESULT_OK || current->opcode != converted.opcode) {
		return false;
	}
	if (kind == CW_EXISTING_EDIT_REMOVE) {
		return aa_program_remove(program, from) == AA_RESULT_OK;
	}
	if (kind == CW_EXISTING_EDIT_MOVE) {
		return aa_program_move(program, from, to) == AA_RESULT_OK;
	}
	return false;
}

static bool reject_existing_edit(cw_existing_edit_kind_t kind,
								size_t from,
									size_t to,
									const aa_legacy_line_snapshot_t *snapshot,
									void *context)
{
	(void)kind;
	(void)from;
	(void)to;
	(void)snapshot;
	(void)context;
	return false;
}

static void set_mouse_position(int x, int y)
{
	SDL_Event event = {0};
	event.motion.type = SDL_MOUSEMOTION;
	event.motion.x = x;
	event.motion.y = y;
	ms_mouse_motion_handler(event);
}

static code_line_t *create_complete_ordinary_line(int opcode)
{
	code_line_t *line = create_line(opcode);
	line->op1 = create_repair_test_operand("RAX", RAX, NULL, false);
	line->op2 = create_repair_test_operand("IB", IB, NULL, false);
	line->state = COMPLETE;
	return line;
}

static void assert_control_flow_transaction_state(
	aa_program_t *program,
	code_line_t *const *expected_order,
	size_t expected_count,
	code_line_t *const *tracked_lines,
	const aa_instruction_id_t *tracked_ids,
	size_t tracked_count,
	code_line_t *const *tracked_jumps,
	code_line_t *const *original_targets,
	size_t jump_count)
{
	assert(aa_program_count(program) == expected_count);
	assert((size_t)cw_get_code_list_size() == expected_count);
	assert(cw_domain_bindings_valid(program));
	for (size_t position = 0; position < expected_count; position++) {
		code_line_t *line = cw_get_code_line_at_pos((int)position);
		assert(line == expected_order[position]);
		const aa_instruction_t *instruction =
			aa_program_instruction_at(program, position);
		assert(instruction != NULL);
		bool tracked = false;
		for (size_t index = 0; index < tracked_count; index++) {
			if (tracked_lines[index] == line) {
				assert(instruction->id == tracked_ids[index]);
				tracked = true;
				break;
			}
		}
		assert(tracked);
		if (line->ins->id == LABEL && line->op1 != NULL) {
			size_t labels_through_position = 0;
			for (size_t label_position = 0; label_position <= position;
				 label_position++) {
				if (expected_order[label_position]->ins->id == LABEL) {
					labels_through_position++;
				}
			}
			int expected_label_number = position == 0 ? 1 :
				(int)(position - labels_through_position + 2);
			assert(line->op1->id == expected_label_number);
		}
		if (cl_is_ins_jmp_type(line->ins->id)) {
			if (line->op1 == NULL) {
				assert(line->state == MISSING_OP1);
				assert(instruction->operands[0].kind == AA_OPERAND_NONE);
			} else {
				size_t target_position = 0;
				while (target_position < expected_count &&
					expected_order[target_position] != line->op1->jptr) {
					target_position++;
				}
				assert(target_position < expected_count);
				assert(expected_order[target_position]->ins->id == LABEL);
				assert(line->op1->id == (int)target_position);
				assert(instruction->operands[0].kind ==
					AA_OPERAND_LABEL_REFERENCE);
			}
		}
	}
	for (size_t jump_index = 0; jump_index < jump_count; jump_index++) {
		code_line_t *jump = tracked_jumps[jump_index];
		bool found = false;
		for (size_t position = 0; position < expected_count; position++) {
			if (expected_order[position] == jump) {
				found = true;
				break;
			}
		}
		assert(found);
		if (original_targets[jump_index] == NULL) {
			assert(jump->op1 == NULL && jump->state == MISSING_OP1);
		} else {
			assert(jump->op1 != NULL && jump->state == COMPLETE);
			assert(jump->op1->jptr == original_targets[jump_index]);
			const aa_instruction_t *domain_jump = NULL;
			const aa_instruction_t *domain_target = NULL;
			for (size_t index = 0; index < tracked_count; index++) {
				if (tracked_lines[index] == jump) {
					domain_jump = aa_program_find_by_id(program, tracked_ids[index]);
				}
				if (tracked_lines[index] == original_targets[jump_index]) {
					domain_target = aa_program_find_by_id(program, tracked_ids[index]);
				}
			}
			assert(domain_jump != NULL && domain_target != NULL);
			assert(domain_jump->operands[0].kind == AA_OPERAND_LABEL_REFERENCE);
			assert(domain_jump->operands[0].value.label_instruction_id ==
				domain_target->id);
		}
	}
}

static cw_existing_edit_result_t move_ordinary_line_to(
	code_line_t *line,
	size_t position,
	aa_program_t *program)
{
	set_mouse_position(100, cw_get_code_line_y(0) +
		(int)position * cw_get_code_line_spacing());
	return cw_edit_existing_line_authoritatively(
		line, true, false, true, edit_domain_from_window, program);
}

static void test_domain_authoritative_control_flow_transactions(void)
{
	cw_set_challenge_text("control flow transaction test");
	cw_create_code_list();
	code_line_t *forward = create_jump_line(JMP, "", 0, NULL);
	code_line_t *mov = create_complete_ordinary_line(MOV);
	code_line_t *label_a = create_label_line("00:", 0);
	code_line_t *add = create_complete_ordinary_line(ADD);
	code_line_t *backward = create_jump_line(JE, "", 0, NULL);
	code_line_t *label_b = create_label_line("00:", 0);
	code_line_t *cmp = create_complete_ordinary_line(CMP);
	code_line_t *jne_a = create_jump_line(JNE, "", 0, NULL);
	code_line_t *jne_b = create_jump_line(JMP, "", 0, NULL);
	code_line_t *incomplete = create_jump_line(JMP, "", 0, NULL);
	code_line_t *label_c = create_label_line("00:", 0);
	code_line_t *label_d = create_label_line("00:", 0);
	code_line_t *initial_order[] = {forward, mov, label_a, add, backward,
		label_b, cmp, jne_a, jne_b, incomplete, label_c, label_d};
	for (size_t position = 0;
		 position < sizeof(initial_order) / sizeof(initial_order[0]); position++) {
		assert(cw_player_holding_instruction(initial_order[position], false, false));
		cw_clear_held_instruction();
	}
	forward->op1 = create_repair_test_operand("line 00", 0, label_b, false);
	forward->state = COMPLETE;
	backward->op1 = create_repair_test_operand("line 00", 0, label_a, false);
	backward->state = COMPLETE;
	jne_a->op1 = create_repair_test_operand("line 00", 0, label_d, false);
	jne_a->state = COMPLETE;
	jne_b->op1 = create_repair_test_operand("line 00", 0, label_d, false);
	jne_b->state = COMPLETE;
	assert(cw_refresh_label_and_jump_presentation());

	aa_program_t *program = aa_program_create();
	assert(program != NULL);
	aa_legacy_program_reader_t reader;
	aa_legacy_import_report_t report;
	assert(cw_get_legacy_program_reader(&reader));
	assert(aa_legacy_program_import(program, &reader, &report) == AA_RESULT_OK);
	assert(cw_rebuild_domain_bindings(program));
	code_line_t *tracked_lines[20];
	aa_instruction_id_t tracked_ids[20];
	size_t tracked_count = sizeof(initial_order) / sizeof(initial_order[0]);
	for (size_t index = 0; index < tracked_count; index++) {
		tracked_lines[index] = initial_order[index];
		tracked_ids[index] = aa_program_instruction_at(program, index)->id;
	}
	code_line_t *tracked_jumps[] = {forward, backward, jne_a, jne_b, incomplete};
	code_line_t *original_targets[] = {label_b, label_a, label_d, label_d, NULL};
	code_line_t *expected_order[20];
	memcpy(expected_order, initial_order, sizeof(initial_order));
	size_t expected_count = tracked_count;
	assert_control_flow_transaction_state(program, expected_order,
		expected_count, tracked_lines, tracked_ids, tracked_count,
		tracked_jumps, original_targets,
		sizeof(tracked_jumps) / sizeof(tracked_jumps[0]));

	code_line_t *candidate = create_line(MOV);
	candidate->state = MISSING_BOTH;
	uint64_t revision = aa_program_revision(program);
	assert(cw_append_new_line_authoritatively(candidate, false, false, false,
		append_from_legacy_snapshot, program) == CW_APPEND_FAILED);
	assert(aa_program_revision(program) == revision);
	cw_repair_fail_for_test(CW_REPAIR_TEST_TEXTURE, LABEL, 0);
	assert(cw_append_new_line_authoritatively(candidate, false, false, true,
		append_from_legacy_snapshot, program) == CW_APPEND_FAILED);
	assert(aa_program_revision(program) == revision);
	assert(cw_append_new_line_authoritatively(candidate, false, false, true,
		reject_domain_append, program) == CW_APPEND_FAILED);
	assert(aa_program_revision(program) == revision);
	assert_control_flow_transaction_state(program, expected_order,
		expected_count, tracked_lines, tracked_ids, tracked_count,
		tracked_jumps, original_targets,
		sizeof(tracked_jumps) / sizeof(tracked_jumps[0]));

	const int append_opcodes[] = {MOV, ADD, CMP};
	for (size_t index = 0; index < sizeof(append_opcodes) / sizeof(append_opcodes[0]);
		 index++) {
		candidate = index == 0 ? candidate :
			create_complete_ordinary_line(append_opcodes[index]);
		candidate->ins->id = append_opcodes[index];
		revision = aa_program_revision(program);
		assert(cw_append_new_line_authoritatively(candidate, false, false, true,
			append_from_legacy_snapshot, program) == CW_APPEND_COMMITTED);
		assert(aa_program_revision(program) == revision + 1);
		tracked_lines[tracked_count] = candidate;
		tracked_ids[tracked_count] =
			aa_program_instruction_at(program, expected_count)->id;
		expected_order[expected_count++] = candidate;
		tracked_count++;
		assert_control_flow_transaction_state(program, expected_order,
			expected_count, tracked_lines, tracked_ids, tracked_count,
			tracked_jumps, original_targets,
			sizeof(tracked_jumps) / sizeof(tracked_jumps[0]));
	}

	code_line_t *rejected_lines[] = {
		create_label_line("00:", 0),
		create_jump_line(JMP, "", 0, NULL),
		create_jump_line(JE, "", 0, NULL),
		create_jump_line(JNE, "", 0, NULL)
	};
	for (size_t index = 0; index < sizeof(rejected_lines) / sizeof(rejected_lines[0]);
		 index++) {
		assert(cw_append_new_line_authoritatively(rejected_lines[index], false,
			false, true, append_from_legacy_snapshot, program) ==
			CW_APPEND_NOT_APPLICABLE);
		assert(cw_edit_existing_line_authoritatively(rejected_lines[index], true,
			false, true, edit_domain_from_window, program) ==
			CW_EXISTING_EDIT_NOT_APPLICABLE);
		cl_destroy_code_line(rejected_lines[index]);
	}
	code_line_t *live_control_flow[] = {label_a, forward, backward, jne_a};
	for (size_t index = 0;
		 index < sizeof(live_control_flow) / sizeof(live_control_flow[0]); index++) {
		assert(cw_edit_existing_line_authoritatively(live_control_flow[index],
			true, false, true, edit_domain_from_window, program) ==
			CW_EXISTING_EDIT_NOT_APPLICABLE);
	}
	assert(cw_edit_existing_line_authoritatively(mov, true, false, false,
		edit_domain_from_window, program) == CW_EXISTING_EDIT_FAILED);

	SDL_Rect scroll = {.x = 1, .y = 1, .w = 1000, .h = 1000};
	cw_set_code_box(scroll);
	cw_set_scroll_box(scroll);
	operand_t *old_label_operand = label_a->op1;
	operand_t *old_forward_operand = forward->op1;
	revision = aa_program_revision(program);
	cw_repair_fail_for_test(CW_REPAIR_TEST_TEXTURE, LABEL, 0);
	assert(move_ordinary_line_to(mov, 3, program) == CW_EXISTING_EDIT_FAILED);
	assert(aa_program_revision(program) == revision);
	assert(label_a->op1 == old_label_operand && forward->op1 == old_forward_operand);
	assert_control_flow_transaction_state(program, expected_order,
		expected_count, tracked_lines, tracked_ids, tracked_count,
		tracked_jumps, original_targets,
		sizeof(tracked_jumps) / sizeof(tracked_jumps[0]));
	set_mouse_position(0, 0);
	revision = aa_program_revision(program);
	cw_repair_fail_for_test(CW_REPAIR_TEST_TEXTURE, LABEL, 0);
	assert(cw_edit_existing_line_authoritatively(add, false, true, true,
		edit_domain_from_window, program) == CW_EXISTING_EDIT_FAILED);
	assert(aa_program_revision(program) == revision);
	assert(label_a->op1 == old_label_operand && forward->op1 == old_forward_operand);
	assert_control_flow_transaction_state(program, expected_order,
		expected_count, tracked_lines, tracked_ids, tracked_count,
		tracked_jumps, original_targets,
		sizeof(tracked_jumps) / sizeof(tracked_jumps[0]));

	code_line_t *moving = mov;
	size_t from = 1;
	size_t to = 2;
	revision = aa_program_revision(program);
	assert(move_ordinary_line_to(moving, to, program) ==
		CW_EXISTING_EDIT_COMMITTED);
	assert(aa_program_revision(program) == revision + 1);
	memmove(&expected_order[from], &expected_order[from + 1],
		(to - from) * sizeof(*expected_order));
	expected_order[to] = moving;
	assert_control_flow_transaction_state(program, expected_order,
		expected_count, tracked_lines, tracked_ids, tracked_count,
		tracked_jumps, original_targets,
		sizeof(tracked_jumps) / sizeof(tracked_jumps[0]));

	moving = add;
	from = 3;
	to = 1;
	revision = aa_program_revision(program);
	assert(move_ordinary_line_to(moving, to, program) ==
		CW_EXISTING_EDIT_COMMITTED);
	assert(aa_program_revision(program) == revision + 1);
	memmove(&expected_order[to + 1], &expected_order[to],
		(from - to) * sizeof(*expected_order));
	expected_order[to] = moving;
	assert_control_flow_transaction_state(program, expected_order,
		expected_count, tracked_lines, tracked_ids, tracked_count,
		tracked_jumps, original_targets,
		sizeof(tracked_jumps) / sizeof(tracked_jumps[0]));

	moving = cmp;
	from = 6;
	to = 0;
	revision = aa_program_revision(program);
	assert(move_ordinary_line_to(moving, to, program) ==
		CW_EXISTING_EDIT_COMMITTED);
	assert(aa_program_revision(program) == revision + 1);
	memmove(&expected_order[to + 1], &expected_order[to],
		(from - to) * sizeof(*expected_order));
	expected_order[to] = moving;
	assert_control_flow_transaction_state(program, expected_order,
		expected_count, tracked_lines, tracked_ids, tracked_count,
		tracked_jumps, original_targets,
		sizeof(tracked_jumps) / sizeof(tracked_jumps[0]));

	moving = tracked_lines[tracked_count - 3];
	from = 12;
	to = 2;
	revision = aa_program_revision(program);
	assert(move_ordinary_line_to(moving, to, program) ==
		CW_EXISTING_EDIT_COMMITTED);
	assert(aa_program_revision(program) == revision + 1);
	memmove(&expected_order[to + 1], &expected_order[to],
		(from - to) * sizeof(*expected_order));
	expected_order[to] = moving;
	assert_control_flow_transaction_state(program, expected_order,
		expected_count, tracked_lines, tracked_ids, tracked_count,
		tracked_jumps, original_targets,
		sizeof(tracked_jumps) / sizeof(tracked_jumps[0]));

	moving = tracked_lines[tracked_count - 2];
	from = 13;
	to = 7;
	revision = aa_program_revision(program);
	assert(move_ordinary_line_to(moving, to, program) ==
		CW_EXISTING_EDIT_COMMITTED);
	assert(aa_program_revision(program) == revision + 1);
	memmove(&expected_order[to + 1], &expected_order[to],
		(from - to) * sizeof(*expected_order));
	expected_order[to] = moving;
	assert_control_flow_transaction_state(program, expected_order,
		expected_count, tracked_lines, tracked_ids, tracked_count,
		tracked_jumps, original_targets,
		sizeof(tracked_jumps) / sizeof(tracked_jumps[0]));

	moving = tracked_lines[tracked_count - 1];
	from = expected_count - 1;
	to = 0;
	revision = aa_program_revision(program);
	assert(move_ordinary_line_to(moving, to, program) ==
		CW_EXISTING_EDIT_COMMITTED);
	assert(aa_program_revision(program) == revision + 1);
	memmove(&expected_order[to + 1], &expected_order[to],
		from * sizeof(*expected_order));
	expected_order[to] = moving;
	assert_control_flow_transaction_state(program, expected_order,
		expected_count, tracked_lines, tracked_ids, tracked_count,
		tracked_jumps, original_targets,
		sizeof(tracked_jumps) / sizeof(tracked_jumps[0]));

	from = 0;
	to = expected_count - 1;
	revision = aa_program_revision(program);
	assert(move_ordinary_line_to(moving, to, program) ==
		CW_EXISTING_EDIT_COMMITTED);
	assert(aa_program_revision(program) == revision + 1);
	memmove(&expected_order[from], &expected_order[from + 1],
		(to - from) * sizeof(*expected_order));
	expected_order[to] = moving;
	assert_control_flow_transaction_state(program, expected_order,
		expected_count, tracked_lines, tracked_ids, tracked_count,
		tracked_jumps, original_targets,
		sizeof(tracked_jumps) / sizeof(tracked_jumps[0]));

	moving = mov;
	from = 5;
	to = 4;
	revision = aa_program_revision(program);
	assert(move_ordinary_line_to(moving, to, program) ==
		CW_EXISTING_EDIT_COMMITTED);
	assert(aa_program_revision(program) == revision + 1);
	memmove(&expected_order[to + 1], &expected_order[to],
		(from - to) * sizeof(*expected_order));
	expected_order[to] = moving;
	assert_control_flow_transaction_state(program, expected_order,
		expected_count, tracked_lines, tracked_ids, tracked_count,
		tracked_jumps, original_targets,
		sizeof(tracked_jumps) / sizeof(tracked_jumps[0]));

	code_line_t *remove_lines[] = {mov, add, tracked_lines[tracked_count - 1]};
	for (size_t removal = 0; removal < sizeof(remove_lines) / sizeof(remove_lines[0]);
		 removal++) {
		code_line_t *removed = remove_lines[removal];
		from = 0;
		while (from < expected_count && expected_order[from] != removed) {
			from++;
		}
		assert(from < expected_count);
		aa_instruction_id_t removed_id = AA_INSTRUCTION_ID_INVALID;
		for (size_t index = 0; index < tracked_count; index++) {
			if (tracked_lines[index] == removed) {
				removed_id = tracked_ids[index];
				break;
			}
		}
		assert(removed_id != AA_INSTRUCTION_ID_INVALID);
		set_mouse_position(0, 0);
		revision = aa_program_revision(program);
		assert(cw_edit_existing_line_authoritatively(removed, false, true, true,
			edit_domain_from_window, program) == CW_EXISTING_EDIT_COMMITTED);
		assert(aa_program_revision(program) == revision + 1);
		assert(aa_program_find_by_id(program, removed_id) == NULL);
		memmove(&expected_order[from], &expected_order[from + 1],
			(expected_count - from - 1) * sizeof(*expected_order));
		expected_count--;
		cl_destroy_code_line(removed);
		assert_control_flow_transaction_state(program, expected_order,
			expected_count, tracked_lines, tracked_ids, tracked_count,
			tracked_jumps, original_targets,
			sizeof(tracked_jumps) / sizeof(tracked_jumps[0]));
	}

	cw_destroy_code_window_assets();
	aa_program_destroy(program);
}

static void init_test_graphics(void)
{
	assert(SDL_Init(0) == 0);
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
	test_font = TTF_OpenFont("fonts/DOSVGA437.ttf", 130);
	assert(test_font != NULL);
	g_font = test_font;
	ms_init_mouse();
	cw_init_code_window_texture();
}

static void test_code_window_transactions(void)
{
	aa_program_t *program = aa_program_create();
	assert(program != NULL);
	cw_set_challenge_text("test challenge");
	cw_create_code_list();
	assert(cw_rebuild_domain_bindings(program));
	code_line_t *first = create_line(MOV);
	uint64_t revision = aa_program_revision(program);
	assert(cw_append_new_line_authoritatively(
		first, false, false, true, reject_domain_append, program) == CW_APPEND_FAILED);
	assert(aa_program_count(program) == 0);
	assert(aa_program_revision(program) == revision);
	assert(cw_get_code_list_size() == 0);
	assert(cw_append_new_line_authoritatively(
		first, false, false, true, append_from_legacy_snapshot, program) ==
		CW_APPEND_COMMITTED);
	assert(aa_program_count(program) == 1);
	assert(aa_program_revision(program) == revision + 1);
	assert(cw_get_code_list_size() == 1);
	assert(cw_get_code_line_at_pos(0) == first);
	assert(cw_domain_bindings_valid(program));
	aa_instruction_id_t first_id = aa_program_instruction_at(program, 0)->id;

	code_line_t *second = create_line(ADD);
	revision = aa_program_revision(program);
	assert(cw_append_new_line_authoritatively(
		second, false, false, true, append_from_legacy_snapshot, program) ==
		CW_APPEND_COMMITTED);
	assert(aa_program_count(program) == 2);
	assert(aa_program_revision(program) == revision + 1);
	assert(cw_get_code_line_at_pos(1) == second);
	assert(cw_domain_bindings_valid(program));
	aa_instruction_id_t second_id = aa_program_instruction_at(program, 1)->id;
	aa_program_t *mismatched_program = aa_program_create();
	assert(mismatched_program != NULL);
	assert(!cw_rebuild_domain_bindings(mismatched_program));
	assert(cw_domain_bindings_valid(program));
	aa_program_destroy(mismatched_program);

	SDL_Rect scroll = {.x = 1, .y = 1, .w = 1000, .h = 1000};
	cw_set_code_box(scroll);
	cw_set_scroll_box(scroll);
	int first_row_y = cw_get_code_line_y(0);
	set_mouse_position(100, first_row_y + cw_get_code_line_spacing());
	revision = aa_program_revision(program);
	assert(cw_edit_existing_line_authoritatively(
		first, true, false, true, reject_existing_edit, program) ==
		CW_EXISTING_EDIT_FAILED);
	assert(aa_program_revision(program) == revision);
	assert(cw_get_code_line_at_pos(0) == first);
	assert(cw_edit_existing_line_authoritatively(
		first, true, false, true, edit_domain_from_window, program) ==
		CW_EXISTING_EDIT_COMMITTED);
	assert(aa_program_revision(program) == revision + 1);
	assert(aa_program_instruction_at(program, 0)->id == second_id);
	assert(aa_program_instruction_at(program, 1)->id == first_id);
	assert(cw_get_code_line_at_pos(0) == second);
	assert(cw_get_code_line_at_pos(1) == first);
	assert(cw_domain_bindings_valid(program));

	set_mouse_position(0, 0);
	revision = aa_program_revision(program);
	assert(cw_edit_existing_line_authoritatively(
		second, false, true, true, edit_domain_from_window, program) ==
		CW_EXISTING_EDIT_COMMITTED);
	assert(aa_program_revision(program) == revision + 1);
	assert(aa_program_count(program) == 1);
	assert(cw_get_code_list_size() == 1);
	assert(aa_program_instruction_at(program, 0)->id == first_id);
	assert(cw_get_code_line_at_pos(0) == first);
	assert(cw_domain_bindings_valid(program));
	cl_destroy_code_line(second);

	cw_destroy_code_window_assets();
	assert(!cw_domain_bindings_valid(program));
	aa_program_destroy(program);
}

static void test_domain_append_has_single_authority(void)
{
	aa_program_t *program = aa_program_create();
	assert(program != NULL);
	uint64_t starting_revision = aa_program_revision(program);
	assert(aa_program_reserve(program, 32) == AA_RESULT_OK);
	assert(aa_program_revision(program) == starting_revision);
	reader_fixture_t fixture = {
		.line = {
			.opcode = MOV,
			.has_operand_1 = true,
			.operand_1 = RAX,
			.has_operand_2 = true,
			.operand_2 = IB,
			.line_state = COMPLETE
		},
		.present = true
	};
	fixture.line.identity = &fixture.identity;
	assert(append_from_legacy_snapshot(&fixture.line, NULL, program));
	assert(aa_program_revision(program) == starting_revision + 1);
	assert(aa_program_count(program) == 1);
	const aa_instruction_t *instruction = aa_program_instruction_at(program, 0);
	assert(instruction->opcode == AA_OPCODE_MOV);
	assert(instruction->operands[0].value.reg == AA_REGISTER_RAX);
	assert(instruction->operands[1].value.buffer == AA_BUFFER_INPUT);

	uint64_t committed_revision = aa_program_revision(program);
	aa_legacy_program_reader_t reader = make_reader(&fixture);
	aa_legacy_import_report_t report;
	assert(aa_legacy_program_import(program, &reader, &report) == AA_RESULT_OK);
	assert(aa_program_revision(program) == committed_revision);
	assert(aa_program_instruction_at(program, 0)->id == instruction->id);
	aa_program_destroy(program);
}

static void test_program_reserve_contract(void)
{
	assert(aa_program_reserve(NULL, 0) == AA_RESULT_INVALID_ARGUMENT);
	aa_program_t *program = aa_program_create();
	assert(program != NULL);
	uint64_t revision = aa_program_revision(program);
	assert(aa_program_reserve(program, 0) == AA_RESULT_OK);
	assert(aa_program_reserve(program, 1) == AA_RESULT_OK);
	assert(aa_program_count(program) == 0);
	assert(aa_program_revision(program) == revision);

	aa_instruction_t label = aa_instruction_create(AA_OPCODE_LABEL);
	aa_instruction_id_t label_id;
	assert(aa_program_append(program, &label, &label_id) == AA_RESULT_OK);
	aa_instruction_t jump = aa_instruction_create(AA_OPCODE_JMP);
	aa_instruction_id_t jump_id;
	assert(aa_program_append(program, &jump, &jump_id) == AA_RESULT_OK);
	assert(aa_program_set_operand(program, jump_id, 0,
		aa_operand_label_reference(label_id)) == AA_RESULT_OK);
	aa_instruction_t ordinary = aa_instruction_create(AA_OPCODE_MOV);
	aa_instruction_id_t ordinary_id;
	assert(aa_program_append(program, &ordinary, &ordinary_id) == AA_RESULT_OK);
	aa_instruction_t before[3];
	for (size_t position = 0; position < 3; position++) {
		before[position] = *aa_program_instruction_at(program, position);
	}
	revision = aa_program_revision(program);
	assert(aa_program_reserve(program, 2) == AA_RESULT_OK);
	assert(aa_program_reserve(program, 4) == AA_RESULT_OK);
	assert(aa_program_reserve(program, 8) == AA_RESULT_OK);
	assert(aa_program_reserve(program, 16) == AA_RESULT_OK);
	assert(aa_program_reserve(program, 16) == AA_RESULT_OK);
	assert(aa_program_reserve(program, SIZE_MAX) == AA_RESULT_ALLOCATION_FAILED);
	assert(aa_program_count(program) == 3);
	assert(aa_program_revision(program) == revision);
	for (size_t position = 0; position < 3; position++) {
		const aa_instruction_t *after = aa_program_instruction_at(program, position);
		assert(after->id == before[position].id);
		assert(after->opcode == before[position].opcode);
		assert(after->operand_count == before[position].operand_count);
		assert(after->operands[0].kind == before[position].operands[0].kind);
		assert(after->operands[1].kind == before[position].operands[1].kind);
	}
	const aa_instruction_t *retained_jump = aa_program_find_by_id(program, jump_id);
	assert(retained_jump != NULL);
	assert(retained_jump->operands[0].value.label_instruction_id == label_id);
	assert(aa_program_find_by_id(program, label_id)->opcode == AA_OPCODE_LABEL);

	aa_instruction_t appended = aa_instruction_create(AA_OPCODE_CMP);
	aa_instruction_id_t appended_id;
	assert(aa_program_append(program, &appended, &appended_id) == AA_RESULT_OK);
	assert(appended_id != AA_INSTRUCTION_ID_INVALID);
	assert(aa_program_revision(program) == revision + 1);
	assert(aa_program_count(program) == 4);
	assert(aa_program_find_by_id(program, ordinary_id) != NULL);
	assert(appended_id == ordinary_id + 1);
	assert(aa_program_find_by_id(program, jump_id)->operands[0].value.label_instruction_id ==
		label_id);
	aa_program_destroy(program);
}

static void test_empty_lifecycle_refresh(void)
{
	aa_program_t *program = aa_program_create();
	assert(program != NULL);
	reader_fixture_t fixture = {0};
	aa_legacy_program_reader_t reader = make_reader(&fixture);
	aa_legacy_import_report_t report;
	assert(aa_legacy_program_import(program, &reader, &report) == AA_RESULT_OK);
	assert(aa_program_count(program) == 0);
	uint64_t revision = aa_program_revision(program);
	assert(aa_legacy_program_import(program, &reader, &report) == AA_RESULT_OK);
	assert(aa_program_revision(program) == revision);
	aa_program_destroy(program);
}

static void test_incomplete_and_conversion_failure(void)
{
	aa_legacy_import_report_t report;
	aa_instruction_t instruction;
	aa_legacy_line_snapshot_t incomplete = {
		.opcode = ADD,
		.has_operand_1 = true,
		.operand_1 = RBX,
		.line_state = MISSING_OP2
	};
	assert(aa_legacy_instruction_from_snapshot(&incomplete, &instruction,
													   &report) == AA_RESULT_OK);
	assert(instruction.operands[0].value.reg == AA_REGISTER_RBX);
	assert(instruction.operands[1].kind == AA_OPERAND_NONE);
	assert(aa_instruction_validate(&instruction) == AA_RESULT_INVALID_OPERAND);

	aa_legacy_line_snapshot_t unsupported = {
		.opcode = MOV,
		.has_operand_1 = true,
		.operand_1 = RAX,
		.has_operand_2 = true,
		.operand_2 = IMMDO9
	};
	unsupported.operand_2 = IMM_MAX + 1;
	instruction = aa_instruction_create(AA_OPCODE_LABEL);
	assert(aa_legacy_instruction_from_snapshot(&unsupported, &instruction,
													   &report) == AA_RESULT_INVALID_OPERAND);
	assert(report.issue == AA_LEGACY_IMPORT_ISSUE_UNKNOWN_OPERAND);
	assert(instruction.opcode == AA_OPCODE_LABEL);
}

static void test_program_lifecycle_reuse(void)
{
	aa_program_t *program = aa_program_create();
	assert(program != NULL);
	aa_legacy_line_snapshot_t saved_lines[2] = {
		{.opcode = LABEL, .line_state = COMPLETE},
		{.opcode = JMP, .has_operand_1 = true, .line_state = COMPLETE}
	};
	unsigned char saved_identity[2];
	saved_lines[0].identity = &saved_identity[0];
	saved_lines[1].identity = &saved_identity[1];
	saved_lines[1].jump_target_identity = saved_lines[0].identity;
	array_reader_fixture_t saved_context = {
		.lines = saved_lines,
		.count = 2
	};
	aa_legacy_program_reader_t reader = {
		.count = array_reader_count,
		.read_line = array_reader_line,
		.context = &saved_context
	};
	aa_legacy_import_report_t report;
	assert(aa_legacy_program_import(program, &reader, &report) == AA_RESULT_OK);
	assert(aa_program_count(program) == 2);
	const aa_instruction_t *jump = aa_program_instruction_at(program, 1);
	assert(jump->opcode == AA_OPCODE_JMP);
	aa_instruction_id_t stale_target_id =
		jump->operands[0].value.label_instruction_id;
	assert(aa_program_find_by_id(program, stale_target_id)->opcode == AA_OPCODE_LABEL);

	aa_legacy_line_snapshot_t starter = {
		.identity = &saved_identity[0],
		.opcode = MOV,
		.has_operand_1 = true,
		.operand_1 = OB,
		.has_operand_2 = true,
		.operand_2 = RAX,
		.line_state = COMPLETE
	};
	array_reader_fixture_t starter_context = {.lines = &starter, .count = 1};
	reader.context = &starter_context;
	assert(aa_legacy_program_import(program, &reader, &report) == AA_RESULT_OK);
	assert(aa_program_count(program) == 1);
	assert(aa_program_instruction_at(program, 0)->opcode == AA_OPCODE_MOV);
	assert(aa_program_find_by_id(program, stale_target_id) == NULL);

	aa_program_clear(program);
	assert(aa_program_count(program) == 0);
	array_reader_fixture_t next_level_context = {.lines = NULL, .count = 0};
	reader.context = &next_level_context;
	assert(aa_legacy_program_import(program, &reader, &report) == AA_RESULT_OK);
	assert(aa_program_count(program) == 0);
	aa_program_destroy(program);
}

static void test_bound_jump_target_completion_reconciliation(void)
{
	aa_program_t *program = aa_program_create();
	assert(program != NULL);
	cw_create_code_list();
	code_line_t *jump = create_jump_line(JMP, "", 0, NULL);
	code_line_t *unrelated = create_complete_ordinary_line(MOV);
	code_line_t *label = create_label_line("00:", 0);
	assert(cw_player_holding_instruction(jump, false, false));
	cw_clear_held_instruction();
	assert(cw_player_holding_instruction(unrelated, false, false));
	cw_clear_held_instruction();
	assert(cw_player_holding_instruction(label, false, false));
	cw_clear_held_instruction();
	assert(stages_refresh_program_snapshot(program));
	assert(cw_domain_bindings_valid(program));
	assert(aa_program_count(program) == 3);
	aa_instruction_id_t jump_id = aa_program_instruction_at(program, 0)->id;
	aa_instruction_id_t unrelated_id = aa_program_instruction_at(program, 1)->id;
	aa_instruction_id_t label_id = aa_program_instruction_at(program, 2)->id;
	uint64_t revision = aa_program_revision(program);

	operand_t *target = cw_create_jmp_op(label);
	assert(target != NULL);
	cw_assign_op_to_line(target, jump);
	assert(jump->state == COMPLETE);
	assert(stages_reconcile_program_snapshot(program));
	assert(aa_program_revision(program) == revision + 1);
	assert(aa_program_instruction_at(program, 0)->id == jump_id);
	assert(aa_program_instruction_at(program, 1)->id == unrelated_id);
	assert(aa_program_instruction_at(program, 2)->id == label_id);
	assert(aa_program_find_by_id(program, unrelated_id)->opcode == AA_OPCODE_MOV);
	const aa_instruction_t *domain_jump = aa_program_find_by_id(program, jump_id);
	assert(domain_jump != NULL && domain_jump->operands[0].kind ==
		AA_OPERAND_LABEL_REFERENCE);
	assert(domain_jump->operands[0].value.label_instruction_id == label_id);
	assert(cw_domain_bindings_valid(program));

	assert(stages_reconcile_program_snapshot(program));
	assert(aa_program_revision(program) == revision + 1);
	assert(aa_program_find_by_id(program, jump_id) == domain_jump);
	assert(cw_domain_bindings_valid(program));

	code_line_t *external_label = create_label_line("00:", 0);
	cl_destroy_operand(jump->op1);
	jump->op1 = create_repair_test_operand("00", 0, external_label, false);
	jump->state = COMPLETE;
	revision = aa_program_revision(program);
	assert(!stages_reconcile_program_snapshot(program));
	assert(aa_program_revision(program) == revision);
	assert(aa_program_find_by_id(program, jump_id)->operands[0].value.
		label_instruction_id == label_id);
	assert(!cw_domain_bindings_valid(program));
	cl_destroy_code_line(external_label);
	cw_clear_code_list();
	cw_destroy_code_window_assets();
	cw_clear_domain_bindings();
	aa_program_destroy(program);
}

static void test_failed_domain_commit_is_atomic(void)
{
	aa_program_t *program = aa_program_create();
	assert(program != NULL);
	reader_fixture_t fixture = {
		.line = {.opcode = MOV, .has_operand_1 = true, .operand_1 = RAX},
		.present = true
	};
	fixture.line.identity = &fixture.identity;
	uint64_t revision = aa_program_revision(program);
	assert(!reject_domain_append(&fixture.line, NULL, program));
	assert(aa_program_count(program) == 0);
	assert(aa_program_revision(program) == revision);
	aa_program_destroy(program);
}

static void test_move_remove_revision_and_rollback(void)
{
	aa_program_t *program = aa_program_create();
	assert(program != NULL);
	aa_instruction_t instruction = aa_instruction_create(AA_OPCODE_MOV);
	aa_instruction_id_t first_id;
	aa_instruction_id_t second_id;
	aa_instruction_id_t third_id;
	assert(aa_program_append(program, &instruction, &first_id) == AA_RESULT_OK);
	assert(aa_program_append(program, &instruction, &second_id) == AA_RESULT_OK);
	assert(aa_program_append(program, &instruction, &third_id) == AA_RESULT_OK);
	uint64_t revision = aa_program_revision(program);
	assert(aa_program_move(program, 0, 2) == AA_RESULT_OK);
	assert(aa_program_revision(program) == revision + 1);
	assert(aa_program_instruction_at(program, 2)->id == first_id);

	revision = aa_program_revision(program);
	assert(aa_program_remove(program, 1) == AA_RESULT_OK);
	assert(aa_program_revision(program) == revision + 1);
	assert(aa_program_find_by_id(program, third_id) == NULL);
	assert(aa_program_find_by_id(program, first_id) != NULL);
	assert(aa_program_find_by_id(program, second_id) != NULL);

	revision = aa_program_revision(program);
	assert(aa_program_move(program, 9, 0) == AA_RESULT_OUT_OF_RANGE);
	assert(aa_program_remove(program, 9) == AA_RESULT_OUT_OF_RANGE);
	assert(aa_program_revision(program) == revision);
	assert(aa_program_count(program) == 2);
	aa_program_destroy(program);
}

int main(void)
{
	init_test_graphics();
	test_domain_append_has_single_authority();
	test_empty_lifecycle_refresh();
	test_incomplete_and_conversion_failure();
	test_failed_domain_commit_is_atomic();
	test_program_lifecycle_reuse();
	test_move_remove_revision_and_rollback();
	test_code_window_transactions();
	test_bound_jump_target_completion_reconciliation();
	test_domain_authoritative_control_flow_transactions();
	test_program_reserve_contract();
	test_control_flow_repair_empty_and_ordinary();
	test_control_flow_repair_positions_and_identity();
	test_control_flow_repair_consecutive_labels();
	test_proposed_order_repair();
	test_control_flow_repair_rejects_invalid_targets();
	test_saved_jump_reconstruction_is_atomic();
	test_live_repair_rejects_malformed_incomplete_jump();
	test_control_flow_repair_failure_atomicity();
	TTF_CloseFont(test_font);
	SDL_DestroyRenderer(test_renderer);
	SDL_FreeSurface(test_surface);
	TTF_Quit();
	SDL_Quit();
	puts("program editor sync tests passed");
	return 0;
}