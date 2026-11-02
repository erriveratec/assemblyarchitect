#include "code_interaction_ci.h"

#include <stddef.h>

ci_action_t ci_translate_hits(const ci_hit_results_t *hits)
{
	if (hits == NULL) {
		return CI_ACTION_NONE;
	}
	if (hits->operand) {
		return CI_ACTION_SELECT_OPERAND;
	}
	if (hits->instruction) {
		return CI_ACTION_SELECT_INSTRUCTION;
	}
	if (hits->code_line) {
		return CI_ACTION_SELECT_CODE_LINE;
	}
	if (hits->right_clicked_code_line) {
		return CI_ACTION_CLONE_CODE_LINE;
	}
	if (hits->left_pressed && hits->has_held_line) {
		return CI_ACTION_UPDATE_HELD_LINE;
	}
	if (!hits->left_pressed && hits->has_held_line) {
		return CI_ACTION_RELEASE_HELD_LINE;
	}
	return CI_ACTION_NONE;
}
