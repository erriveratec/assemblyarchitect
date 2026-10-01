#ifndef RUN_RESULT_RR_H
#define RUN_RESULT_RR_H

#include <stdbool.h>

typedef enum run_result_action_t {
	RUN_RESULT_ACTION_NONE = 0,
	RUN_RESULT_ACTION_BACK,
	RUN_RESULT_ACTION_CONTINUE
} run_result_action_t;

bool rr_initialize(void);
void rr_destroy(void);

run_result_action_t rr_update(int operation_id);
void rr_render(int operation_id);

void rr_reset_state(void);

#endif