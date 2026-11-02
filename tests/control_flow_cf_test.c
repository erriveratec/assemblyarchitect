#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "control_flow_cf.h"

static void free_line(code_line_t *line)
{
	if (line == NULL) {
		return;
	}
	free(line->ins);
	free(line->op1);
	free(line->op2);
	free(line);
}

static code_line_t *new_line(int instruction, uint64_t identity)
{
	code_line_t *line = calloc(1, sizeof(*line));
	assert(line != NULL);
	line->ins = calloc(1, sizeof(*line->ins));
	assert(line->ins != NULL);
	line->ins->id = instruction;
	line->label_identity = identity;
	return line;
}

static code_line_t *new_jump(uint64_t identity)
{
	code_line_t *jump = new_line(JMP, 0);
	jump->op1 = calloc(1, sizeof(*jump->op1));
	assert(jump->op1 != NULL);
	jump->op1->target_label_identity = identity;
	return jump;
}

static void test_v1_translation_and_repair(void)
{
	code_program_t *program = cp_create(free_line);
	assert(program != NULL);
	code_line_t *backward_label = new_line(LABEL, 0);
	code_line_t *forward_jump = new_jump(0);
	code_line_t *middle = new_line(CMP, 0);
	code_line_t *forward_label = new_line(LABEL, 0);
	code_line_t *backward_jump = new_jump(0);
	forward_jump->op1->id = 3;
	backward_jump->op1->id = 0;
	assert(cp_insert(program, 0, backward_label) == CP_OK);
	assert(cp_insert(program, 1, forward_jump) == CP_OK);
	assert(cp_insert(program, 2, middle) == CP_OK);
	assert(cp_insert(program, 3, forward_label) == CP_OK);
	assert(cp_insert(program, 4, backward_jump) == CP_OK);
	uint64_t next_identity = 1;
	assert(cf_translate_v1_targets(program, &next_identity) == CF_VALID_TARGET);
	assert(forward_jump->op1->jptr == forward_label);
	assert(backward_jump->op1->jptr == backward_label);
	assert(forward_jump->op1->target_label_identity ==
	       forward_label->label_identity);
	assert(backward_jump->op1->target_label_identity ==
	       backward_label->label_identity);
	assert(cp_move(program, 3, 2) == CP_OK);
	assert(cf_repair_program_targets(program, &next_identity) == CF_VALID_TARGET);
	assert(forward_jump->op1->jptr == forward_label);
	assert(forward_jump->op1->id == 2);
	code_line_t *removed = NULL;
	assert(cp_remove(program, cp_index_of(program, backward_label), &removed) ==
	       CP_OK);
	assert(cf_repair_program_targets(program, &next_identity) == CF_MISSING_LABEL);
	assert(backward_jump->op1->jptr == NULL);
	free_line(removed);
	cp_destroy(program);
}

static void test_jump_execution_state_ranges(void)
{
	code_program_t *program = cp_create(free_line);
	assert(program != NULL);
	code_line_t *label = new_line(LABEL, 7);
	code_line_t *middle = new_line(CMP, 0);
	code_line_t *backward_jump = new_jump(7);
	assert(cp_insert(program, 0, label) == CP_OK);
	assert(cp_insert(program, 1, middle) == CP_OK);
	assert(cp_insert(program, 2, backward_jump) == CP_OK);
	label->state = EXECUTED;
	middle->state = EXECUTED;
	backward_jump->state = EXECUTED;
	assert(cf_apply_jump_execution(program, backward_jump) == CF_VALID_TARGET);
	assert(label->state == COMPLETE);
	assert(middle->state == COMPLETE);
	assert(backward_jump->state == COMPLETE);
	cp_destroy(program);
}

int main(void)
{
	test_v1_translation_and_repair();
	test_jump_execution_state_ranges();
	code_program_t *program = cp_create(free_line);
	assert(program != NULL);
	code_line_t *jump = new_jump(42);
	code_line_t *shared_jump = new_jump(42);
	code_line_t *label = new_line(LABEL, 42);
	assert(cp_insert(program, 0, jump) == CP_OK);
	assert(cp_insert(program, 1, shared_jump) == CP_OK);
	assert(cp_insert(program, 2, label) == CP_OK);
	code_line_t *target = NULL;
	assert(cf_resolve_jump(program, jump, &target) == CF_VALID_TARGET);
	assert(target == label);
	assert(cf_validate_program(program) == CF_VALID_TARGET);
	code_program_t *identity_program = cp_create(free_line);
	assert(identity_program != NULL);
	code_line_t *first_label = new_line(LABEL, 0);
	code_line_t *second_label = new_line(LABEL, 0);
	uint64_t next_identity = 1;
	assert(cp_insert(identity_program, 0, first_label) == CP_OK);
	assert(cp_insert(identity_program, 1, second_label) == CP_OK);
	assert(cf_assign_label_identities(identity_program, &next_identity) ==
	       CF_VALID_TARGET);
	assert(first_label->label_identity == 1);
	assert(second_label->label_identity == 2);
	assert(cp_move(identity_program, 0, 1) == CP_OK);
	assert(cf_assign_label_identities(identity_program, &next_identity) ==
	       CF_VALID_TARGET);
	assert(first_label->label_identity == 1);
	assert(second_label->label_identity == 2);
	cp_destroy(identity_program);
	code_line_t *removed_label = NULL;
	assert(cp_remove(program, 2, &removed_label) == CP_OK);
	assert(removed_label == label);
	assert(cf_resolve_jump(program, jump, NULL) == CF_MISSING_LABEL);
	assert(cp_insert(program, 2, removed_label) == CP_OK);

	code_line_t *unresolved = new_jump(0);
	assert(cf_resolve_jump(program, unresolved, NULL) == CF_UNRESOLVED_TARGET);
	unresolved->op1->target_label_identity = 99;
	assert(cf_resolve_jump(program, unresolved, NULL) == CF_MISSING_LABEL);
	code_line_t *bad = new_line(JMP, 0);
	assert(cf_resolve_jump(program, bad, NULL) == CF_INVALID_JUMP_OPERAND);
	free_line(bad);

	code_line_t *duplicate = new_line(LABEL, 42);
	assert(cp_insert(program, 3, duplicate) == CP_OK);
	assert(cf_resolve_jump(program, jump, NULL) == CF_DUPLICATE_IDENTITY);
	free_line(unresolved);
	cp_destroy(program);
	puts("control-flow tests passed");
	return 0;
}
