#ifndef CODE_EDIT_CE_H
#define CODE_EDIT_CE_H

#include "code_program_cp.h"
#include "code_line_cl.h"

typedef enum ce_session_state_t {
	CE_IDLE,
	CE_MOVING_LINE,
	CE_REPLACING_OPERAND_1,
	CE_REPLACING_OPERAND_2
} ce_session_state_t;

typedef struct ce_session_t {
	ce_session_state_t state;
	code_line_t *line;
} ce_session_t;

typedef enum ce_pending_query_t {
	CE_PENDING_ANY,
	CE_PENDING_OPERAND_1,
	CE_PENDING_OPERAND_2
} ce_pending_query_t;

typedef enum ce_result_t {
	CE_OK,
	CE_INVALID_ARGUMENT,
	CE_INVALID_SESSION,
	CE_INCOMPATIBLE_OPERAND,
	CE_PROGRAM_ERROR
} ce_result_t;

ce_result_t ce_begin(
	ce_session_t *session, code_line_t *line, ce_session_state_t state);
ce_result_t ce_cancel(ce_session_t *session);
/* On CE_OK the line owns operand; the caller owns non-NULL *replaced and must
 * destroy it. *replaced is NULL for an empty slot. On failure the caller retains
 * ownership of operand. Jump targets require a nonzero label identity; a zero
 * operand identity is filled from that label on success. The caller validates
 * owning-program membership because the session has no program reference. */
ce_result_t ce_replace_operand(
	ce_session_t *session, operand_t *operand, operand_t **replaced);
ce_result_t ce_insert_line(
	code_program_t *program, int index, code_line_t *line);
ce_result_t ce_remove_line(
	code_program_t *program, int index, code_line_t **line);
ce_result_t ce_move_line(
	code_program_t *program, code_line_t *line, int destination);
bool ce_has_pending_operand(
	const code_program_t *program, ce_pending_query_t query);

#endif
