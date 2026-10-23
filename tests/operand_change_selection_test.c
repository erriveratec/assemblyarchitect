#include <assert.h>
#include <stdio.h>

#define rg_chk_rel_in_reg test_reg_selected
#define bf_ms_rel_in_buf test_buffer_selected
#define cw_ms_rel_in_label test_label_selected
#define im_ms_rel_in_upimm test_immediate_selected
#define im_create_sel_imm_op test_create_selected_immediate
#define cw_highlight_code_pending_operand test_pending_highlight
#include "../source/stages.c"
#undef rg_chk_rel_in_reg
#undef bf_ms_rel_in_buf
#undef cw_ms_rel_in_label
#undef im_ms_rel_in_upimm
#undef im_create_sel_imm_op
#undef cw_highlight_code_pending_operand

#include "list.h"

List *get_code_list(void);

static bool immediate_selected;
static operand_t *selected_immediate;

bool test_reg_selected(void) { return false; }
bool test_buffer_selected(void) { return false; }
bool test_label_selected(void) { return false; }
bool test_immediate_selected(void) { return immediate_selected; }
operand_t *test_create_selected_immediate(void) { return selected_immediate; }
void test_pending_highlight(void) {}

typedef struct line_fixture_t {
	instruction_t instruction;
	btn_t instruction_button;
	code_line_t line;
	operand_t operands[2];
	btn_t buttons[2];
} line_fixture_t;

static line_fixture_t fixtures[12];
static int fixture_count;

