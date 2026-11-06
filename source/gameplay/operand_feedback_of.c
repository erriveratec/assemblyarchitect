#include "gameplay/operand_feedback_of.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

enum { OF_MAX_DEFINITIONS = 32, OF_LINE_CAPACITY = 512 };

struct of_repository_t {
	of_definition_t *definitions;
	size_t count;
	char fallback_name[64];
	bool has_format;
	bool has_fallback;
	bool has_defaults;
	bool has_cases;
};

typedef struct of_defaults_t {
	enum text_box_positions box;
	bool large_box;
	bool large_text;
	int header;
	of_dismiss_t dismiss;
} of_defaults_t;

static char *of_trim(char *text)
{
	while (*text == ' ' || *text == '\t') {
		text++;
	}
	size_t length = strlen(text);
	while (length > 0 &&
	       (text[length - 1] == ' ' || text[length - 1] == '\t')) {
		text[--length] = '\0';
	}
	return text;
}

static bool of_set_error(
	char *error, size_t error_size, const char *source,
	const char *section, const char *property, const char *message)
{
	if (error != NULL && error_size > 0) {
		snprintf(error, error_size, "%s: section '%s', property '%s': %s",
		         source != NULL ? source : "operand_feedback.cfg",
		         section != NULL ? section : "operand_feedback",
		         property != NULL ? property : "configuration", message);
	}
	return false;
}

static bool of_parse_box(const char *value, enum text_box_positions *box)
{
	if (strcmp(value, "center_up") == 0) *box = TX_CENTER_UP_BOX;
	else if (strcmp(value, "upper") == 0) *box = TX_UPPER_BOX;
	else if (strcmp(value, "upper_right") == 0) *box = TX_UPPER_RIGHT_BOX;
	else if (strcmp(value, "center") == 0) *box = TX_CENTER_BOX;
	else if (strcmp(value, "center_right") == 0) *box = TX_CENTER_RIGHT_BOX;
	else if (strcmp(value, "lower") == 0) *box = TX_LOWER_BOX;
	else if (strcmp(value, "code") == 0) *box = TX_CODE_BOX;
	else if (strcmp(value, "instruction") == 0) *box = TX_INS_BOX;
	else return false;
	return true;
}

static bool of_parse_header(const char *value, int *header)
{
	if (strcmp(value, "none") == 0) *header = TX_NONE;
	else if (strcmp(value, "system_message") == 0) *header = TX_SYSMES;
	else if (strcmp(value, "system_notice") == 0) *header = TX_SYSNOT;
	else if (strcmp(value, "system_warning") == 0) *header = TX_SYSWAR;
	else if (strcmp(value, "instruction") == 0) *header = TX_INS;
	else if (strcmp(value, "instruction_feedback") == 0) {
		*header = TX_INS_FEEDBACK;
	}
	else return false;
	return true;
}

static ce_operand_feedback_id_t of_parse_diagnostic(const char *value)
{
	if (strcmp(value, "MOV_OUTPUT_BUFFER_AS_SOURCE") == 0) {
		return CE_OPERAND_FEEDBACK_MOV_OUTPUT_BUFFER_AS_SOURCE;
	}
	if (strcmp(value, "MOV_INPUT_BUFFER_AS_DESTINATION") == 0) {
		return CE_OPERAND_FEEDBACK_MOV_INPUT_BUFFER_AS_DESTINATION;
	}
	if (strcmp(value, "MOV_BUFFER_TO_BUFFER") == 0) {
		return CE_OPERAND_FEEDBACK_MOV_BUFFER_TO_BUFFER;
	}
	if (strcmp(value, "INCOMPATIBLE_OPERAND_GENERIC") == 0) {
		return CE_OPERAND_FEEDBACK_INCOMPATIBLE_OPERAND_GENERIC;
	}
	return CE_OPERAND_FEEDBACK_NONE;
}

static const of_definition_t *of_find_name(
	const of_repository_t *repository, const char *name)
{
	for (size_t index = 0; index < repository->count; index++) {
		if (strcmp(repository->definitions[index].name, name) == 0) {
			return &repository->definitions[index];
		}
	}
	return NULL;
}

