#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL_ttf.h>

#include "code_edit_ce.h"
#include "code_window_cw_internal.h"

static bool reject_assignment;
static bool reject_insertion;
static bool reject_operand_allocation;
static int label_destructions;
static int candidate_destructions;
static int replaced_destructions;
static operand_t *replaced_operand;
static operand_t *candidate_operand;
static code_line_t foreign_label;
static instruction_t foreign_instruction = {.id = MOV};

static ce_result_t checked_replace(
	ce_session_t *session, operand_t *operand, operand_t **replaced);
static ce_result_t checked_insert(
	code_program_t *program, int index, code_line_t *line);
static void counted_destroy_operand(operand_t *operand);
static void counted_destroy_line(code_line_t *line);
static void *checked_malloc(size_t size);

#define rg_chk_rel_in_reg no_selection
#define bf_ms_rel_in_buf no_selection
#define im_ms_rel_in_upimm no_selection
#define cw_highlight_code_pending_operand no_highlight
#include "../source/stages.c"
#undef rg_chk_rel_in_reg
#undef bf_ms_rel_in_buf
#undef im_ms_rel_in_upimm
#undef cw_highlight_code_pending_operand

#define ce_replace_operand checked_replace
#define ce_insert_line checked_insert
#define cl_destroy_operand counted_destroy_operand
#define cl_destroy_code_line counted_destroy_line
#define malloc checked_malloc
#include "../source/code_window_cw.c"
#undef ce_replace_operand
#undef ce_insert_line
#undef cl_destroy_operand
#undef cl_destroy_code_line
#undef malloc

bool no_selection(void) { return false; }
void no_highlight(void) {}

static void *checked_malloc(size_t size)
{
	code_line_t *last = cp_get(g_code_window.program,
	                          cp_count(g_code_window.program) - 1);
	if (reject_operand_allocation && size == sizeof(operand_t) &&
	    last != NULL && last->ins->id == LABEL && last->op1 != NULL) {
		reject_operand_allocation = false;
		return NULL;
	}
	return malloc(size);
}

static ce_result_t checked_replace(
	ce_session_t *session, operand_t *operand, operand_t **replaced)
{
	candidate_operand = operand;
	if (reject_assignment) {
		operand->jptr = &foreign_label;
		operand->target_label_identity = foreign_label.label_identity;
	}
	return ce_replace_operand(session, operand, replaced);
}

static ce_result_t checked_insert(
	code_program_t *program, int index, code_line_t *line)
{
	return reject_insertion ? CE_PROGRAM_ERROR
		: ce_insert_line(program, index, line);
}

static void counted_destroy_operand(operand_t *operand)
{
	assert(operand != NULL);
	if (operand == candidate_operand) {
		candidate_destructions++;
		candidate_operand = NULL;
	}
	if (operand == replaced_operand) {
		replaced_destructions++;
		replaced_operand = NULL;
	}
	cl_destroy_operand(operand);
}

static void counted_destroy_line(code_line_t *line)
{
	if (line->ins->id == LABEL) {
		label_destructions++;
	}
	cl_destroy_code_line(line);
}

static code_line_t *append_line(int instruction)
{
	texture_t *texture = dw_create_text_tex(
		cl_get_instruction_text(instruction), C_WHITE);
	assert(texture != NULL);
	btn_t *button = bt_create_btn(cl_get_code_button_size(), texture);
	code_line_t *line = cl_create_code_line(
		cl_create_instruction(instruction, button));
	assert(line != NULL);
	assert(cp_insert(g_code_window.program, cp_count(g_code_window.program),
	                 line) == CP_OK);
	return line;
}

static int label_count(void)
{
	int count = 0;
	for (int index = 0; index < cp_count(g_code_window.program); index++) {
		count += cp_get(g_code_window.program, index)->ins->id == LABEL;
	}
	return count;
}

static void reset_program(void)
{
	reject_assignment = false;
	reject_insertion = false;
	reject_operand_allocation = false;
	candidate_operand = NULL;
	replaced_operand = NULL;
	cw_clear_code_list();
	label_destructions = 0;
	candidate_destructions = 0;
	replaced_destructions = 0;
}

static void test_single_label(int preceding_lines)
{
	for (int index = 0; index < preceding_lines; index++) {
		char padding[] = "MOV rax IMMUP0";
		cw_add_saved_line(padding);
	}
	code_line_t *jump = append_line(JMP);
	assert(jump->state == MISSING_OP1);
	int original_count = cp_count(g_code_window.program);
	code_line_t *label = pending_operand_handler();
	assert(label != NULL && label->ins->id == LABEL);
	assert(cp_count(g_code_window.program) == original_count + 1);
	assert(label_count() == 1);
	assert(jump->op1 != NULL && jump->op1->jptr == label);
	uint64_t identity = label->label_identity;
	assert(identity != 0 && jump->op1->target_label_identity == identity);
	assert(jump->state == COMPLETE);
	assert(!ce_has_pending_operand(g_code_window.program, CE_PENDING_ANY));
	assert(cw_get_code_line_pending_operand() == NULL);
	assert(g_code_window.edit_session.state == CE_IDLE);
	assert(g_code_window.edit_session.line == NULL);
	for (int frame = 0; frame < 10; frame++) {
		assert(pending_operand_handler() == NULL);
		assert(cp_count(g_code_window.program) == original_count + 1);
		assert(label_count() == 1);
		assert(jump->op1->jptr == label);
		assert(jump->op1->target_label_identity == identity);
	}
	assert(ce_move_line(g_code_window.program, label, 0) == CE_OK);
	cw_player_holding_instruction(label, true, true);
	assert(label->label_identity == identity);
	assert(jump->op1->target_label_identity == identity);
	reset_program();
}

