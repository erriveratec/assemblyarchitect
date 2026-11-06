#include <assert.h>
#include <stdlib.h>

#include "gameplay/operand_feedback_of.h"
#include "ui/instruction_feedback_if.h"

static bool left_press;
static bool left_down;
static bool right_down;
static bool consume_suppressed_left;
static bool consume_suppressed_right;
static int consume_count;
static int draw_count;
static int hint_count;
static int destroy_repository_count;
static int fake_repository_storage;
static tx_styled_text_t *created_messages[4];
static size_t created_message_count;

static of_definition_t fake_definitions[] = {
	{
		.name = "specific",
		.diagnostic_id = CE_OPERAND_FEEDBACK_MOV_OUTPUT_BUFFER_AS_SOURCE,
		.box = TX_CENTER_BOX,
		.large_box = true,
		.large_text = false,
		.header = TX_INS_FEEDBACK,
		.dismiss = OF_DISMISS_MOUSE_PRESS,
		.text = "@warning Specific feedback"
	},
	{
		.name = "generic",
		.diagnostic_id = CE_OPERAND_FEEDBACK_INCOMPATIBLE_OPERAND_GENERIC,
		.box = TX_CENTER_BOX,
		.large_box = true,
		.large_text = false,
		.header = TX_INS_FEEDBACK,
		.dismiss = OF_DISMISS_MOUSE_PRESS,
		.text = "@warning Generic feedback"
	}
};

static of_repository_t *test_of_load_file(
	const char *path, char *error, size_t error_size)
{
	assert(path != NULL && error != NULL && error_size > 0);
	error[0] = '\0';
	return (of_repository_t *)&fake_repository_storage;
}

static void test_of_destroy(of_repository_t *repository)
{
	if (repository == NULL) return;
	assert(repository == (of_repository_t *)&fake_repository_storage);
	destroy_repository_count++;
}

static size_t test_of_definition_count(const of_repository_t *repository)
{
	assert(repository == (const of_repository_t *)&fake_repository_storage);
	return sizeof(fake_definitions) / sizeof(fake_definitions[0]);
}

static const of_definition_t *test_of_definition_at(
	const of_repository_t *repository, size_t index)
{
	assert(repository == (const of_repository_t *)&fake_repository_storage);
	return index < test_of_definition_count(repository)
		? &fake_definitions[index] : NULL;
}

static const of_definition_t *test_of_resolve(
	const of_repository_t *repository, ce_operand_feedback_id_t diagnostic_id)
{
	assert(repository == (const of_repository_t *)&fake_repository_storage);
	for (size_t index = 0; index < test_of_definition_count(repository); index++) {
		if (fake_definitions[index].diagnostic_id == diagnostic_id) {
			return &fake_definitions[index];
		}
	}
	return &fake_definitions[1];
}

static tx_styled_text_t *test_create_styled_text_box_message(
	const tx_text_box_options_t *options, const char *text,
	tx_text_layout_info_t *layout)
{
	assert(options != NULL && options->position == TX_CENTER_BOX);
	assert(options->large_box && !options->large_text);
	assert(text != NULL && text[0] != '\0');
	assert(created_message_count < sizeof(created_messages) /
	       sizeof(created_messages[0]));
	if (layout != NULL) {
		*layout = (tx_text_layout_info_t){.required_rows = 2, .visible_rows = 2};
	}
	tx_styled_text_t *message = calloc(1, sizeof(*message));
	assert(message != NULL);
	created_messages[created_message_count++] = message;
	return message;
}

static void test_free_styled_text(tx_styled_text_t *message)
{
	free(message);
}

static void test_draw_styled_text_box_message(
	const tx_text_box_options_t *options, const tx_styled_text_t *message,
	int header)
{
	assert(options != NULL && message != NULL && header == TX_INS_FEEDBACK);
	assert(options->large_box && !options->large_text);
	draw_count++;
}

