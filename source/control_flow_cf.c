#include "control_flow_cf.h"

#include <stdint.h>

static bool cf_is_jump_instruction(int instruction_id)
{
	return instruction_id == JMP || instruction_id == JE || instruction_id == JNE;
}

cf_result_t cf_ensure_label_identity(
	code_program_t *program, code_line_t *label, uint64_t *next_identity)
{
	if (program == NULL || label == NULL || next_identity == NULL ||
	    label->ins == NULL || label->ins->id != LABEL ||
	    !cp_contains(program, label)) {
		return CF_INVALID_LABEL;
	}
	for (int index = 0; index < cp_count(program); index++) {
		code_line_t *candidate = cp_get(program, index);
		if (candidate == label || candidate == NULL || candidate->ins == NULL ||
		    candidate->ins->id != LABEL || candidate->label_identity == 0) {
			continue;
		}
		if (label->label_identity != 0 &&
		    candidate->label_identity == label->label_identity) {
			return CF_DUPLICATE_IDENTITY;
		}
	}
	if (label->label_identity != 0) {
		if (*next_identity == 0 || *next_identity <= label->label_identity) {
			*next_identity = label->label_identity == UINT64_MAX
				? UINT64_MAX : label->label_identity + 1;
		}
		return CF_VALID_TARGET;
	}
	if (*next_identity == 0) {
		*next_identity = 1;
	}
	while (*next_identity < UINT64_MAX) {
		bool in_use = false;
		for (int index = 0; index < cp_count(program); index++) {
			code_line_t *candidate = cp_get(program, index);
			if (candidate != NULL && candidate != label &&
			    candidate->ins != NULL && candidate->ins->id == LABEL &&
			    candidate->label_identity == *next_identity) {
				in_use = true;
				break;
			}
		}
		if (!in_use) {
			label->label_identity = *next_identity;
			(*next_identity)++;
			return CF_VALID_TARGET;
		}
		(*next_identity)++;
	}
	return CF_IDENTITY_EXHAUSTED;
}

cf_result_t cf_assign_label_identities(
	code_program_t *program, uint64_t *next_identity)
{
	if (program == NULL || next_identity == NULL) {
		return CF_INVALID_LABEL;
	}
	for (int index = 0; index < cp_count(program); index++) {
		code_line_t *line = cp_get(program, index);
		if (line != NULL && line->ins != NULL && line->ins->id == LABEL) {
			cf_result_t result = cf_ensure_label_identity(
				program, line, next_identity);
			if (result != CF_VALID_TARGET) {
				return result;
			}
		}
	}
	return CF_VALID_TARGET;
}

cf_result_t cf_translate_v1_targets(
	code_program_t *program, uint64_t *next_identity)
{
	if (program == NULL || next_identity == NULL) {
		return CF_INVALID_LABEL;
	}
	for (int index = 0; index < cp_count(program); index++) {
		code_line_t *jump = cp_get(program, index);
		if (jump == NULL || jump->ins == NULL ||
		    !cf_is_jump_instruction(jump->ins->id)) {
			continue;
		}
		if (jump->op1 == NULL) {
			return CF_INVALID_JUMP_OPERAND;
		}
		code_line_t *target = cp_get(program, jump->op1->id);
		if (target == NULL || target->ins == NULL || target->ins->id != LABEL) {
			return CF_MISSING_LABEL;
		}
	}
	cf_result_t assignment = cf_assign_label_identities(program, next_identity);
	if (assignment != CF_VALID_TARGET) {
		return assignment;
	}
	for (int index = 0; index < cp_count(program); index++) {
		code_line_t *jump = cp_get(program, index);
		if (jump == NULL || jump->ins == NULL ||
		    !cf_is_jump_instruction(jump->ins->id)) {
			continue;
		}
		code_line_t *target = cp_get(program, jump->op1->id);
		jump->op1->jptr = target;
		jump->op1->target_label_identity = target->label_identity;
	}
	return CF_VALID_TARGET;
}

