#ifndef CONTROL_FLOW_CF_H
#define CONTROL_FLOW_CF_H

#include "code_line_cl.h"
#include "code_program_cp.h"

typedef enum cf_result_t {
	CF_VALID_TARGET,
	CF_UNRESOLVED_TARGET,
	CF_MISSING_LABEL,
	CF_DUPLICATE_IDENTITY,
	CF_INVALID_JUMP_OPERAND,
	CF_INVALID_LABEL,
	CF_IDENTITY_EXHAUSTED
} cf_result_t;

cf_result_t cf_ensure_label_identity(
	code_program_t *program, code_line_t *label, uint64_t *next_identity);
cf_result_t cf_assign_label_identities(
	code_program_t *program, uint64_t *next_identity);
cf_result_t cf_translate_v1_targets(
	code_program_t *program, uint64_t *next_identity);
cf_result_t cf_repair_program_targets(
	code_program_t *program, uint64_t *next_identity);

/* target is borrowed from program and is set only for CF_VALID_TARGET. */
cf_result_t cf_resolve_jump(
	const code_program_t *program, const code_line_t *jump,
	code_line_t **target);
cf_result_t cf_validate_program(const code_program_t *program);
void cf_reset_execution(code_program_t *program);
cf_result_t cf_apply_jump_execution(
	const code_program_t *program, code_line_t *jump);

#endif
