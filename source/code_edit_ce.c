#include "code_edit_ce.h"

#include "code_line_cl.h"

static int ce_line_state_after_edit(const code_line_t *line)
{
	int quantity = cl_get_instruction_operand_quantity(line->ins->id);
	if (quantity == TWO_OPERANDS && line->op1 == NULL && line->op2 == NULL) {
		return MISSING_BOTH;
	}
	if (quantity != ZERO_OPERANDS && line->op1 == NULL) {
		return MISSING_OP1;
	}
	if (quantity == TWO_OPERANDS && line->op2 == NULL) {
		return MISSING_OP2;
	}
	return COMPLETE;
}

static void ce_discard_invalid_instruction(
	instruction_t *instruction, const instruction_t *source)
{
	if (instruction == NULL || instruction == source) {
		return;
	}
	if (instruction->b == NULL) {
		free(instruction);
	} else if (instruction->b == source->b) {
		free(instruction);
	} else if (instruction->b->t == NULL) {
		free(instruction->b);
		free(instruction);
	} else if (instruction->b->t == source->b->t) {
		free(instruction->b);
		free(instruction);
	} else {
		cl_destroy_instruction(instruction);
	}
}

static void ce_discard_invalid_operand(
	operand_t *operand, const operand_t *source)
{
	if (operand == NULL || operand == source) {
		return;
	}
	if (operand->b == NULL) {
		free(operand);
	} else if (operand->b == source->b) {
		free(operand);
	} else if (operand->b->t == NULL) {
		free(operand->b);
		free(operand);
	} else if (operand->b->t == source->b->t) {
		free(operand->b);
		free(operand);
	} else {
		cl_destroy_operand(operand);
	}
}

ce_result_t ce_clone_line(
	const code_line_t *source,
	ce_clone_instruction_factory_fn create_instruction,
	ce_clone_operand_factory_fn create_operand,
	void *context,
	code_line_t **clone)
{
	if (clone != NULL) {
		*clone = NULL;
	}
	if (source == NULL || source->ins == NULL || source->ins->b == NULL ||
	    source->ins->b->t == NULL ||
	    create_instruction == NULL || create_operand == NULL || clone == NULL ||
	    source->ins->id <= INSTRUCTION_MIN ||
	    source->ins->id >= INSTRUCTION_MAX || source->ins->id == LABEL) {
		return CE_INVALID_ARGUMENT;
	}
	int quantity = cl_get_instruction_operand_quantity(source->ins->id);
	if (quantity == ZERO_OPERANDS &&
	    (source->op1 != NULL || source->op2 != NULL)) {
		return CE_INVALID_ARGUMENT;
	}
	if (quantity == ONE_OPERAND && source->op2 != NULL) {
		return CE_INVALID_ARGUMENT;
	}
	if (quantity == TWO_OPERANDS && cl_is_ins_jmp_type(source->ins->id)) {
		return CE_INVALID_ARGUMENT;
	}
	if (quantity < ZERO_OPERANDS || quantity > TWO_OPERANDS) {
		return CE_INVALID_ARGUMENT;
	}
	bool is_jump = cl_is_ins_jmp_type(source->ins->id);
	if (is_jump && source->op1 != NULL &&
	    (source->op1->b == NULL || source->op1->jptr == NULL ||
	     source->op1->jptr->ins == NULL ||
	     source->op1->jptr->ins->id != LABEL ||
	     source->op1->jptr->label_identity == 0 ||
	     source->op1->target_label_identity == 0 ||
	     source->op1->target_label_identity !=
	         source->op1->jptr->label_identity)) {
		return CE_INVALID_ARGUMENT;
	}

	instruction_t *instruction = create_instruction(source->ins, context);
	if (instruction == NULL) {
		return CE_FACTORY_FAILED;
	}
	if (instruction->id != source->ins->id || instruction->b == NULL ||
	    instruction->b->t == NULL ||
	    instruction->b == source->ins->b ||
	    instruction->b->t == source->ins->b->t) {
		ce_discard_invalid_instruction(instruction, source->ins);
		return CE_FACTORY_FAILED;
	}
	code_line_t *copy = cl_create_code_line(instruction);
	if (copy == NULL) {
		cl_destroy_instruction(instruction);
		return CE_ALLOCATION_FAILED;
	}
	copy->label_identity = 0;
	operand_t *source_operands[] = {source->op1, source->op2};
	operand_t **copy_operands[] = {&copy->op1, &copy->op2};
	int slot_count = is_jump ? 0 : quantity == TWO_OPERANDS ? 2 :
		quantity == ONE_OPERAND ? 1 : 0;
	for (int slot = 0; slot < slot_count; slot++) {
		const operand_t *source_operand = source_operands[slot];
		if (source_operand == NULL) {
			continue;
		}
		if (source_operand->b == NULL || source_operand->b->t == NULL ||
		    (!cl_is_ins_jmp_type(source->ins->id) &&
		     !cl_is_operand_id_valid(source_operand->id))) {
			cl_destroy_code_line(copy);
			return CE_INVALID_ARGUMENT;
		}
		operand_t *operand = create_operand(source_operand, is_jump, context);
		if (operand == NULL) {
			cl_destroy_code_line(copy);
			return CE_FACTORY_FAILED;
		}
		if (operand == source_operand || operand->b == NULL ||
		    operand->b->t == NULL ||
		    operand->b == source_operand->b ||
		    operand->b->t == source_operand->b->t) {
			ce_discard_invalid_operand(operand, source_operand);
			cl_destroy_code_line(copy);
			return CE_FACTORY_FAILED;
		}
		operand->id = source_operand->id;
		operand->jptr = is_jump ? source_operand->jptr : NULL;
		operand->target_label_identity = is_jump
			? source_operand->target_label_identity : 0;
		operand->b->r = source_operand->b->r;
		*copy_operands[slot] = operand;
	}
	copy->state = is_jump ? MISSING_OP1 : ce_line_state_after_edit(copy);
	*clone = copy;
	return CE_OK;
}

