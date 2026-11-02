#ifndef CODE_INTERACTION_CI_H
#define CODE_INTERACTION_CI_H

#include <stdbool.h>

typedef enum ci_action_t {
	CI_ACTION_NONE,
	CI_ACTION_SELECT_OPERAND,
	CI_ACTION_SELECT_INSTRUCTION,
	CI_ACTION_SELECT_CODE_LINE,
	CI_ACTION_CLONE_CODE_LINE,
	CI_ACTION_UPDATE_HELD_LINE,
	CI_ACTION_RELEASE_HELD_LINE
} ci_action_t;

typedef struct ci_hit_results_t {
	bool operand;
	bool instruction;
	bool code_line;
	bool right_clicked_code_line;
	bool left_pressed;
	bool has_held_line;
} ci_hit_results_t;

ci_action_t ci_translate_hits(const ci_hit_results_t *hits);

#endif