static bool of_append_text(
	of_definition_t *definition, const char *line, size_t length,
	bool add_newline)
{
	size_t used = definition->text != NULL ? strlen(definition->text) : 0;
	if (length > 8192 || used > 8192 - length - (add_newline ? 1u : 0u)) {
		return false;
	}
	char *text = realloc(definition->text, used + length +
	                     (add_newline ? 2u : 1u));
	if (text == NULL) {
		return false;
	}
	definition->text = text;
	if (add_newline) text[used++] = '\n';
	memcpy(text + used, line, length);
	text[used + length] = '\0';
	return true;
}

static bool of_apply_property(
	const char *key, const char *value, of_definition_t *definition,
	of_defaults_t *defaults, bool is_defaults, bool *diagnostic_seen,
	char *error, size_t error_size, const char *source, const char *section)
{
	if (strcmp(key, "box") == 0) {
		enum text_box_positions box;
		if (!of_parse_box(value, &box)) {
			return of_set_error(error, error_size, source, section, key,
			                    "unsupported box value");
		}
		if (is_defaults) defaults->box = box;
		else definition->box = box;
	} else if (strcmp(key, "box_size") == 0) {
		bool large;
		if (strcmp(value, "large") == 0) large = true;
		else if (strcmp(value, "normal") == 0) large = false;
		else return of_set_error(error, error_size, source, section, key,
		                         "unsupported box_size value");
		if (is_defaults) defaults->large_box = large;
		else definition->large_box = large;
	} else if (strcmp(key, "text_size") == 0) {
		bool large;
		if (strcmp(value, "large") == 0) large = true;
		else if (strcmp(value, "normal") == 0) large = false;
		else return of_set_error(error, error_size, source, section, key,
		                         "unsupported text_size value");
		if (is_defaults) defaults->large_text = large;
		else definition->large_text = large;
	} else if (strcmp(key, "header") == 0) {
		int header;
		if (!of_parse_header(value, &header)) {
			char message[160];
			snprintf(message, sizeof(message),
			         "unsupported header value '%s'", value);
			return of_set_error(error, error_size, source, section, key, message);
		}
		if (is_defaults) defaults->header = header;
		else definition->header = header;
	} else if (strcmp(key, "dismiss") == 0) {
		if (strcmp(value, "mouse_press") != 0) {
			return of_set_error(error, error_size, source, section, key,
			                    "unsupported dismissal value");
		}
		if (is_defaults) defaults->dismiss = OF_DISMISS_MOUSE_PRESS;
		else definition->dismiss = OF_DISMISS_MOUSE_PRESS;
	} else if (strcmp(key, "diagnostic") == 0 && !is_defaults) {
		if (*diagnostic_seen) {
			return of_set_error(error, error_size, source, section, key,
			                    "duplicate diagnostic identifier");
		}
		ce_operand_feedback_id_t id = of_parse_diagnostic(value);
		if (id == CE_OPERAND_FEEDBACK_NONE) {
			return of_set_error(error, error_size, source, section, key,
			                    "unknown diagnostic identifier");
		}
		definition->diagnostic_id = id;
		*diagnostic_seen = true;
	} else {
		return of_set_error(error, error_size, source, section, key,
		                    "unsupported property");
	}
	return true;
}

static bool of_finish_section(
	of_definition_t *definition, bool diagnostic_seen, bool text_seen,
	const char *source, char *error, size_t error_size)
{
	if (definition == NULL) return true;
	if (!diagnostic_seen || definition->diagnostic_id == CE_OPERAND_FEEDBACK_NONE) {
		return of_set_error(error, error_size, source, definition->name,
		                    "diagnostic", "missing diagnostic identifier");
	}
	bool has_text = false;
	if (definition->text != NULL) {
		for (const char *character = definition->text; *character != '\0';
		     character++) {
			if (!isspace((unsigned char)*character)) {
				has_text = true;
				break;
			}
		}
	}
	if (!text_seen || !has_text) {
		return of_set_error(error, error_size, source, definition->name,
		                    "text_begin", "missing or empty text block");
	}
	return true;
}

