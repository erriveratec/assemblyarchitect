#include <assert.h>
#include <stdio.h>
#include <SDL_ttf.h>

#include "code_window_cw_internal.h"
#include "buffers_bf.h"
#include "mouse_ms.h"
#include "registers_rg.h"

extern operand_t *input_buffer;
extern operand_t *output_buffer;

typedef enum clone_failure_t {
	CLONE_FAIL_NONE,
	CLONE_FAIL_INSTRUCTION_TEXTURE,
	CLONE_FAIL_INSTRUCTION_BUTTON,
	CLONE_FAIL_INSTRUCTION_OBJECT,
	CLONE_FAIL_LINE_OBJECT,
	CLONE_FAIL_OPERAND_1,
	CLONE_FAIL_OPERAND_2
} clone_failure_t;

static clone_failure_t clone_failure;
static bool clone_active;
static int clone_operand_factory_calls;
static int destroyed_lines;
static int destroyed_instructions;
static int destroyed_operands;
static int destroyed_buttons;
static int destroyed_textures;

static bool no_selection(void);
static void no_highlight(void);
static code_line_t *clone_test_create_line(instruction_t *instruction);
static void clone_test_destroy_line(code_line_t *line);
static void clone_test_destroy_instruction(instruction_t *instruction);
static void clone_test_destroy_operand(operand_t *operand);
static texture_t *clone_test_instruction_texture(int id);
static btn_t *clone_test_create_button(SDL_Rect rect, texture_t *texture);
static instruction_t *clone_test_create_instruction(int id, btn_t *button);
static void clone_test_free_texture(texture_t *texture);
static void clone_test_destroy_button(btn_t *button);
static operand_t *clone_test_register_operand(int id);
static operand_t *clone_test_buffer_operand(int id);
static operand_t *clone_test_immediate_operand(int id);

#define rg_chk_rel_in_reg no_selection
#define bf_ms_rel_in_buf no_selection
#define im_ms_rel_in_upimm no_selection
#define cw_highlight_code_pending_operand no_highlight
#include "../source/stages.c"
#undef rg_chk_rel_in_reg
#undef bf_ms_rel_in_buf
#undef im_ms_rel_in_upimm
#undef cw_highlight_code_pending_operand

#define cl_create_code_line clone_test_create_line
#define cl_destroy_code_line clone_test_destroy_line
#define cl_destroy_instruction clone_test_destroy_instruction
#define cl_destroy_operand clone_test_destroy_operand
#include "../source/code_edit_ce.c"
#undef cl_create_code_line
#undef cl_destroy_code_line
#undef cl_destroy_instruction
#undef cl_destroy_operand

#define cl_create_instruction_texture clone_test_instruction_texture
#define bt_create_btn clone_test_create_button
#define cl_create_instruction clone_test_create_instruction
#define dw_free_texture clone_test_free_texture
#define bt_destroy_button clone_test_destroy_button
#define rg_create_register_operand_by_id clone_test_register_operand
#define bf_create_buffer_operand_by_id clone_test_buffer_operand
#define im_create_imm_op_by_id clone_test_immediate_operand
#include "../source/code_window_cw.c"
#undef cl_create_instruction_texture
#undef bt_create_btn
#undef cl_create_instruction
#undef dw_free_texture
#undef bt_destroy_button
#undef rg_create_register_operand_by_id
#undef bf_create_buffer_operand_by_id
#undef im_create_imm_op_by_id

static bool no_selection(void)
{
	return false;
}

static void no_highlight(void)
{
}

static void clone_test_destroy_line(code_line_t *line)
{
	assert(line != NULL);
	destroyed_lines++;
	destroyed_instructions++;
	destroyed_buttons++;
	destroyed_textures++;
	if (line->op1 != NULL) {
		destroyed_operands++;
		destroyed_buttons++;
		destroyed_textures++;
	}
	if (line->op2 != NULL) {
		destroyed_operands++;
		destroyed_buttons++;
		destroyed_textures++;
	}
	cl_destroy_code_line(line);
}

