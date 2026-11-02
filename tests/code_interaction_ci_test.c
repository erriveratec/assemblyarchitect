#include <assert.h>
#include <stdio.h>

#include "code_interaction_ci.h"

int main(void)
{
	ci_hit_results_t hits = {0};
	assert(ci_translate_hits(NULL) == CI_ACTION_NONE);
	assert(ci_translate_hits(&hits) == CI_ACTION_NONE);
	hits.operand = true;
	hits.instruction = true;
	assert(ci_translate_hits(&hits) == CI_ACTION_SELECT_OPERAND);
	hits.operand = false;
	assert(ci_translate_hits(&hits) == CI_ACTION_SELECT_INSTRUCTION);
	hits.instruction = false;
	hits.code_line = true;
	hits.right_clicked_code_line = true;
	assert(ci_translate_hits(&hits) == CI_ACTION_SELECT_CODE_LINE);
	hits.code_line = false;
	assert(ci_translate_hits(&hits) == CI_ACTION_CLONE_CODE_LINE);
	hits.right_clicked_code_line = false;
	hits.has_held_line = true;
	hits.left_pressed = true;
	assert(ci_translate_hits(&hits) == CI_ACTION_UPDATE_HELD_LINE);
	hits.left_pressed = false;
	assert(ci_translate_hits(&hits) == CI_ACTION_RELEASE_HELD_LINE);
	hits.has_held_line = false;
	assert(ci_translate_hits(&hits) == CI_ACTION_NONE);
	puts("code interaction tests passed");
	return 0;
}
