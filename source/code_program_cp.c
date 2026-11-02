#include "code_program_cp.h"

#include <stdlib.h>

#include "list.h"

struct code_program_t {
	List *lines;
	cp_line_destroy_fn destroy_line;
};

static ListNode *cp_node_at(const code_program_t *program, int index)
{
	ListNode *node = program->lines->first;
	for (int position = 0; node != NULL && position < index; position++) {
		node = node->next;
	}
	return node;
}

code_program_t *cp_create(cp_line_destroy_fn destroy_line)
{
	if (destroy_line == NULL) {
		return NULL;
	}
	code_program_t *program = calloc(1, sizeof(*program));
	if (program == NULL) {
		return NULL;
	}
	program->lines = List_create();
	if (program->lines == NULL) {
		free(program);
		return NULL;
	}
	program->destroy_line = destroy_line;
	return program;
}

void cp_clear(code_program_t *program)
{
	if (program == NULL) {
		return;
	}
	while (program->lines->first != NULL) {
		code_line_t *line = List_shift(program->lines);
		program->destroy_line(line);
	}
}

void cp_destroy(code_program_t *program)
{
	if (program == NULL) {
		return;
	}
	cp_clear(program);
	List_destroy(program->lines);
	free(program);
}

int cp_count(const code_program_t *program)
{
	return program == NULL ? 0 : List_count(program->lines);
}

code_line_t *cp_get(const code_program_t *program, int index)
{
	if (program == NULL || index < 0 || index >= List_count(program->lines)) {
		return NULL;
	}
	return cp_node_at(program, index)->value;
}

code_line_t *cp_iterator_next(const code_program_t *program,
							  cp_iterator_t *iterator)
{
	if (program == NULL || iterator == NULL) {
		return NULL;
	}
	return cp_get(program, iterator->next_index++);
}

int cp_index_of(const code_program_t *program, const code_line_t *line)
{
	if (program == NULL || line == NULL) {
		return -1;
	}
	int index = 0;
	for (ListNode *node = program->lines->first; node != NULL; node = node->next) {
		if (node->value == line) {
			return index;
		}
		index++;
	}
	return -1;
}

bool cp_contains(const code_program_t *program, const code_line_t *line)
{
	return cp_index_of(program, line) >= 0;
}

cp_result_t cp_insert(code_program_t *program, int index, code_line_t *line)
{
	if (program == NULL || line == NULL) {
		return CP_INVALID_ARGUMENT;
	}
	int count = List_count(program->lines);
	if (index < 0 || index > count) {
		return CP_OUT_OF_RANGE;
	}
	if (cp_contains(program, line)) {
		return CP_DUPLICATE_LINE;
	}
	ListNode *node = calloc(1, sizeof(*node));
	if (node == NULL) {
		return CP_ALLOCATION_FAILED;
	}
	node->value = line;
	if (index == 0) {
		node->next = program->lines->first;
		if (node->next != NULL) {
			node->next->prev = node;
		} else {
			program->lines->last = node;
		}
		program->lines->first = node;
	} else if (index == count) {
		node->prev = program->lines->last;
		program->lines->last->next = node;
		program->lines->last = node;
	} else {
		ListNode *next = cp_node_at(program, index);
		node->prev = next->prev;
		node->next = next;
		next->prev->next = node;
		next->prev = node;
	}
	program->lines->count++;
	return CP_OK;
}

cp_result_t cp_remove(code_program_t *program, int index, code_line_t **line)
{
	if (program == NULL || line == NULL) {
		return CP_INVALID_ARGUMENT;
	}
	if (index < 0 || index >= List_count(program->lines)) {
		return CP_OUT_OF_RANGE;
	}
	*line = List_remove(program->lines, cp_node_at(program, index));
	return CP_OK;
}

cp_result_t cp_move(code_program_t *program, int from, int to)
{
	if (program == NULL) {
		return CP_INVALID_ARGUMENT;
	}
	int count = List_count(program->lines);
	if (from < 0 || from >= count || to < 0 || to >= count) {
		return CP_OUT_OF_RANGE;
	}
	if (from == to) {
		return CP_OK;
	}

	List *lines = program->lines;
	ListNode *moving = cp_node_at(program, from);
	if (moving->prev != NULL) {
		moving->prev->next = moving->next;
	} else {
		lines->first = moving->next;
	}
	if (moving->next != NULL) {
		moving->next->prev = moving->prev;
	} else {
		lines->last = moving->prev;
	}
	lines->count--;

	if (to == lines->count) {
		moving->prev = lines->last;
		moving->next = NULL;
		if (lines->last != NULL) {
			lines->last->next = moving;
		} else {
			lines->first = moving;
		}
		lines->last = moving;
	} else {
		ListNode *destination = cp_node_at(program, to);
		moving->prev = destination->prev;
		moving->next = destination;
		if (destination->prev != NULL) {
			destination->prev->next = moving;
		} else {
			lines->first = moving;
		}
		destination->prev = moving;
	}
	lines->count++;
	return CP_OK;
}