static void clone_test_destroy_instruction(instruction_t *instruction)
{
	assert(instruction != NULL);
	destroyed_instructions++;
	destroyed_buttons++;
	destroyed_textures++;
	cl_destroy_instruction(instruction);
}

static void clone_test_destroy_operand(operand_t *operand)
{
	assert(operand != NULL);
	destroyed_operands++;
	destroyed_buttons++;
	destroyed_textures++;
	cl_destroy_operand(operand);
}

static code_line_t *clone_test_create_line(instruction_t *instruction)
{
	if (clone_active && clone_failure == CLONE_FAIL_LINE_OBJECT) {
		return NULL;
	}
	return cl_create_code_line(instruction);
}

static texture_t *clone_test_instruction_texture(int id)
{
	if (clone_active && clone_failure == CLONE_FAIL_INSTRUCTION_TEXTURE) {
		return NULL;
	}
	return cl_create_instruction_texture(id);
}

static btn_t *clone_test_create_button(SDL_Rect rect, texture_t *texture)
{
	if (clone_active && clone_failure == CLONE_FAIL_INSTRUCTION_BUTTON) {
		return NULL;
	}
	return bt_create_btn(rect, texture);
}

static instruction_t *clone_test_create_instruction(int id, btn_t *button)
{
	if (clone_active && clone_failure == CLONE_FAIL_INSTRUCTION_OBJECT) {
		return NULL;
	}
	return cl_create_instruction(id, button);
}

static void clone_test_free_texture(texture_t *texture)
{
	if (clone_active) {
		destroyed_textures++;
	}
	dw_free_texture(texture);
}

static void clone_test_destroy_button(btn_t *button)
{
	if (clone_active) {
		destroyed_buttons++;
		destroyed_textures++;
	}
	bt_destroy_button(button);
}

static bool should_fail_operand_factory(void)
{
	clone_operand_factory_calls++;
	return (clone_failure == CLONE_FAIL_OPERAND_1 &&
	        clone_operand_factory_calls == 1) ||
	       (clone_failure == CLONE_FAIL_OPERAND_2 &&
	        clone_operand_factory_calls == 2);
}

static operand_t *clone_test_register_operand(int id)
{
	return clone_active && should_fail_operand_factory()
		? NULL : rg_create_register_operand_by_id(id);
}

static operand_t *clone_test_buffer_operand(int id)
{
	return clone_active && should_fail_operand_factory()
		? NULL : bf_create_buffer_operand_by_id(id);
}

static operand_t *clone_test_immediate_operand(int id)
{
	return clone_active && should_fail_operand_factory()
		? NULL : im_create_imm_op_by_id(id);
}

static void clear_program(void)
{
	cw_cancel_pending_operand_change();
	cw_clear_code_list();
	ms_init_mouse();
}

static code_line_t *add_saved_line(const char *text)
{
	char mutable_text[64];
	assert(snprintf(mutable_text, sizeof(mutable_text), "%s", text) > 0);
	cw_add_saved_line(mutable_text);
	return cp_get(g_code_window.program, cp_count(g_code_window.program) - 1);
}

static void assert_independent_operand(
	const operand_t *source, const operand_t *clone)
{
	assert(source != NULL && clone != NULL);
	assert(source != clone && source->id == clone->id);
	assert(source->b != clone->b && source->b->t != clone->b->t);
	assert(source->b->t->texture != clone->b->t->texture);
	assert(clone->jptr == NULL && clone->target_label_identity == 0);
}

