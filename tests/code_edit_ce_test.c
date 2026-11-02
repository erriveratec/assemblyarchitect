#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "code_edit_ce.h"
#include "code_line_cl.h"

static void destroy_line(code_line_t *line)
{
	free(line->ins);
	free(line->op1);
	free(line->op2);
	free(line);
}

static code_line_t *new_line(int instruction, int state)
{
	code_line_t *line = calloc(1, sizeof(*line));
	assert(line != NULL);
	line->ins = calloc(1, sizeof(*line->ins));
	assert(line->ins != NULL);
	line->ins->id = instruction;
	line->state = state;
	return line;
}

static operand_t *new_operand(int id)
{
	operand_t *operand = calloc(1, sizeof(*operand));
	assert(operand != NULL);
	operand->id = id;
	return operand;
}

static void test_empty_slot_ownership(void)
{
	const int states[] = {MISSING_BOTH, MISSING_OP1, MISSING_OP2};
	for (int index = 0; index < 3; index++) {
		code_line_t *line = new_line(MOV, states[index]);
		bool first_slot = states[index] != MISSING_OP2;
		if (states[index] == MISSING_OP1) line->op2 = new_operand(RBX);
		if (!first_slot) line->op1 = new_operand(RAX);
		ce_session_t session = {0};
		assert(ce_begin(&session, line, first_slot
			? CE_REPLACING_OPERAND_1 : CE_REPLACING_OPERAND_2) == CE_OK);
		operand_t *operand = new_operand(RCX);
		operand_t *replaced = operand;
		assert(ce_replace_operand(&session, operand, &replaced) == CE_OK);
		assert(replaced == NULL);
		assert((first_slot ? line->op1 : line->op2) == operand);
		assert(line->state == (states[index] == MISSING_BOTH ? MISSING_OP2 : COMPLETE));
		assert(session.state == CE_IDLE && session.line == NULL);
		destroy_line(line);
	}
}

static void test_jump_position_validation(void)
{
	const int positions[] = {0, 1, 2, IMM_MAX + 10};
	const int jumps[] = {JMP, JE, JNE};
	for (int kind = 0; kind < 3; kind++) {
		for (int index = 0; index < 4; index++) {
			assert(!cl_is_operand_id_valid(positions[index]));
			code_program_t *program = cp_create(destroy_line);
			assert(program != NULL);
			for (int padding = 0; padding < positions[index]; padding++) {
				assert(cp_insert(program, cp_count(program),
				                 new_line(MOV, COMPLETE)) == CP_OK);
			}
			code_line_t *jump = new_line(jumps[kind], MISSING_OP1);
			code_line_t *label = new_line(LABEL, COMPLETE);
			assert(cp_insert(program, cp_count(program), label) == CP_OK);
			assert(cp_insert(program, cp_count(program), jump) == CP_OK);
			label->label_identity = 1;
			assert(cp_index_of(program, label) == positions[index]);
			operand_t *operand = new_operand(cp_index_of(program, label));
			operand->jptr = label;
			ce_session_t session = {0};
			assert(ce_begin(&session, jump, CE_REPLACING_OPERAND_1) == CE_OK);
			operand_t *replaced = NULL;
			assert(ce_replace_operand(&session, operand, &replaced) == CE_OK);
			assert(replaced == NULL && jump->op1 == operand);
			assert(operand->target_label_identity == label->label_identity);
			assert(jump->state == COMPLETE && session.state == CE_IDLE);
			cp_destroy(program);
		}
	}
}

static void test_operand_selection_state(void)
{
	code_program_t *program = cp_create(destroy_line);
	assert(program != NULL);
	code_line_t *first = new_line(MOV, COMPLETE);
	code_line_t *second = new_line(ADD, COMPLETE);
	first->op1 = new_operand(RAX);
	first->op2 = new_operand(RBX);
	second->op1 = new_operand(RCX);
	second->op2 = new_operand(RDX);
	assert(cp_insert(program, 0, first) == CP_OK);
	assert(cp_insert(program, 1, second) == CP_OK);
	ce_session_t session = {0};
	assert(ce_select_operand(program, &session, first,
	                         CE_OPERAND_SLOT_1) == CE_OK);
	assert(first->state == CHANGING_OP1);
	assert(session.state == CE_REPLACING_OPERAND_1 && session.line == first);
	assert(ce_select_operand(program, &session, second,
	                         CE_OPERAND_SLOT_2) == CE_OK);
	assert(first->state == COMPLETE && second->state == CHANGING_OP2);
	assert(session.state == CE_REPLACING_OPERAND_2 && session.line == second);
	assert(ce_cancel(&session) == CE_OK);
	assert(second->state == COMPLETE);
	assert(ce_select_operand(program, &session, second,
	                         CE_OPERAND_SLOT_1) == CE_OK);
	second->op1 = NULL;
	second->state = MISSING_OP2;
	assert(ce_select_operand(program, &session, first,
	                         CE_OPERAND_SLOT_2) == CE_OK);
	assert(second->state == MISSING_OP2 && first->state == CHANGING_OP2);
	assert(ce_cancel(&session) == CE_OK);
	cp_destroy(program);
}