static code_line_t *add_line(int instruction_id, bool has_op1, bool has_op2)
{
	int index = fixture_count++;
	assert(index < 12);
	line_fixture_t *fixture = &fixtures[index];
	*fixture = (line_fixture_t){0};
	fixture->instruction.id = instruction_id;
	fixture->instruction.b = &fixture->instruction_button;
	fixture->line.ins = &fixture->instruction;
	fixture->line.state = COMPLETE;
	for (int position = 0; position < 2; position++) {
		fixture->buttons[position].r = (SDL_Rect){
			.x = 100 + position * 100, .y = 100 + index * 40, .w = 80, .h = 30
		};
		fixture->buttons[position].enabled = true;
		fixture->operands[position].b = &fixture->buttons[position];
		fixture->operands[position].id = position == 0 ? RAX : IMMUP7;
	}
	fixture->line.op1 = has_op1 ? &fixture->operands[0] : NULL;
	fixture->line.op2 = has_op2 ? &fixture->operands[1] : NULL;
	List_push(get_code_list(), &fixture->line);
	return &fixture->line;
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

static void assert_reset(btn_t *button)
{
	assert(!button->animated);
	assert(!button->anim_dir);
	assert(button->anim_state == 0);
}

static void select_operand(code_line_t *line, int position)
{
	btn_t *button = position == OP1 ? line->op1->b : line->op2->b;
	SDL_Rect original_rect = button->r;
	button->animated = true;
	button->anim_dir = true;
	button->anim_state = 8;
	release_at(button->r.x + 5, button->r.y + 5);
	cw_change_clicked_code_line_state();
	int changing_count = 0;
	LIST_FOREACH(get_code_list(), first, next, current) {
		code_line_t *candidate = current->value;
		if (candidate->state == CHANGING_OP1 || candidate->state == CHANGING_OP2) {
			changing_count++;
			assert(candidate == line);
		}
	}
	assert(changing_count == 1);
	assert(line->state == (position == OP1 ? CHANGING_OP1 : CHANGING_OP2));
	assert_reset(button);
	assert(button->r.x == original_rect.x && button->r.y == original_rect.y);
	assert(button->r.w == original_rect.w && button->r.h == original_rect.h);
}

static void mark_changing_with_stale_animation(code_line_t *line, int state)
{
	line->state = state;
	operand_t *operands[] = {line->op1, line->op2};
	for (int position = 0; position < 2; position++) {
		if (operands[position] != NULL && operands[position]->b != NULL) {
			operands[position]->b->animated = true;
			operands[position]->b->anim_dir = true;
			operands[position]->b->anim_state = 8;
		}
	}
}

static void reset_fixture(void)
{
	while (List_count(get_code_list()) > 0) {
		List_pop(get_code_list());
	}
	fixture_count = 0;
	immediate_selected = false;
	selected_immediate = NULL;
	ms_init_mouse();
}

static void test_transfer_and_reselection(void)
{
	code_line_t *first = add_line(MOV, true, true);
	code_line_t *second = add_line(ADD, true, true);
	operand_t *original = first->op1;
	select_operand(first, OP1);
	assert(cw_chk_click_code_op());
	mark_changing_with_stale_animation(first, CHANGING_OP1);
	select_operand(second, OP2);
	assert(first->state == COMPLETE);
	assert_reset(first->op1->b);
	assert_reset(first->op2->b);
	assert(first->op1 == original && original->id == RAX);
	assert_reset(second->op1->b);
	select_operand(second, OP1);
	assert_reset(second->op2->b);
	select_operand(second, OP2);
	assert_reset(second->op1->b);
	operand_t *selected = second->op2;
	select_operand(second, OP2);
	assert(second->op2 == selected && selected->id == IMMUP7);
	assert(cw_is_operand_pending());
	assert(cw_get_code_line_pending_operand() == second);
	reset_fixture();
}

static void test_structural_restoration(void)
{
	code_line_t *missing_second = add_line(MOV, true, false);
	code_line_t *missing_first = add_line(ADD, false, true);
	code_line_t *missing_both = add_line(CMP, false, false);
	code_line_t *missing_jump = add_line(JMP, false, false);
	code_line_t *selected = add_line(MOV, true, true);
	select_operand(missing_second, OP1);
	mark_changing_with_stale_animation(missing_first, CHANGING_OP2);
	mark_changing_with_stale_animation(missing_both, CHANGING_OP1);
	mark_changing_with_stale_animation(missing_jump, CHANGING_OP1);
	code_line_t *missing_button = add_line(MOV, true, true);
	missing_button->op1->b = NULL;
	mark_changing_with_stale_animation(missing_button, CHANGING_OP1);
	select_operand(selected, OP2);
	assert(missing_second->state == MISSING_OP2);
	assert(missing_first->state == MISSING_OP1);
	assert(missing_both->state == MISSING_BOTH);
	assert(missing_jump->state == MISSING_OP1);
	assert(missing_button->state == COMPLETE);
	assert_reset(missing_button->op2->b);
	assert_reset(missing_second->op1->b);
	assert_reset(missing_first->op2->b);
	reset_fixture();
}

static void test_jumps_and_labels(void)
{
	int jump_ids[] = {JMP, JE, JNE};
	for (int index = 0; index < 3; index++) {
		code_line_t *jump = add_line(jump_ids[index], true, false);
		code_line_t *label = add_line(LABEL, true, false);
		code_line_t *other = add_line(MOV, true, true);
		jump->op1->jptr = label;
		operand_t *original = jump->op1;
		label->op1->b->animated = true;
		label->op1->b->anim_dir = true;
		label->op1->b->anim_state = 8;
		select_operand(jump, OP1);
		assert_reset(label->op1->b);
		mark_changing_with_stale_animation(label, CHANGING_OP1);
		select_operand(other, OP2);
		assert_reset(jump->op1->b);
		assert_reset(label->op1->b);
		assert(jump->state == COMPLETE && label->state == COMPLETE);
		assert(jump->op1 == original && jump->op1->jptr == label);
		release_at(label->op1->b->r.x + 5, label->op1->b->r.y + 5);
		cw_change_clicked_code_line_state();
		assert(label->state == COMPLETE && other->state == CHANGING_OP2);
		jump->op1->jptr = NULL;
		select_operand(jump, OP1);
		select_operand(other, OP1);
		jump->op1->jptr = label;
		label->op1->b = NULL;
		select_operand(jump, OP1);
		select_operand(other, OP1);
		label->op1 = NULL;
		select_operand(jump, OP1);
		select_operand(other, OP1);
		reset_fixture();
	}
}

static void test_click_away(void)
{
	int instruction_ids[] = {MOV, MOV, JMP, MOV};
	for (int index = 0; index < 4; index++) {
		code_line_t *line = add_line(instruction_ids[index], true, index == 0 || index == 3);
		code_line_t *label = add_line(LABEL, true, false);
		if (index == 2) line->op1->jptr = label;
		operand_t *original = line->op1;
		operand_t *original_second = line->op2;
		select_operand(line, index == 3 ? OP2 : OP1);
		release_at(0, 0);
		cw_change_clicked_code_line_state();
		assert(line->state == (index == 3 ? CHANGING_OP2 : CHANGING_OP1));
		assert(pending_operand_handler() == NULL);
		assert(line->op1 == original && original->id == RAX);
		assert(line->op2 == original_second);
		if (original_second != NULL) {
			assert(original_second->id == IMMUP7);
			assert_reset(original_second->b);
		}
		assert(line->state == (index == 1 ? MISSING_OP2 : COMPLETE));
		assert_reset(original->b);
		if (index == 2) {
			assert_reset(label->op1->b);
			assert(line->op1->jptr == label);
		}
		reset_fixture();
	}
}

static void test_pending_lookup_priority(void)
{
	assert(cw_get_code_line_pending_operand() == NULL);
	assert(pending_operand_handler() == NULL);
	code_line_t *complete = add_line(MOV, true, true);
	code_line_t *first_missing = add_line(MOV, true, false);
	code_line_t *second_missing = add_line(ADD, false, false);
	first_missing->state = MISSING_OP2;
	second_missing->state = MISSING_BOTH;
	assert(cw_get_code_line_pending_operand() == first_missing);
	code_line_t *changing = add_line(MOV, true, true);
	for (int position = OP1; position <= OP2; position++) {
		select_operand(changing, position);
		assert(cw_get_code_line_pending_operand() == changing);
		release_at(0, 0);
		assert(pending_operand_handler() == NULL);
		assert(cw_get_code_line_pending_operand() == first_missing);
	}
	first_missing->state = COMPLETE;
	assert(cw_get_code_line_pending_operand() == second_missing);
	second_missing->state = MISSING_OP1;
	assert(cw_get_code_line_pending_operand() == second_missing);
	second_missing->state = COMPLETE;
	assert(cw_get_code_line_pending_operand() == NULL);
	complete->state = CHANGING_OP1;
	complete->ins = NULL;
	assert(pending_operand_handler() == NULL);
	assert(complete->state == CHANGING_OP1);
	reset_fixture();
}

static void test_incomplete_transfer_click_away(void)
{
	code_line_t *incomplete = add_line(MOV, true, false);
	code_line_t *selected = add_line(ADD, true, true);
	operand_t *incomplete_original = incomplete->op1;
	operand_t *selected_original = selected->op2;
	select_operand(incomplete, OP1);
	select_operand(selected, OP2);
	assert(incomplete->state == MISSING_OP2);
	assert(selected->state == CHANGING_OP2);
	assert(cw_get_code_line_pending_operand() == selected);
	release_at(0, 0);
	assert(!cw_chk_click_code_op());
	assert(pending_operand_handler() == NULL);
	assert(incomplete->state == MISSING_OP2 && incomplete->op2 == NULL);
	assert(incomplete->op1 == incomplete_original);
	assert(selected->state == COMPLETE);
	assert(selected->op2 == selected_original && selected_original->id == IMMUP7);
	assert_reset(selected_original->b);
	assert(cw_get_code_line_pending_operand() == incomplete);
	reset_fixture();
}

static operand_t *create_test_operand(int operand_id, SDL_Rect bounds)
{
	operand_t *operand = calloc(1, sizeof(*operand));
	assert(operand != NULL);
	operand->id = operand_id;
	operand->b = calloc(1, sizeof(*operand->b));
	assert(operand->b != NULL);
	operand->b->t = calloc(1, sizeof(*operand->b->t));
	assert(operand->b->t != NULL);
	operand->b->r = bounds;
	operand->b->enabled = true;
	return operand;
}

static void test_incomplete_transfer_replacement(void)
{
	code_line_t *incomplete = add_line(MOV, true, false);
	code_line_t *selected = add_line(ADD, true, true);
	operand_t *incomplete_original = incomplete->op1;
	selected->op2 = create_test_operand(IMMUP7, selected->op2->b->r);
	select_operand(incomplete, OP1);
	select_operand(selected, OP2);
	assert(incomplete->state == MISSING_OP2);
	assert(cw_get_code_line_pending_operand() == selected);
	selected_immediate = create_test_operand(IMMUP9, (SDL_Rect){0});
	assert(cl_is_op_compatible(selected_immediate, selected));
	immediate_selected = true;
	release_at(0, 0);
	assert(pending_operand_handler() == NULL);
	assert(selected->op2 == selected_immediate && selected->op2->id == IMMUP9);
	assert(selected->state == COMPLETE);
	assert_reset(selected->op1->b);
	assert_reset(selected->op2->b);
	assert(incomplete->state == MISSING_OP2 && incomplete->op2 == NULL);
	assert(incomplete->op1 == incomplete_original);
	assert(cw_get_code_line_pending_operand() == incomplete);
	cl_destroy_operand(selected_immediate);
	reset_fixture();
}

static void test_defensive_jump_click_away(void)
{
	for (int scenario = 0; scenario < 6; scenario++) {
		code_line_t *jump = add_line(JMP, true, false);
		code_line_t *label = add_line(LABEL, true, false);
		jump->op1->jptr = label;
		if (scenario == 0) jump->op1->jptr = NULL;
		if (scenario == 1) label->op1 = NULL;
		if (scenario == 2) label->op1->b = NULL;
		if (scenario == 3) jump->op1->b = NULL;
		if (scenario == 4) jump->op1 = NULL;
		operand_t *original = jump->op1;
		operand_t *label_original = label->op1;
		code_line_t *target = original != NULL ? original->jptr : NULL;
		if (original != NULL && original->b != NULL) {
			select_operand(jump, OP1);
		}
		if (scenario == 5) assert_reset(label->op1->b);
		mark_changing_with_stale_animation(jump, CHANGING_OP1);
		if (label->op1 != NULL && label->op1->b != NULL && target != NULL) {
			label->op1->b->animated = true;
			label->op1->b->anim_dir = true;
			label->op1->b->anim_state = 8;
		}
		release_at(0, 0);
		assert(pending_operand_handler() == NULL);
		assert(jump->state == (scenario == 4 ? MISSING_OP1 : COMPLETE));
		assert(jump->op1 == original && label->op1 == label_original);
		if (original != NULL) {
			assert(original->id == RAX && original->jptr == target);
			if (original->b != NULL) assert_reset(original->b);
		}
		if (label->op1 != NULL && label->op1->b != NULL && target != NULL) {
			assert_reset(label->op1->b);
		}
		reset_fixture();
	}
	code_line_t *missing_button = add_line(MOV, true, true);
	missing_button->op2->b = NULL;
	mark_changing_with_stale_animation(missing_button, CHANGING_OP2);
	release_at(0, 0);
	assert(pending_operand_handler() == NULL);
	assert(missing_button->state == COMPLETE);
	assert_reset(missing_button->op1->b);
	reset_fixture();
}

static void test_replacement(void)
{
	for (int position = OP1; position <= OP2; position++) {
		code_line_t *line = add_line(MOV, true, true);
		operand_t *replacement = calloc(1, sizeof(*replacement));
		assert(replacement != NULL);
		replacement->id = RBX;
		replacement->b = calloc(1, sizeof(*replacement->b));
		assert(replacement->b != NULL);
		replacement->b->t = calloc(1, sizeof(*replacement->b->t));
		assert(replacement->b->t != NULL);
		if (position == OP1) {
			replacement->b->r = line->op1->b->r;
			line->op1 = replacement;
		} else {
			replacement->b->r = line->op2->b->r;
			line->op2 = replacement;
		}
		replacement->b->enabled = true;
		select_operand(line, position);
		operand_t candidate = {.id = RAX};
		assert(cl_is_op_compatible(&candidate, line));
		operand_t *new_operand = calloc(1, sizeof(*new_operand));
		assert(new_operand != NULL);
		new_operand->id = candidate.id;
		new_operand->b = calloc(1, sizeof(*new_operand->b));
		assert(new_operand->b != NULL);
		new_operand->b->t = calloc(1, sizeof(*new_operand->b->t));
		assert(new_operand->b->t != NULL);
		cw_assign_op_to_line(new_operand, line);
		assert((position == OP1 ? line->op1 : line->op2) == new_operand);
		assert(line->state == COMPLETE);
		assert_reset(line->op1->b);
		assert_reset(line->op2->b);
		assert(!cw_is_operand_pending());
		cl_destroy_operand(new_operand);
		reset_fixture();
	}
}

int main(void)
{
	cw_create_code_list();
	ms_init_mouse();
	add_line(MOV, false, true);
	assert(cw_get_instruction_operand(0, FIRST_OP) == NO_OPERAND);
	assert(cw_get_instruction_operand(0, SECOND_OP) == IMMUP7);
	add_line(JMP, false, false);
	assert(cw_get_instruction_operand(1, FIRST_OP) == NO_OPERAND);
	assert(cw_get_instruction_operand(1, SECOND_OP) == NO_OPERAND);
	reset_fixture();
	test_transfer_and_reselection();
	test_structural_restoration();
	test_jumps_and_labels();
	test_click_away();
	test_replacement();
	test_pending_lookup_priority();
	test_incomplete_transfer_click_away();
	test_incomplete_transfer_replacement();
	test_defensive_jump_click_away();
	List_destroy(get_code_list());
	puts("operand change selection tests passed");
	return 0;
}