static void test_complete_ordinary_clones(void)
{
	static const char *valid_lines[] = {
		"MOV rax rbx",
		"MOV rax [ib]",
		"MOV rax IMMUP0",
		"ADD rbx IMMUP5",
		"CMP rcx IMMUP_1"
	};
	for (size_t index = 0;
	     index < sizeof(valid_lines) / sizeof(valid_lines[0]); index++) {
		clear_program();
		code_line_t *source = add_saved_line(valid_lines[index]);
		assert(source != NULL && source->state == COMPLETE);
		if (index == 0) {
			source->ins->b->animated = true;
			source->op1->b->animated = true;
			source->op2->b->animated = true;
		}
		code_line_t *clone = NULL;
		assert(cw_clone_rclicked_line(source, &clone) == CE_OK);
		assert(clone != NULL && clone != source);
		assert(clone->ins != source->ins && clone->ins->id == source->ins->id);
		assert(clone->ins->b != source->ins->b);
		assert(clone->ins->b->t != source->ins->b->t);
		assert(clone->ins->b->t->texture != source->ins->b->t->texture);
		assert(clone->state == COMPLETE);
		assert_independent_operand(source->op1, clone->op1);
		assert_independent_operand(source->op2, clone->op2);
		assert(!clone->ins->b->animated && !clone->op1->b->animated &&
		       !clone->op2->b->animated);
		assert(clone->op1->id == source->op1->id);
		assert(clone->op2->id == source->op2->id);

		cw_player_holding_instruction(clone, false, false);
		assert(cp_contains(g_code_window.program, clone));
		assert(clone->state == COMPLETE && clone->op1 != NULL &&
		       clone->op2 != NULL);
		assert(!ce_has_pending_operand(
			g_code_window.program, CE_PENDING_ANY));
		clear_program();
	}
}

static void test_incomplete_clones(void)
{
	static const struct {
		const char *text;
		int state;
		bool has_op1;
		bool has_op2;
	} cases[] = {
		{"MOV rax _", MISSING_OP2, true, false},
		{"MOV _ IMMUP7", MISSING_OP1, false, true},
		{"MOV _ _", MISSING_BOTH, false, false}
	};
	for (size_t index = 0; index < sizeof(cases) / sizeof(cases[0]); index++) {
		clear_program();
		code_line_t *source = add_saved_line(cases[index].text);
		assert(source->state == cases[index].state);
		code_line_t *clone = NULL;
		assert(cw_clone_rclicked_line(source, &clone) == CE_OK);
		assert(clone != NULL && clone->state == cases[index].state);
		assert((clone->op1 != NULL) == cases[index].has_op1);
		assert((clone->op2 != NULL) == cases[index].has_op2);
		if (cases[index].has_op1) {
			assert_independent_operand(source->op1, clone->op1);
		}
		if (cases[index].has_op2) {
			assert_independent_operand(source->op2, clone->op2);
		}
		cl_destroy_code_line(clone);
		assert(source->state == cases[index].state);
		clear_program();
	}
}

static int label_count(void)
{
	int count = 0;
	for (int index = 0; index < cp_count(g_code_window.program); index++) {
		count += cp_get(g_code_window.program, index)->ins->id == LABEL;
	}
	return count;
}

static void release_at(int x, int y)
{
	SDL_Event event = {0};
	event.type = SDL_MOUSEBUTTONUP;
	event.button.button = SDL_BUTTON_LEFT;
	event.button.state = SDL_RELEASED;
	event.button.x = x;
	event.button.y = y;
	ms_mouse_button_handler(event);
}

