#ifndef CODE_WINDOW_CW_INTERNAL_H
#define CODE_WINDOW_CW_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>

#include "code_line_cl.h"

typedef struct cw_control_flow_repair_plan cw_control_flow_repair_plan_t;

typedef enum cw_repair_result {
	CW_REPAIR_OK = 0,
	CW_REPAIR_INVALID_ARGUMENT,
	CW_REPAIR_INVALID_TARGET,
	CW_REPAIR_TARGET_NOT_LABEL,
	CW_REPAIR_ALLOCATION_FAILED,
	CW_REPAIR_TEXTURE_FAILED
} cw_repair_result_t;

typedef enum cw_repair_test_stage {
	CW_REPAIR_TEST_BEFORE_ENTRY,
	CW_REPAIR_TEST_TEXTURE,
	CW_REPAIR_TEST_BUTTON,
	CW_REPAIR_TEST_OPERAND
} cw_repair_test_stage_t;

cw_repair_result_t cw_prepare_control_flow_repair(
	code_line_t *const *order,
	size_t count,
	bool reconstruct_saved_targets,
	cw_control_flow_repair_plan_t **plan_out);
void cw_commit_control_flow_repair(cw_control_flow_repair_plan_t *plan);
void cw_discard_control_flow_repair(cw_control_flow_repair_plan_t *plan);

#ifdef CW_REPAIR_TESTING
void cw_repair_fail_for_test(cw_repair_test_stage_t stage,
							 int opcode,
							 size_t matching_entries_to_skip);
#endif

#endif