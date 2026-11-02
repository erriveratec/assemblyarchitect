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
