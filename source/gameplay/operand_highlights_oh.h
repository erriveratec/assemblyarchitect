#ifndef OPERAND_HIGHLIGHTS_OH_H
#define OPERAND_HIGHLIGHTS_OH_H

#include <stdbool.h>
#include <stddef.h>
#include "code_line_cl.h"

typedef struct operand_availability_t {
	bool registers;
	bool input_buffer;
	bool output_buffer;
	bool immediates;
} operand_availability_t;

operand_availability_t oh_get_operand_availability(
	code_line_t *line,
	const int *register_ids,
	size_t register_count,
	bool registers_selectable,
	bool buffers_selectable,
	bool immediates_visible);
void oh_update(bool enabled, bool selection_possible);
void oh_clear(void);

#endif