cf_result_t cf_resolve_jump(
	const code_program_t *program, const code_line_t *jump,
	code_line_t **target)
{
	if (target != NULL) {
		*target = NULL;
	}
	if (program == NULL || jump == NULL || jump->ins == NULL ||
	    !cf_is_jump_instruction(jump->ins->id) || jump->op1 == NULL) {
		return CF_INVALID_JUMP_OPERAND;
	}
	uint64_t identity = jump->op1->target_label_identity;
	if (identity == 0) {
		return CF_UNRESOLVED_TARGET;
	}
	code_line_t *match = NULL;
	unsigned int matches = 0;
	for (int index = 0; index < cp_count(program); index++) {
		code_line_t *candidate = cp_get(program, index);
		if (candidate != NULL && candidate->ins != NULL &&
		    candidate->ins->id == LABEL &&
		    candidate->label_identity == identity) {
			match = candidate;
			matches++;
		}
	}
	if (matches > 1) {
		return CF_DUPLICATE_IDENTITY;
	}
	if (matches == 0) {
		return CF_MISSING_LABEL;
	}
	if (target != NULL) {
		*target = match;
	}
	return CF_VALID_TARGET;
}

cf_result_t cf_repair_program_targets(
	code_program_t *program, uint64_t *next_identity)
{
	if (program == NULL || next_identity == NULL) {
		return CF_INVALID_LABEL;
	}
	cf_result_t result = cf_assign_label_identities(program, next_identity);
	if (result != CF_VALID_TARGET) {
		return result;
	}
	cf_result_t first_failure = CF_VALID_TARGET;
	for (int index = 0; index < cp_count(program); index++) {
		code_line_t *jump = cp_get(program, index);
		if (jump == NULL || jump->ins == NULL ||
		    !cf_is_jump_instruction(jump->ins->id) || jump->op1 == NULL) {
			continue;
		}
		if (jump->op1->target_label_identity == 0) {
			if (jump->op1->jptr == NULL ||
			    !cp_contains(program, jump->op1->jptr) ||
			    jump->op1->jptr->ins == NULL ||
			    jump->op1->jptr->ins->id != LABEL) {
				jump->op1->jptr = NULL;
				if (first_failure == CF_VALID_TARGET) {
					first_failure = CF_MISSING_LABEL;
				}
				continue;
			}
			jump->op1->target_label_identity =
				jump->op1->jptr->label_identity;
		}
		code_line_t *target = NULL;
		cf_result_t resolved = cf_resolve_jump(program, jump, &target);
		if (resolved != CF_VALID_TARGET) {
			jump->op1->jptr = NULL;
			if (first_failure == CF_VALID_TARGET) {
				first_failure = resolved;
			}
			continue;
		}
		jump->op1->jptr = target;
		jump->op1->id = cp_index_of(program, target);
	}
	return first_failure;
}

cf_result_t cf_validate_program(const code_program_t *program)
{
	if (program == NULL) {
		return CF_INVALID_JUMP_OPERAND;
	}
	for (int index = 0; index < cp_count(program); index++) {
		code_line_t *line = cp_get(program, index);
		if (line != NULL && line->ins != NULL &&
		    cf_is_jump_instruction(line->ins->id)) {
			cf_result_t result = cf_resolve_jump(program, line, NULL);
			if (result != CF_VALID_TARGET) {
				return result;
			}
		}
	}
	return CF_VALID_TARGET;
}

void cf_reset_execution(code_program_t *program)
{
	if (program == NULL) {
		return;
	}
	for (int index = 0; index < cp_count(program); index++) {
		code_line_t *line = cp_get(program, index);
		line->state = COMPLETE;
	}
}

cf_result_t cf_apply_jump_execution(
	const code_program_t *program, code_line_t *jump)
{
	code_line_t *target = NULL;
	cf_result_t result = cf_resolve_jump(program, jump, &target);
	if (result != CF_VALID_TARGET) {
		return result;
	}
	int jump_position = cp_index_of(program, jump);
	int target_position = cp_index_of(program, target);
	if (jump_position < 0 || target_position < 0) {
		return CF_MISSING_LABEL;
	}
	for (int index = 0; index < cp_count(program); index++) {
		code_line_t *line = cp_get(program, index);
		if (target_position < jump_position && index >= target_position &&
		    index <= jump_position) {
			line->state = COMPLETE;
		} else if (target_position > jump_position && index < target_position &&
		           index >= jump_position) {
			line->state = EXECUTED;
		}
	}
	return CF_VALID_TARGET;
}