of_repository_t *of_load_stream(
	FILE *stream, const char *source_name, char *error, size_t error_size)
{
	if (error != NULL && error_size > 0) error[0] = '\0';
	if (stream == NULL || source_name == NULL) {
		of_set_error(error, error_size, source_name, NULL, NULL,
		             "invalid input stream");
		return NULL;
	}
	of_repository_t *repository = calloc(1, sizeof(*repository));
	if (repository == NULL) {
		of_set_error(error, error_size, source_name, NULL, NULL,
		             "allocation failed");
		return NULL;
	}
	of_defaults_t defaults = {
		.box = TX_CENTER_BOX,
		.large_box = true,
		.large_text = false,
		.header = TX_INS_FEEDBACK,
		.dismiss = OF_DISMISS_MOUSE_PRESS
	};
	of_definition_t *current = NULL;
	bool in_defaults = false;
	bool reading_text = false;
	bool diagnostic_seen = false;
	bool text_seen = false;
	char line[OF_LINE_CAPACITY];
	char section[64] = "operand_feedback";
	bool valid = true;

	while (fgets(line, sizeof(line), stream) != NULL) {
		if (strchr(line, '\n') == NULL && !feof(stream)) {
			valid = of_set_error(error, error_size, source_name, section,
			                     "line", "line exceeds parser buffer");
			break;
		}
		size_t raw_length = strcspn(line, "\r\n");
		line[raw_length] = '\0';
		char raw_line[OF_LINE_CAPACITY];
		memcpy(raw_line, line, raw_length + 1);
		char *text = of_trim(line);
		size_t length = strlen(text);
		if (reading_text) {
			if (strcmp(text, "text_end") == 0) {
				reading_text = false;
				continue;
			}
			if (!of_append_text(current, raw_line, raw_length,
			                    current->text != NULL)) {
				valid = of_set_error(error, error_size, source_name, section,
				                     "text_begin", "text allocation or size limit failed");
				break;
			}
			continue;
		}
		if (length == 0 || text[0] == '#' || text[0] == ';') continue;
		if (text[0] == '[') {
			if (length < 3 || text[length - 1] != ']') {
				valid = of_set_error(error, error_size, source_name, section,
				                     "section", "malformed section header");
				break;
			}
			if (!of_finish_section(current, diagnostic_seen, text_seen,
			                       source_name, error, error_size)) {
				valid = false;
				break;
			}
			text[length - 1] = '\0';
			char *name = text + 1;
			if (strlen(name) >= sizeof(section)) {
				valid = of_set_error(error, error_size, source_name, name,
				                     "section", "section name is too long");
				break;
			}
			strcpy(section, name);
			current = NULL;
			diagnostic_seen = false;
			text_seen = false;
			if (strcmp(name, "operand_feedback") == 0) {
				if (repository->has_format) {
					valid = of_set_error(error, error_size, source_name, name,
					                     "section", "duplicate root section");
					break;
				}
				continue;
			}
			if (strcmp(name, "defaults") == 0) {
				if (repository->has_defaults || repository->has_cases) {
					valid = of_set_error(error, error_size, source_name, name,
					                     "section", "defaults must appear once before cases");
					break;
				}
				repository->has_defaults = true;
				in_defaults = true;
				continue;
			}
			in_defaults = false;
			if (repository->count >= OF_MAX_DEFINITIONS ||
			    of_find_name(repository, name) != NULL) {
				valid = of_set_error(error, error_size, source_name, name,
				                     "section", "duplicate section or definition limit exceeded");
				break;
			}
			of_definition_t *definitions = realloc(
				repository->definitions,
				(repository->count + 1) * sizeof(*definitions));
			if (definitions == NULL) {
				valid = of_set_error(error, error_size, source_name, name,
				                     "section", "allocation failed");
				break;
			}
			repository->definitions = definitions;
			current = &repository->definitions[repository->count++];
			*current = (of_definition_t){
				.box = defaults.box,
				.large_box = defaults.large_box,
				.large_text = defaults.large_text,
				.header = defaults.header,
				.dismiss = defaults.dismiss
			};
			strcpy(current->name, name);
			repository->has_cases = true;
			continue;
		}
		if (strcmp(text, "text_begin") == 0) {
			if (current == NULL || text_seen) {
				valid = of_set_error(error, error_size, source_name, section,
				                     "text_begin", "text block is not allowed here or is duplicated");
				break;
			}
			reading_text = true;
			text_seen = true;
			continue;
		}
		if (strcmp(text, "text_end") == 0) {
			valid = of_set_error(error, error_size, source_name, section,
			                     "text_end", "text_end without text_begin");
			break;
		}
		char *equals = strchr(text, '=');
		if (equals == NULL) {
			valid = of_set_error(error, error_size, source_name, section,
			                     "property", "expected key = value");
			break;
		}
		*equals = '\0';
		char *key = of_trim(text);
		char *value = of_trim(equals + 1);
		if (strcmp(section, "operand_feedback") == 0) {
			if (strcmp(key, "format") == 0 && strcmp(value, "1") == 0 &&
			    !repository->has_format) {
				repository->has_format = true;
			} else if (strcmp(key, "fallback") == 0 &&
			           strlen(value) < sizeof(repository->fallback_name) &&
			           !repository->has_fallback) {
				strcpy(repository->fallback_name, value);
				repository->has_fallback = true;
			} else {
				valid = of_set_error(error, error_size, source_name, section,
				                     key, "unsupported, duplicate, or invalid root property");
				break;
			}
		} else if (in_defaults) {
			if (!of_apply_property(key, value, NULL, &defaults, true,
			                       &diagnostic_seen, error, error_size,
			                       source_name, section)) {
				valid = false;
				break;
			}
		} else if (current != NULL) {
			if (!of_apply_property(key, value, current, &defaults, false,
			                       &diagnostic_seen, error, error_size,
			                       source_name, section)) {
				valid = false;
				break;
			}
		} else {
			valid = of_set_error(error, error_size, source_name, section,
			                     key, "property outside a supported section");
			break;
		}
	}
	if (valid && ferror(stream)) {
		valid = of_set_error(error, error_size, source_name, section,
		                     "file", "read failed");
	}
	if (valid && reading_text) {
		valid = of_set_error(error, error_size, source_name, section,
		                     "text_end", "missing text_end");
	}
	if (valid && !of_finish_section(current, diagnostic_seen, text_seen,
	                                source_name, error, error_size)) {
		valid = false;
	}
	if (valid && (!repository->has_format || !repository->has_fallback)) {
		valid = of_set_error(error, error_size, source_name, "operand_feedback",
		                     !repository->has_format ? "format" : "fallback",
		                     "required root property is missing");
	}
	if (valid) {
		for (size_t index = 0; index < repository->count; index++) {
			for (size_t other = index + 1; other < repository->count; other++) {
				if (repository->definitions[index].diagnostic_id ==
				    repository->definitions[other].diagnostic_id) {
					valid = of_set_error(error, error_size, source_name,
						repository->definitions[other].name, "diagnostic",
						"duplicate diagnostic identifier");
					break;
				}
			}
			if (!valid) break;
		}
	}
	if (valid) {
		const of_definition_t *fallback = of_find_name(
			repository, repository->fallback_name);
		if (fallback == NULL) {
			valid = of_set_error(error, error_size, source_name,
			                     "operand_feedback", "fallback",
			                     "fallback section does not exist");
		} else if (fallback->diagnostic_id !=
		           CE_OPERAND_FEEDBACK_INCOMPATIBLE_OPERAND_GENERIC) {
			valid = of_set_error(error, error_size, source_name,
			                     fallback->name, "diagnostic",
			                     "fallback must use generic diagnostic identifier");
		}
	}
	if (!valid) {
		of_destroy(repository);
		return NULL;
	}
	return repository;
}

