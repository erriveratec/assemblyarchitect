#include "ui/instruction_feedback_if.h"

#include <stdio.h>
#include <stdlib.h>

#include "gameplay/operand_feedback_of.h"
#include "mouse_ms.h"
#include "text_tx.h"

typedef struct ifb_entry_t {
	const of_definition_t *definition;
	tx_styled_text_t *message;
} ifb_entry_t;

typedef struct ifb_state_t {
	of_repository_t *repository;
	ifb_entry_t *entries;
	size_t entry_count;
	ifb_entry_t *active_entry;
} ifb_state_t;

static ifb_state_t g_feedback;

static tx_text_box_options_t ifb_text_box_options(
	const of_definition_t *definition)
{
	return (tx_text_box_options_t){
		.position = definition->box,
		.large_box = definition->large_box,
		.large_text = definition->large_text
	};
}

static void ifb_clear_active(void)
{
	g_feedback.active_entry = NULL;
}

void ifb_destroy(void)
{
	ifb_clear_active();
	if (g_feedback.entries != NULL) {
		for (size_t index = 0; index < g_feedback.entry_count; index++) {
			tx_free_styled_text(g_feedback.entries[index].message);
		}
	}
	free(g_feedback.entries);
	g_feedback.entries = NULL;
	g_feedback.entry_count = 0;
	of_destroy(g_feedback.repository);
	g_feedback.repository = NULL;
}

bool ifb_init(const char *configuration_path)
{
	ifb_destroy();
	char error[512];
	g_feedback.repository = of_load_file(configuration_path, error,
	                                     sizeof(error));
	if (g_feedback.repository == NULL) {
		fprintf(stderr, "Instruction Feedback configuration: %s\n", error);
		return false;
	}
	g_feedback.entry_count = of_definition_count(g_feedback.repository);
	g_feedback.entries = calloc(g_feedback.entry_count,
	                            sizeof(*g_feedback.entries));
	if (g_feedback.entries == NULL) {
		fprintf(stderr, "Instruction Feedback: allocation failed\n");
		ifb_destroy();
		return false;
	}
	for (size_t index = 0; index < g_feedback.entry_count; index++) {
		const of_definition_t *definition =
			of_definition_at(g_feedback.repository, index);
		tx_text_box_options_t options = ifb_text_box_options(definition);
		tx_text_layout_info_t layout = {0};
		tx_styled_text_t *message = tx_create_styled_text_box_message(
			&options, definition->text, &layout);
		if (message == NULL || layout.overflowed) {
			fprintf(stderr,
			        "Instruction Feedback configuration: section '%s' text %s "
			        "(requires %d rows, box supports %d)\n",
			        definition->name,
			        message == NULL ? "could not be rendered" : "exceeds its box",
			        layout.required_rows, layout.visible_rows);
			tx_free_styled_text(message);
			ifb_destroy();
			return false;
		}
		g_feedback.entries[index] = (ifb_entry_t){
			.definition = definition,
			.message = message
		};
	}
	return true;
}

bool ifb_open(const ce_diagnostic_t *diagnostic)
{
	if (g_feedback.repository == NULL || diagnostic == NULL ||
	    diagnostic->feedback_id == CE_OPERAND_FEEDBACK_NONE) {
		return false;
	}
	if (g_feedback.active_entry != NULL) {
		return true;
	}
	const of_definition_t *definition = of_resolve(
		g_feedback.repository, diagnostic->feedback_id);
	for (size_t index = 0; index < g_feedback.entry_count; index++) {
		if (g_feedback.entries[index].definition == definition) {
			g_feedback.active_entry = &g_feedback.entries[index];
			ms_consume_pointer_input(true);
			return true;
		}
	}
	return false;
}

bool ifb_is_active(void)
{
	return g_feedback.active_entry != NULL;
}

bool ifb_update(bool input_enabled)
{
	if (!ifb_is_active() || !input_enabled) {
		return false;
	}
	bool dismiss = ms_left_pressed();
	ms_consume_pointer_input(dismiss);
	if (dismiss) {
		ifb_clear_active();
	}
	return true;
}

void ifb_draw(void)
{
	if (!ifb_is_active()) {
		return;
	}
	const of_definition_t *definition = g_feedback.active_entry->definition;
	tx_text_box_options_t options = ifb_text_box_options(definition);
	tx_draw_styled_text_box_message(&options,
	                                g_feedback.active_entry->message,
	                                definition->header);
	if (definition->dismiss == OF_DISMISS_MOUSE_PRESS) {
		tx_bottom_msg(&options, TX_MSG_CLICKANY);
	}
}
