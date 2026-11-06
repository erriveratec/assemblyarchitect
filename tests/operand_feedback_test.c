#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "gameplay/operand_feedback_of.h"

static of_repository_t *load_text(const char *text, char *error, size_t error_size)
{
	FILE *stream = tmpfile();
	assert(stream != NULL);
	size_t length = strlen(text);
	assert(fwrite(text, 1, length, stream) == length);
	rewind(stream);
	of_repository_t *repository = of_load_stream(
		stream, "operand-feedback-test.cfg", error, error_size);
	fclose(stream);
	return repository;
}

static void test_production_configuration(void)
{
	char error[512];
	of_repository_t *repository = of_load_file(
		"data/operand_feedback.cfg", error, sizeof(error));
	assert(repository != NULL && error[0] == '\0');
	assert(of_definition_count(repository) == 4);
	for (size_t index = 0; index < of_definition_count(repository); index++) {
		const of_definition_t *definition =
			of_definition_at(repository, index);
		assert(definition != NULL);
		assert(definition->box == TX_CENTER_BOX);
		assert(definition->large_box);
		assert(!definition->large_text);
		assert(definition->header == TX_INS_FEEDBACK);
		assert(definition->dismiss == OF_DISMISS_MOUSE_PRESS);
	}
	const of_definition_t *output = of_resolve(
		repository, CE_OPERAND_FEEDBACK_MOV_OUTPUT_BUFFER_AS_SOURCE);
	const of_definition_t *input = of_resolve(
		repository, CE_OPERAND_FEEDBACK_MOV_INPUT_BUFFER_AS_DESTINATION);
	const of_definition_t *buffers = of_resolve(
		repository, CE_OPERAND_FEEDBACK_MOV_BUFFER_TO_BUFFER);
	const of_definition_t *fallback = of_fallback(repository);
	assert(output != NULL && input != NULL && buffers != NULL && fallback != NULL);
	assert(output->box == TX_CENTER_BOX && output->large_box && !output->large_text);
	assert(output->header == TX_INS_FEEDBACK &&
	       output->dismiss == OF_DISMISS_MOUSE_PRESS);
	assert(strstr(output->text, "@warning INVALID SOURCE OPERAND") != NULL);
	assert(strstr(output->text, "@directive") != NULL);
	assert(strstr(output->text, "@syntax MOV [ob], rax") != NULL);
	assert(strstr(input->text, "Input Buffer [ib] is read-only") != NULL);
	assert(strstr(input->text, "@warning INVALID DESTINATION OPERAND") != NULL);
	assert(strstr(buffers->text, "@syntax MOV rax, [ib]") != NULL);
	assert(strstr(buffers->text, "@syntax MOV [ob], rax") != NULL);
	assert(strstr(buffers->text,
	              "@warning DIRECT BUFFER MOV NOT ALLOWED") != NULL);
	assert(strstr(buffers->text, "MOV cannot transfer [ib] to [ob].") != NULL);
	assert(strstr(fallback->text,
	              "@warning OPERAND COMBINATION NOT ALLOWED") != NULL);
	assert(of_resolve(repository, CE_OPERAND_FEEDBACK_NONE) == fallback);
	of_destroy(repository);
}

static const char *minimal_configuration =
	"[operand_feedback]\nformat = 1\nfallback = generic\n"
	"[defaults]\nbox = center\nbox_size = large\ntext_size = normal\n"
	"dismiss = mouse_press\n"
	"[generic]\ndiagnostic = INCOMPATIBLE_OPERAND_GENERIC\n"
	"text_begin\n@warning GENERIC\n@directive Continue\ntext_end\n"
	"[large_case]\ndiagnostic = MOV_OUTPUT_BUFFER_AS_SOURCE\n"
	"text_size = large\nheader = instruction\n"
	"text_begin\n@syntax MOV [ob], rax\ntext_end\n";

static void assert_rejected(const char *text, const char *expected)
{
	char error[512];
	of_repository_t *repository = load_text(text, error, sizeof(error));
	assert(repository == NULL);
	assert(error[0] != '\0');
	assert(strstr(error, expected) != NULL);
}