of_repository_t *of_load_file(const char *path, char *error, size_t error_size)
{
	if (path == NULL) {
		of_set_error(error, error_size, path, NULL, NULL, "missing file path");
		return NULL;
	}
	FILE *stream = fopen(path, "r");
	if (stream == NULL) {
		of_set_error(error, error_size, path, NULL, NULL, "could not open file");
		return NULL;
	}
	of_repository_t *repository = of_load_stream(stream, path, error, error_size);
	fclose(stream);
	return repository;
}

void of_destroy(of_repository_t *repository)
{
	if (repository == NULL) return;
	for (size_t index = 0; index < repository->count; index++) {
		free(repository->definitions[index].text);
	}
	free(repository->definitions);
	free(repository);
}

size_t of_definition_count(const of_repository_t *repository)
{
	return repository != NULL ? repository->count : 0;
}

const of_definition_t *of_definition_at(
	const of_repository_t *repository, size_t index)
{
	return repository != NULL && index < repository->count
		? &repository->definitions[index] : NULL;
}

const of_definition_t *of_fallback(const of_repository_t *repository)
{
	return repository != NULL
		? of_find_name(repository, repository->fallback_name) : NULL;
}

const of_definition_t *of_resolve(
	const of_repository_t *repository, ce_operand_feedback_id_t diagnostic_id)
{
	if (repository == NULL) return NULL;
	for (size_t index = 0; index < repository->count; index++) {
		if (repository->definitions[index].diagnostic_id == diagnostic_id) {
			return &repository->definitions[index];
		}
	}
	return of_fallback(repository);
}