static void test_jump_clones(void)
{
	const int jump_ids[] = {JMP, JE, JNE};
	const char *jump_texts[] = {"JMP 1", "JE 1", "JNE 1"};
	for (size_t index = 0; index < 3; index++) {
		clear_program();
		code_line_t *source = add_saved_line(jump_texts[index]);
		code_line_t *target = add_saved_line("line 1");
		cw_update_saved_jump_instructions();
		assert(source->ins->id == jump_ids[index]);
		assert(source->state == COMPLETE && source->op1->jptr == target);
		uint64_t identity = target->label_identity;
		assert(identity != 0 &&
		       source->op1->target_label_identity == identity);
		int labels_before = label_count();
		code_line_t *clone = NULL;
		assert(cw_clone_rclicked_line(source, &clone) == CE_OK);
		assert(clone->state == COMPLETE && clone->op1 != NULL);
		assert(clone->op1 != source->op1 && clone->op1->b != source->op1->b);
		assert(clone->op1->b->t != source->op1->b->t);
		assert(clone->op1->id == source->op1->id);
		assert(clone->op1->jptr == target);
		assert(clone->op1->target_label_identity == identity);
		assert(label_count() == labels_before);
		assert(!ce_has_pending_operand(
			g_code_window.program, CE_PENDING_ANY));

		operand_t *placed_operand = clone->op1;
		btn_t *placed_button = clone->op1->b;
		cw_player_holding_instruction(clone, false, false);
		assert(cp_contains(g_code_window.program, clone));
		assert(clone->state == COMPLETE && clone->op1 == placed_operand);
		assert(clone->op1->b == placed_button && clone->op1->jptr == target);
		assert(clone->op1->target_label_identity == identity);
		assert(label_count() == labels_before);
		for (int frame = 0; frame < 5; frame++) {
			assert(pending_operand_handler() == NULL);
			assert(label_count() == labels_before);
			assert(clone->state == COMPLETE && clone->op1 != NULL);
			assert(clone->op1->jptr == target);
			assert(clone->op1->target_label_identity == identity);
		}

		release_at(clone->op1->b->r.x + 5, clone->op1->b->r.y + 5);
		cw_change_clicked_code_line_state();
		assert(clone->state == CHANGING_OP1);
		assert(clone->op1->b->animated);
		assert(target->ins->b->animated && target->op1->b->animated);
		assert(source->op1->jptr == target &&
		       source->op1->target_label_identity == identity);
		cw_cancel_pending_operand_change();
		clear_program();
	}
}

static void test_clone_independence(void)
{
	clear_program();
	code_line_t *source = add_saved_line("MOV rax rbx");
	code_line_t *clone = NULL;
	assert(cw_clone_rclicked_line(source, &clone) == CE_OK);
	cw_player_holding_instruction(clone, false, false);
	clone->state = CHANGING_OP1;
	cw_assign_op_to_line(rg_create_register_operand_by_id(RCX), clone);
	assert(clone->op1->id == RCX && clone->state == COMPLETE);
	assert(source->op1->id == RAX && source->state == COMPLETE);
	code_line_t *removed = NULL;
	assert(cp_remove(g_code_window.program, cp_index_of(
		g_code_window.program, clone), &removed) == CP_OK);
	assert(removed == clone);
	cl_destroy_code_line(removed);
	assert(source->op1->id == RAX && source->op2->id == RBX);

	assert(cw_clone_rclicked_line(source, &clone) == CE_OK);
	assert(cp_remove(g_code_window.program, cp_index_of(
		g_code_window.program, source), &removed) == CP_OK);
	assert(removed == source);
	cl_destroy_code_line(removed);
	assert(clone->state == COMPLETE && clone->op1->id == RAX &&
	       clone->op2->id == RBX);
	cl_destroy_code_line(clone);
	clear_program();
}

static void reset_failure_counts(void)
{
	destroyed_lines = 0;
	destroyed_instructions = 0;
	destroyed_operands = 0;
	destroyed_buttons = 0;
	destroyed_textures = 0;
	clone_operand_factory_calls = 0;
}

