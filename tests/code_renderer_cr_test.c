#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "code_renderer_cr.h"
#include "code_line_cl.h"

static void destroy_line(code_line_t *line)
{
	free(line->ins);
	free(line->op1);
	free(line->op2);
	free(line);
}

int main(void)
{
	code_program_t *program = cp_create(destroy_line);
	assert(program != NULL);
	code_line_t *line = calloc(1, sizeof(*line));
	assert(line != NULL);
	line->ins = calloc(1, sizeof(*line->ins));
	assert(line->ins != NULL);
	line->ins->id = LABEL;
	line->state = COMPLETE;
	line->label_identity = 37;
	assert(cp_insert(program, 0, line) == CP_OK);
	code_line_t *saved_line = cp_get(program, 0);
	int saved_state = line->state;
	uint64_t saved_identity = line->label_identity;
	cr_assets_t assets = {0};
	cr_visual_state_t visual_state = {0};
	cr_draw_code_lines(program, &assets, NULL, &visual_state, 0, false);
	cr_draw_line_numbers(program, &assets, 0, 0, 1, 1);
	assert(cp_get(program, 0) == saved_line);
	assert(line->state == saved_state);
	assert(line->label_identity == saved_identity);
	cp_destroy(program);
	puts("code renderer non-mutation tests passed");
	return 0;
}
