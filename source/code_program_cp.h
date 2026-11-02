#ifndef CODE_PROGRAM_CP_H
#define CODE_PROGRAM_CP_H

#include <stdbool.h>

typedef struct code_line_t code_line_t;
typedef struct code_program_t code_program_t;
typedef struct cp_iterator_t {
	int next_index;
} cp_iterator_t;
typedef void (*cp_line_destroy_fn)(code_line_t *line);

typedef enum cp_result_t {
	CP_OK,
	CP_INVALID_ARGUMENT,
	CP_OUT_OF_RANGE,
	CP_DUPLICATE_LINE,
	CP_ALLOCATION_FAILED
} cp_result_t;

/* create transfers no line ownership; destroy owns and destroys inserted lines. */
code_program_t *cp_create(cp_line_destroy_fn destroy_line);
void cp_destroy(code_program_t *program);
void cp_clear(code_program_t *program);
int cp_count(const code_program_t *program);
/* get returns a borrowed pointer, invalidated by remove, clear, or destroy. */
code_line_t *cp_get(const code_program_t *program, int index);
/* Iteration returns borrowed lines in order; initialize with {0}. */
code_line_t *cp_iterator_next(const code_program_t *program,
							  cp_iterator_t *iterator);
int cp_index_of(const code_program_t *program, const code_line_t *line);
/* insert transfers line ownership only on CP_OK. */
cp_result_t cp_insert(code_program_t *program, int index, code_line_t *line);
/* remove transfers ownership to the caller through line. */
cp_result_t cp_remove(code_program_t *program, int index, code_line_t **line);
cp_result_t cp_move(code_program_t *program, int from, int to);
bool cp_contains(const code_program_t *program, const code_line_t *line);

#endif
