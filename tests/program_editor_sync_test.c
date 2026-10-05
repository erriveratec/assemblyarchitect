#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "code_window_cw.h"
#include "dimensions_dm.h"
#include "draw_dw.h"
#include "domain/program.h"
#include "legacy_code_ids.h"
#include "migration/legacy_program_adapter.h"
#include "mouse_ms.h"
#include "sdl_config.h"
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
	void *context)
{
	aa_program_t *program = context;
	aa_instruction_t instruction;
	aa_legacy_import_report_t report;
	if (aa_legacy_instruction_from_snapshot(snapshot, &instruction, &report) !=
		AA_RESULT_OK) {
		return false;
	}
	return aa_program_append(program, &instruction, NULL) == AA_RESULT_OK;
}

static bool reject_domain_append(const aa_legacy_line_snapshot_t *snapshot,
								 void *context)
{
	(void)snapshot;
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
}

static void test_code_window_transactions(void)
{
	aa_program_t *program = aa_program_create();
	assert(program != NULL);
	cw_set_challenge_text("test challenge");
	cw_create_code_list();
	code_line_t *first = create_line(MOV);
	uint64_t revision = aa_program_revision(program);
	assert(cw_append_new_line_authoritatively(
		first, false, false, reject_domain_append, program) == CW_APPEND_FAILED);
	assert(aa_program_count(program) == 0);
	assert(aa_program_revision(program) == revision);
	assert(cw_get_code_list_size() == 0);
	assert(cw_append_new_line_authoritatively(
		first, false, false, append_from_legacy_snapshot, program) ==
		CW_APPEND_COMMITTED);
	assert(aa_program_count(program) == 1);
	assert(aa_program_revision(program) == revision + 1);
	assert(cw_get_code_list_size() == 1);
	assert(cw_get_code_line_at_pos(0) == first);
	aa_instruction_id_t first_id = aa_program_instruction_at(program, 0)->id;

	code_line_t *second = create_line(ADD);
	revision = aa_program_revision(program);
	assert(cw_append_new_line_authoritatively(
		second, false, false, append_from_legacy_snapshot, program) ==
		CW_APPEND_COMMITTED);
	assert(aa_program_count(program) == 2);
	assert(aa_program_revision(program) == revision + 1);
	assert(cw_get_code_line_at_pos(1) == second);
	aa_instruction_id_t second_id = aa_program_instruction_at(program, 1)->id;

	SDL_Rect scroll = {.x = 1, .y = 1, .w = 1000, .h = 1000};
	cw_set_code_box(scroll);
	cw_set_scroll_box(scroll);
	int first_row_y = cw_get_code_line_y(0);
	set_mouse_position(100, first_row_y + cw_get_code_line_spacing());
	revision = aa_program_revision(program);
	assert(cw_edit_existing_line_authoritatively(
		first, true, false, false, reject_existing_edit, program) ==
		CW_EXISTING_EDIT_FAILED);
	assert(aa_program_revision(program) == revision);
	assert(cw_get_code_line_at_pos(0) == first);
	assert(cw_edit_existing_line_authoritatively(
		first, true, false, false, edit_domain_from_window, program) ==
		CW_EXISTING_EDIT_COMMITTED);
	assert(aa_program_revision(program) == revision + 1);
	assert(aa_program_instruction_at(program, 0)->id == second_id);
	assert(aa_program_instruction_at(program, 1)->id == first_id);
	assert(cw_get_code_line_at_pos(0) == second);
	assert(cw_get_code_line_at_pos(1) == first);

	set_mouse_position(0, 0);
	revision = aa_program_revision(program);
	assert(cw_edit_existing_line_authoritatively(
		second, false, true, false, edit_domain_from_window, program) ==
		CW_EXISTING_EDIT_COMMITTED);
	assert(aa_program_revision(program) == revision + 1);
	assert(aa_program_count(program) == 1);
	assert(cw_get_code_list_size() == 1);
	assert(aa_program_instruction_at(program, 0)->id == first_id);
	assert(cw_get_code_line_at_pos(0) == first);
	cl_destroy_code_line(second);

	cw_destroy_code_window_assets();
	aa_program_destroy(program);
}

static void test_domain_append_has_single_authority(void)
{
	aa_program_t *program = aa_program_create();
	assert(program != NULL);
	uint64_t starting_revision = aa_program_revision(program);
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
	assert(append_from_legacy_snapshot(&fixture.line, program));
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
	assert(!reject_domain_append(&fixture.line, program));
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
	TTF_CloseFont(test_font);
	SDL_DestroyRenderer(test_renderer);
	SDL_FreeSurface(test_surface);
	TTF_Quit();
	SDL_Quit();
	puts("program editor sync tests passed");
	return 0;
}