#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "code_program_cp.h"

static void destroy_line(code_line_t *line)
{
	free(line);
}

static code_line_t *new_line(void)
{
	return (code_line_t *)malloc(sizeof(int));
}

int main(void)
{
	code_program_t *program = cp_create(destroy_line);
	assert(program != NULL);
	code_line_t *first = new_line();
	code_line_t *second = new_line();
	code_line_t *third = new_line();
	assert(first != NULL && second != NULL && third != NULL);
	assert(cp_insert(program, 0, first) == CP_OK);
	assert(cp_insert(program, 1, second) == CP_OK);
	assert(cp_insert(program, 1, third) == CP_OK);
	assert(cp_count(program) == 3);
	assert(cp_get(program, 0) == first);
	assert(cp_get(program, 1) == third);
	assert(cp_get(program, 2) == second);
	assert(cp_index_of(program, second) == 2);
	assert(cp_contains(program, third));
	cp_iterator_t iterator = {0};
	assert(cp_iterator_next(program, &iterator) == first);
	assert(cp_iterator_next(program, &iterator) == third);
	assert(cp_iterator_next(program, &iterator) == second);
	assert(cp_iterator_next(program, &iterator) == NULL);
	assert(cp_insert(program, 2, third) == CP_DUPLICATE_LINE);
	code_line_t *unaccepted = new_line();
	assert(unaccepted != NULL);
	assert(cp_insert(program, 4, unaccepted) == CP_OUT_OF_RANGE);
	destroy_line(unaccepted);
	assert(cp_move(program, 0, 2) == CP_OK);
	assert(cp_get(program, 0) == third);
	assert(cp_get(program, 1) == second);
	assert(cp_get(program, 2) == first);
	assert(cp_move(program, 2, 0) == CP_OK);
	assert(cp_get(program, 0) == first);
	code_line_t *removed = NULL;
	assert(cp_remove(program, 2, &removed) == CP_OK);
	assert(removed == second && !cp_contains(program, second));
	destroy_line(removed);
	assert(cp_remove(program, 0, &removed) == CP_OK);
	destroy_line(removed);
	cp_clear(program);
	assert(cp_count(program) == 0);
	cp_destroy(program);
	puts("code program tests passed");
	return 0;
}