static void test_invalid_configuration_cases(void)
{
	char unknown_diagnostic[1024];
	snprintf(unknown_diagnostic, sizeof(unknown_diagnostic),
		"[operand_feedback]\nformat = 1\nfallback = generic\n"
		"[generic]\ndiagnostic = UNKNOWN_ID\ntext_begin\nText\ntext_end\n");
	assert_rejected(unknown_diagnostic, "unknown diagnostic identifier");

	static const char duplicate_diagnostic[] =
		"[operand_feedback]\nformat = 1\nfallback = generic\n"
		"[generic]\ndiagnostic = INCOMPATIBLE_OPERAND_GENERIC\n"
		"text_begin\nGeneric\ntext_end\n"
		"[another]\ndiagnostic = INCOMPATIBLE_OPERAND_GENERIC\n"
		"text_begin\nDuplicate\ntext_end\n";
	assert_rejected(duplicate_diagnostic, "duplicate diagnostic identifier");

	static const char malformed_text[] =
		"[operand_feedback]\nformat = 1\nfallback = generic\n"
		"[generic]\ndiagnostic = INCOMPATIBLE_OPERAND_GENERIC\n"
		"text_begin\nMissing end\n";
	assert_rejected(malformed_text, "missing text_end");

	static const char missing_fallback[] =
		"[operand_feedback]\nformat = 1\n"
		"[generic]\ndiagnostic = INCOMPATIBLE_OPERAND_GENERIC\n"
		"text_begin\nGeneric\ntext_end\n";
	assert_rejected(missing_fallback, "fallback");

	static const char missing_fallback_section[] =
		"[operand_feedback]\nformat = 1\nfallback = absent\n"
		"[generic]\ndiagnostic = INCOMPATIBLE_OPERAND_GENERIC\n"
		"text_begin\nGeneric\ntext_end\n";
	assert_rejected(missing_fallback_section, "fallback section does not exist");

	static const char unsupported_value[] =
		"[operand_feedback]\nformat = 1\nfallback = generic\n"
		"[defaults]\nbox = nowhere\n"
		"[generic]\ndiagnostic = INCOMPATIBLE_OPERAND_GENERIC\n"
		"text_begin\nGeneric\ntext_end\n";
	assert_rejected(unsupported_value, "unsupported box value");
}

static void test_unknown_header_diagnostic(void)
{
	static const char text[] =
		"[operand_feedback]\nformat = 1\nfallback = generic\n"
		"[defaults]\nheader = unknown_feedback_header\n"
		"[generic]\ndiagnostic = INCOMPATIBLE_OPERAND_GENERIC\n"
		"text_begin\nFallback\ntext_end\n";
	char error[512];
	of_repository_t *repository = load_text(text, error, sizeof(error));
	assert(repository == NULL);
	assert(strstr(error, "operand-feedback-test.cfg") != NULL);
	assert(strstr(error, "section 'defaults'") != NULL);
	assert(strstr(error, "property 'header'") != NULL);
	assert(strstr(error, "unknown_feedback_header") != NULL);
}

static void test_defaults_and_fallback_resolution(void)
{
	char error[512];
	of_repository_t *repository = load_text(
		minimal_configuration, error, sizeof(error));
	assert(repository != NULL && error[0] == '\0');
	const of_definition_t *definition = of_definition_at(repository, 0);
	assert(definition != NULL);
	assert(definition->box == TX_CENTER_BOX);
	assert(definition->large_box && !definition->large_text);
	assert(definition->header == TX_INS_FEEDBACK);
	assert(definition->dismiss == OF_DISMISS_MOUSE_PRESS);
	assert(of_resolve(repository,
	                  CE_OPERAND_FEEDBACK_MOV_BUFFER_TO_BUFFER) == definition);
	assert(strcmp(definition->text, "@warning GENERIC\n@directive Continue") == 0);
	const of_definition_t *large_definition = of_definition_at(repository, 1);
	assert(large_definition != NULL && large_definition->large_box);
	assert(large_definition->large_text);
	assert(large_definition->header == TX_INS);
	of_destroy(repository);
}

int main(void)
{
	test_production_configuration();
	test_invalid_configuration_cases();
	test_defaults_and_fallback_resolution();
	test_unknown_header_diagnostic();
	return 0;
}