static void test_allocation_rollback(void)
{
	clear_program();
	code_line_t *source = add_saved_line("MOV rax rbx");
	operand_t *source_op1 = source->op1;
	operand_t *source_op2 = source->op2;
	int original_count = cp_count(g_code_window.program);
	for (clone_failure_t failure = CLONE_FAIL_INSTRUCTION_TEXTURE;
	     failure <= CLONE_FAIL_OPERAND_2; failure++) {
		reset_failure_counts();
		clone_failure = failure;
		clone_active = true;
		code_line_t *clone = NULL;
		ce_result_t result = cw_clone_rclicked_line(source, &clone);
		clone_active = false;
		clone_failure = CLONE_FAIL_NONE;
		assert(result != CE_OK && clone == NULL);
		assert(cp_count(g_code_window.program) == original_count);
		assert(source->state == COMPLETE && source->op1 == source_op1 &&
		       source->op2 == source_op2);
		assert(source_op1->id == RAX && source_op2->id == RBX);
		switch (failure) {
		case CLONE_FAIL_INSTRUCTION_TEXTURE:
			assert(destroyed_lines == 0 && destroyed_instructions == 0);
			assert(destroyed_operands == 0 && destroyed_buttons == 0 &&
			       destroyed_textures == 0);
			break;
		case CLONE_FAIL_INSTRUCTION_BUTTON:
			assert(destroyed_lines == 0 && destroyed_instructions == 0);
			assert(destroyed_operands == 0 && destroyed_buttons == 0 &&
			       destroyed_textures == 1);
			break;
		case CLONE_FAIL_INSTRUCTION_OBJECT:
			assert(destroyed_lines == 0 && destroyed_instructions == 0);
			assert(destroyed_operands == 0 && destroyed_buttons == 1 &&
			       destroyed_textures == 1);
			break;
		case CLONE_FAIL_LINE_OBJECT:
			assert(destroyed_lines == 0 && destroyed_instructions == 1);
			assert(destroyed_operands == 0 && destroyed_buttons == 1 &&
			       destroyed_textures == 1);
			break;
		case CLONE_FAIL_OPERAND_1:
			assert(destroyed_lines == 1 && destroyed_instructions == 1);
			assert(destroyed_operands == 0 && destroyed_buttons == 1 &&
			       destroyed_textures == 1);
			break;
		case CLONE_FAIL_OPERAND_2:
			assert(destroyed_lines == 1 && destroyed_instructions == 1);
			assert(destroyed_operands == 1 && destroyed_buttons == 2 &&
			       destroyed_textures == 2);
			break;
		case CLONE_FAIL_NONE:
			assert(false);
		}
	}
	clear_program();
}

static void test_label_copy_restriction(void)
{
	clear_program();
	code_line_t *label = add_saved_line("line 1");
	cw_update_saved_jump_instructions();
	ms_init_mouse();
	SDL_Event right_press = {0};
	right_press.button.button = SDL_BUTTON_RIGHT;
	right_press.button.state = SDL_PRESSED;
	right_press.button.x = label->ins->b->r.x + 5;
	right_press.button.y = label->ins->b->r.y + 5;
	ms_mouse_button_handler(right_press);
	assert(!cw_chk_rclick_code());
	code_line_t *clone = NULL;
	assert(cw_clone_rclicked_line(label, &clone) == CE_INVALID_ARGUMENT);
	assert(clone == NULL && label_count() == 1);
	clear_program();
}

int main(void)
{
	assert(SDL_Init(0) == 0 && TTF_Init() == 0);
	g_screen = SDL_CreateRGBSurfaceWithFormat(
		0, 1920, 1080, 32, SDL_PIXELFORMAT_RGBA32);
	assert(g_screen != NULL);
	g_renderer = SDL_CreateSoftwareRenderer(g_screen);
	assert(g_renderer != NULL);
	g_font = TTF_OpenFont("fonts/DOSVGA437.ttf", 130);
	assert(g_font != NULL);
	cw_create_code_list();
	cw_init_code_window_texture();
	bf_init_buf_ops();
	ms_init_mouse();

	test_complete_ordinary_clones();
	test_incomplete_clones();
	test_jump_clones();
	test_clone_independence();
	test_allocation_rollback();
	test_label_copy_restriction();

	cw_destroy_code_window_assets();
	cl_destroy_operand(input_buffer);
	cl_destroy_operand(output_buffer);
	input_buffer = NULL;
	output_buffer = NULL;
	cw_destroy_renderer_assets();
	TTF_CloseFont(g_font);
	SDL_DestroyRenderer(g_renderer);
	SDL_FreeSurface(g_screen);
	TTF_Quit();
	SDL_Quit();
	puts("code line clone tests passed");
	return 0;
}