static void test_invalid_jump_targets(void)
{
	code_line_t *jump = new_line(JMP, MISSING_OP1);
	code_line_t *label = new_line(LABEL, COMPLETE);
	operand_t *operand = new_operand(0);
	ce_session_t session = {0};
	assert(ce_begin(&session, jump, CE_REPLACING_OPERAND_1) == CE_OK);
	for (int failure = 0; failure < 5; failure++) {
		operand->jptr = failure == 0 ? NULL : label;
		instruction_t *instruction = label->ins;
		if (failure == 1) label->ins = NULL;
		if (failure == 2) instruction->id = MOV;
		label->label_identity = failure == 3 ? 0 : 1;
		operand->target_label_identity = failure == 4 ? 2 : 0;
		operand_t *replaced = operand;
		assert(ce_replace_operand(&session, operand, &replaced) ==
		       CE_INCOMPATIBLE_OPERAND);
		assert(replaced == NULL && jump->op1 == NULL);
		assert(jump->state == MISSING_OP1);
		assert(session.state == CE_REPLACING_OPERAND_1 && session.line == jump);
		label->ins = instruction;
		instruction->id = LABEL;
	}
	free(operand);
	destroy_line(jump);
	destroy_line(label);
}

int main(void)
{
        test_jump_position_validation();
        test_invalid_jump_targets();
	test_operand_selection_state();
	test_empty_slot_ownership();
	code_program_t *program = cp_create(destroy_line);
	assert(program != NULL);
	code_line_t *first = new_line(MOV, COMPLETE);
	code_line_t *second = new_line(ADD, CHANGING_OP1);
	first->op1 = new_operand(RAX);
	first->op2 = new_operand(RBX);
	second->op1 = new_operand(RAX);
	second->op2 = new_operand(RBX);
	assert(ce_insert_line(program, 0, first) == CE_OK);
	assert(ce_insert_line(program, 1, second) == CE_OK);
	assert(ce_has_pending_operand(program, CE_PENDING_ANY));
	assert(ce_has_pending_operand(program, CE_PENDING_OPERAND_1));
	assert(!ce_has_pending_operand(program, CE_PENDING_OPERAND_2));
	second->state = CHANGING_OP2;
	assert(ce_has_pending_operand(program, CE_PENDING_OPERAND_2));
	second->state = CHANGING_OP1;

	ce_session_t session = {0};
	assert(ce_begin(&session, second, CE_REPLACING_OPERAND_1) == CE_OK);
	operand_t *incompatible = new_operand(IB);
	operand_t *replaced = NULL;
	assert(ce_replace_operand(&session, incompatible, &replaced) ==
	       CE_INCOMPATIBLE_OPERAND);
	assert(second->op1->id == RAX && second->state == CHANGING_OP1);
	assert(session.state == CE_REPLACING_OPERAND_1);
	assert(replaced == NULL);
	free(incompatible);
	operand_t *invalid = new_operand(-1);
	assert(ce_replace_operand(&session, invalid, &replaced) ==
	       CE_INCOMPATIBLE_OPERAND);
	assert(second->op1->id == RAX && second->state == CHANGING_OP1);
	free(invalid);

	operand_t *previous = second->op1;
	operand_t *replacement = new_operand(RCX);
	assert(ce_replace_operand(&session, replacement, &replaced) == CE_OK);
	assert(replaced == previous && replaced->id == RAX);
	assert(second->op1 == replacement && second->state == COMPLETE);
	free(replaced);

	assert(ce_begin(&session, second, CE_REPLACING_OPERAND_2) == CE_OK);
	second->state = CHANGING_OP2;
	assert(ce_cancel(&session) == CE_OK);
	assert(second->op2->id == RBX && second->state == COMPLETE);

	assert(ce_move_line(program, second, 0) == CE_OK);
	assert(cp_get(program, 0) == second);
	code_line_t *removed = NULL;
	assert(ce_remove_line(program, 0, &removed) == CE_OK);
	assert(removed == second && cp_count(program) == 1);
	destroy_line(removed);
	cp_destroy(program);
	puts("code edit tests passed");
	return 0;
}
