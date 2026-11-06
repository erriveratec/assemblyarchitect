#ifndef OPERAND_FEEDBACK_OF_H
#define OPERAND_FEEDBACK_OF_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "code_edit_ce.h"
#include "text_tx.h"

typedef enum of_dismiss_t {
	OF_DISMISS_MOUSE_PRESS
} of_dismiss_t;

typedef struct of_definition_t {
	char name[64];
	ce_operand_feedback_id_t diagnostic_id;
	enum text_box_positions box;
	bool large_box;
	bool large_text;
	int header;
	of_dismiss_t dismiss;
	char *text;
} of_definition_t;

typedef struct of_repository_t of_repository_t;

/* Loads and validates an operand-feedback CFG. The returned repository owns
 * all definition names and text; destroy it with of_destroy(). */
of_repository_t *of_load_file(const char *path, char *error, size_t error_size);
/* Stream variant used by deterministic parser tests; it does not close stream. */
of_repository_t *of_load_stream(
	FILE *stream, const char *source_name, char *error, size_t error_size);
void of_destroy(of_repository_t *repository);
size_t of_definition_count(const of_repository_t *repository);
const of_definition_t *of_definition_at(
	const of_repository_t *repository, size_t index);
/* Resolves a diagnostic to its case, falling back to the configured generic
 * definition when the case is not present. Returned definitions are borrowed. */
const of_definition_t *of_resolve(
	const of_repository_t *repository, ce_operand_feedback_id_t diagnostic_id);
const of_definition_t *of_fallback(const of_repository_t *repository);

#endif