ce_result_t ce_select_operand(
	code_program_t *program, ce_session_t *session, code_line_t *line,
	ce_operand_slot_t slot)
{
	if (program == NULL || session == NULL || line == NULL ||
	    line->ins == NULL || !cp_contains(program, line) ||
	    line->ins->id == LABEL ||
	    (slot != CE_OPERAND_SLOT_1 && slot != CE_OPERAND_SLOT_2) ||
	    (slot == CE_OPERAND_SLOT_1 && line->op1 == NULL) ||
	    (slot == CE_OPERAND_SLOT_2 &&
	     (line->op2 == NULL || cl_is_ins_jmp_type(line->ins->id)))) {
		return CE_INVALID_ARGUMENT;
	}
	for (int index = 0; index < cp_count(program); index++) {
		code_line_t *candidate = cp_get(program, index);
		if (candidate != NULL && candidate->ins != NULL &&
		    (candidate->state == CHANGING_OP1 ||
		     candidate->state == CHANGING_OP2)) {
			candidate->state = ce_line_state_after_edit(candidate);
		}
	}
	line->state = slot == CE_OPERAND_SLOT_1 ? CHANGING_OP1 : CHANGING_OP2;
	return ce_begin(session, line, slot == CE_OPERAND_SLOT_1
		? CE_REPLACING_OPERAND_1 : CE_REPLACING_OPERAND_2);
}

ce_result_t ce_begin(
	ce_session_t *session, code_line_t *line, ce_session_state_t state)
{
	if (session == NULL || line == NULL || line->ins == NULL ||
	    state <= CE_IDLE || state > CE_REPLACING_OPERAND_2) {
		return CE_INVALID_ARGUMENT;
	}
	session->line = line;
	session->state = state;
	return CE_OK;
}