static void test_rollback(int failure)
{
	code_line_t *jump = append_line(JMP);
	int original_count = cp_count(g_code_window.program);
	reject_insertion = failure == 0;
	reject_operand_allocation = failure == 1;
	reject_assignment = failure == 2;
	assert(pending_operand_handler() == NULL);
	assert(cp_count(g_code_window.program) == original_count);
	assert(label_count() == 0 && label_destructions == 1);
	assert(candidate_destructions == (failure == 2 ? 1 : 0));
	assert(jump->state == MISSING_OP1 && jump->op1 == NULL);
	assert(ce_has_pending_operand(g_code_window.program, CE_PENDING_OPERAND_1));
	reset_program();
}

static void test_replacement(void)
{
	code_line_t *jump = append_line(JMP);
	code_line_t *old_label = pending_operand_handler();
	assert(old_label != NULL);
	code_line_t *new_label = create_label_code_line();
	assert(new_label != NULL);
	cw_player_holding_instruction(new_label, false, true);
	assert(label_count() == 2);
	int original_count = cp_count(g_code_window.program);
	replaced_operand = jump->op1;
	jump->state = CHANGING_OP1;
	operand_t *replacement = create_jump_operand(new_label);
	assert(replacement != NULL);
	assert(try_assign_op_to_line(replacement, jump));
	assert(replaced_destructions == 1);
	assert(jump->op1 == replacement);
	assert(jump->op1->jptr == new_label);
	assert(jump->op1->target_label_identity == new_label->label_identity);
	assert(cp_contains(g_code_window.program, old_label));
	assert(cp_count(g_code_window.program) == original_count);
	assert(jump->state == COMPLETE);
	assert(pending_operand_handler() == NULL && label_count() == 2);
	jump->state = CHANGING_OP1;
	start_active_jump_replacement_highlight(jump);
	assert(new_label->ins->b->animated && new_label->op1->b->animated);
	assert(!old_label->ins->b->animated && !old_label->op1->b->animated);
	reset_program();
}

static void test_nonmember_target(void)
{
	code_line_t *jump = append_line(JMP);
	code_line_t *label = create_label_code_line();
	assert(label != NULL);
	operand_t *operand = calloc(1, sizeof(*operand));
	assert(operand != NULL);
	operand->jptr = label;
	assert(!try_assign_op_to_line(operand, jump));
	assert(jump->state == MISSING_OP1 && jump->op1 == NULL);
	assert(cp_count(g_code_window.program) == 1 && label_count() == 0);
	assert(create_jump_operand(label) == NULL);
	free(operand);
	cl_destroy_code_line(label);
	reset_program();
}

static void test_v1_targets(void)
{
	char forward[] = "JMP 2";
	char middle[] = "MOV rax IMMUP0";
	char label[] = "line 1";
	char backward[] = "JMP 2";
	cw_add_saved_line(forward);
	cw_add_saved_line(middle);
	cw_add_saved_line(label);
	cw_add_saved_line(backward);
	cw_update_saved_jump_instructions();
	code_line_t *target = cp_get(g_code_window.program, 2);
	assert(target->ins->id == LABEL && target->label_identity != 0);
	for (int index = 0; index <= 3; index += 3) {
		code_line_t *jump = cp_get(g_code_window.program, index);
		assert(jump->op1->jptr == target);
		assert(jump->op1->target_label_identity == target->label_identity);
		assert(jump->state == COMPLETE);
		code_line_t *resolved = NULL;
		assert(cf_resolve_jump(g_code_window.program, jump, &resolved) ==
		       CF_VALID_TARGET && resolved == target);
	}
	reset_program();
}

int main(void)
{
	assert(SDL_Init(0) == 0 && TTF_Init() == 0);
	g_screen = SDL_CreateRGBSurfaceWithFormat(0, 1920, 1080, 32,
	                                         SDL_PIXELFORMAT_RGBA32);
	assert(g_screen != NULL);
	g_renderer = SDL_CreateSoftwareRenderer(g_screen);
	assert(g_renderer != NULL);
	g_font = TTF_OpenFont("fonts/DOSVGA437.ttf", 130);
	assert(g_font != NULL);
	foreign_label.ins = &foreign_instruction;
	foreign_label.label_identity = UINT64_MAX;
	cw_create_code_list();
	cw_init_code_window_texture();
	ms_init_mouse();
	test_single_label(0);
	test_single_label(1);
	test_single_label(64);
	for (int failure = 0; failure < 3; failure++) {
		test_rollback(failure);
	}
	test_replacement();
	test_nonmember_target();
	test_v1_targets();
	cw_destroy_code_window_assets();
	cw_destroy_renderer_assets();
	TTF_CloseFont(g_font);
	SDL_DestroyRenderer(g_renderer);
	SDL_FreeSurface(g_screen);
	TTF_Quit();
	SDL_Quit();
	puts("automatic jump label tests passed: one label after 10 updates, COMPLETE, idle");
	return 0;
}