static void test_bottom_msg(
	const tx_text_box_options_t *options, int message_id)
{
	assert(options != NULL && options->position == TX_CENTER_BOX);
	assert(options->large_box && !options->large_text);
	assert(message_id == TX_MSG_CLICKANY);
	hint_count++;
}

static bool test_left_pressed(void)
{
	return left_press;
}

static void test_consume_pointer_input(bool suppress_held_releases)
{
	consume_count++;
	consume_suppressed_left = suppress_held_releases && left_down;
	consume_suppressed_right = suppress_held_releases && right_down;
	left_press = false;
}

#define of_load_file test_of_load_file
#define of_destroy test_of_destroy
#define of_definition_count test_of_definition_count
#define of_definition_at test_of_definition_at
#define of_resolve test_of_resolve
#define tx_create_styled_text_box_message test_create_styled_text_box_message
#define tx_free_styled_text test_free_styled_text
#define tx_draw_styled_text_box_message test_draw_styled_text_box_message
#define tx_bottom_msg test_bottom_msg
#define ms_left_pressed test_left_pressed
#define ms_consume_pointer_input test_consume_pointer_input
#include "../source/ui/instruction_feedback_if.c"
#undef of_load_file
#undef of_destroy
#undef of_definition_count
#undef of_definition_at
#undef of_resolve
#undef tx_create_styled_text_box_message
#undef tx_free_styled_text
#undef tx_draw_styled_text_box_message
#undef tx_bottom_msg
#undef ms_left_pressed
#undef ms_consume_pointer_input

static void reset_input(void)
{
	created_message_count = 0;
	left_press = false;
	left_down = false;
	right_down = false;
	consume_suppressed_left = false;
	consume_suppressed_right = false;
}

static void test_open_draw_and_left_dismissal(void)
{
	reset_input();
	assert(ifb_init("fixture.cfg"));
	ce_diagnostic_t diagnostic = {
		.result = CE_INCOMPATIBLE_OPERAND,
		.feedback_id = CE_OPERAND_FEEDBACK_MOV_OUTPUT_BUFFER_AS_SOURCE
	};
	left_down = true;
	assert(ifb_open(&diagnostic));
	assert(ifb_is_active());
	assert(consume_count == 1 && consume_suppressed_left);
	assert(ifb_open(&diagnostic));
	assert(consume_count == 1);
	ifb_draw();
	assert(draw_count == 1 && hint_count == 1);

	left_press = true;
	left_down = true;
	right_down = true;
	assert(ifb_update(true));
	assert(!ifb_is_active());
	assert(consume_suppressed_left && consume_suppressed_right);
	assert(consume_count == 2);
	assert(!ifb_update(true));
	ifb_destroy();
	assert(destroy_repository_count == 1);
	assert(created_message_count == 2);
}

static void test_right_press_is_consumed_without_dismissal(void)
{
	reset_input();
	assert(ifb_init("fixture.cfg"));
	ce_diagnostic_t diagnostic = {
		.result = CE_INCOMPATIBLE_OPERAND,
		.feedback_id = CE_OPERAND_FEEDBACK_MOV_OUTPUT_BUFFER_AS_SOURCE
	};
	assert(ifb_open(&diagnostic));
	right_down = true;
	assert(ifb_update(true));
	assert(ifb_is_active());
	assert(!consume_suppressed_right);
	assert(!ifb_update(false));
	assert(ifb_is_active());
	ifb_destroy();
}

static void test_unconfigured_case_uses_generic_fallback(void)
{
	reset_input();
	assert(ifb_init("fixture.cfg"));
	ce_diagnostic_t diagnostic = {
		.result = CE_INCOMPATIBLE_OPERAND,
		.feedback_id = CE_OPERAND_FEEDBACK_MOV_BUFFER_TO_BUFFER
	};
	assert(ifb_open(&diagnostic));
	assert(ifb_is_active());
	ifb_destroy();
}

int main(void)
{
	test_open_draw_and_left_dismissal();
	test_right_press_is_consumed_without_dismissal();
	test_unconfigured_case_uses_generic_fallback();
	return 0;
}