ce_result_t ce_cancel(ce_session_t *session)
{
	if (session == NULL) {
		return CE_INVALID_ARGUMENT;
	}
	if (session->state == CE_IDLE || session->line == NULL) {
		return CE_INVALID_SESSION;
	}
	if (session->state == CE_REPLACING_OPERAND_1 ||
	    session->state == CE_REPLACING_OPERAND_2) {
		session->line->state = ce_line_state_after_edit(session->line);
	}
	session->state = CE_IDLE;
	session->line = NULL;
	return CE_OK;
}

ce_result_t ce_replace_operand(
	ce_session_t *session, operand_t *operand, operand_t **replaced)
{
	if (session == NULL || operand == NULL || replaced == NULL) {
		return CE_INVALID_ARGUMENT;
	}
	*replaced = NULL;
	code_line_t *line = session->line;
	if (line == NULL || line->ins == NULL ||
	    (session->state != CE_REPLACING_OPERAND_1 &&
	     session->state != CE_REPLACING_OPERAND_2)) {
		return CE_INVALID_SESSION;
	}
	bool replacing_first = session->state == CE_REPLACING_OPERAND_1;
	if (replacing_first && line->state != MISSING_BOTH &&
	    line->state != MISSING_OP1 && line->state != CHANGING_OP1) {
		return CE_INVALID_SESSION;
	}
	if (!replacing_first && line->state != MISSING_OP2 &&
	    line->state != CHANGING_OP2) {
		return CE_INVALID_SESSION;
	}
	if (cl_is_ins_jmp_type(line->ins->id)) {
		if (!replacing_first || operand->jptr == NULL ||
		    operand->jptr->ins == NULL || operand->jptr->ins->id != LABEL ||
		    operand->jptr->label_identity == 0 ||
		    (operand->target_label_identity != 0 &&
		     operand->target_label_identity != operand->jptr->label_identity)) {
			return CE_INCOMPATIBLE_OPERAND;
		}
		operand->target_label_identity = operand->jptr->label_identity;
	} else if (!cl_is_operand_id_valid(operand->id) ||
	           !cl_is_op_compatible(operand, line)) {
		return CE_INCOMPATIBLE_OPERAND;
	}
	if (replacing_first) {
		*replaced = line->op1;
		line->op1 = operand;
	} else {
		*replaced = line->op2;
		line->op2 = operand;
	}
	line->state = ce_line_state_after_edit(line);
	session->state = CE_IDLE;
	session->line = NULL;
	return CE_OK;
}

ce_result_t ce_insert_line(
	code_program_t *program, int index, code_line_t *line)
{
	return cp_insert(program, index, line) == CP_OK
		? CE_OK : CE_PROGRAM_ERROR;
}

ce_result_t ce_remove_line(
	code_program_t *program, int index, code_line_t **line)
{
	return cp_remove(program, index, line) == CP_OK
		? CE_OK : CE_PROGRAM_ERROR;
}

ce_result_t ce_move_line(
	code_program_t *program, code_line_t *line, int destination)
{
	int source = cp_index_of(program, line);
	if (source < 0) {
		return CE_INVALID_ARGUMENT;
	}
	return cp_move(program, source, destination) == CP_OK
		? CE_OK : CE_PROGRAM_ERROR;
}

bool ce_has_pending_operand(
	const code_program_t *program, ce_pending_query_t query)
{
	if (query < CE_PENDING_ANY || query > CE_PENDING_OPERAND_2) {
		return false;
	}
	for (int index = 0; index < cp_count(program); index++) {
		code_line_t *line = cp_get(program, index);
		int state = line->state;
		if (query == CE_PENDING_ANY &&
		    (state == MISSING_BOTH || state == MISSING_OP1 ||
		     state == MISSING_OP2 || state == CHANGING_OP1 ||
		     state == CHANGING_OP2)) {
			return true;
		}
		if (query == CE_PENDING_OPERAND_1 &&
		    (state == MISSING_BOTH || state == MISSING_OP1 ||
		     state == CHANGING_OP1)) {
			return true;
		}
		if (query == CE_PENDING_OPERAND_2 &&
		    (state == MISSING_BOTH || state == MISSING_OP2 ||
		     state == CHANGING_OP2)) {
			return true;
		}
	}
	return false;
}
