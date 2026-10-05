#include <assert.h>
#include <stdio.h>

#include "gameplay/level_presentation_lp.h"
#include "gameplay/operand_highlights_oh.h"
#include "gameplay/ui_highlight_source.h"

static void test_operand_availability(void)
{
	int register_ids[] = {RAX, RBX};
	operand_availability_t availability = oh_get_operand_availability(
	    NULL, register_ids, sizeof(register_ids) / sizeof(register_ids[0]),
	    true, true, true);
	assert(!availability.registers && !availability.input_buffer &&
	       !availability.output_buffer && !availability.immediates);

	instruction_t instruction = {.id = MOV};
	code_line_t line = {.ins = &instruction, .state = MISSING_BOTH};
	availability = oh_get_operand_availability(
	    &line, register_ids, sizeof(register_ids) / sizeof(register_ids[0]),
	    true, true, true);
	assert(availability.registers);
	assert(!availability.input_buffer);
	assert(availability.output_buffer);
	assert(!availability.immediates);

	operand_t source = {.id = RAX};
	line.op1 = &source;
	line.state = MISSING_OP2;
	availability = oh_get_operand_availability(
	    &line, register_ids, sizeof(register_ids) / sizeof(register_ids[0]),
	    true, true, true);
	assert(availability.registers);
	assert(availability.input_buffer);
	assert(!availability.output_buffer);
	assert(availability.immediates);

	availability = oh_get_operand_availability(
	    &line, register_ids, sizeof(register_ids) / sizeof(register_ids[0]),
	    false, false, false);
	assert(!availability.registers);
	assert(!availability.input_buffer);
	assert(!availability.output_buffer);
	assert(!availability.immediates);

	instruction.id = JMP;
	availability = oh_get_operand_availability(
	    &line, register_ids, sizeof(register_ids) / sizeof(register_ids[0]),
	    true, true, true);
	assert(!availability.registers);
	assert(!availability.input_buffer);
	assert(!availability.output_buffer);
	assert(!availability.immediates);
}

static void test_highlight_source_composition(void)
{
	unsigned int sources = 0;
	sources = ui_highlight_source_set_enabled(
	    sources, UI_HIGHLIGHT_SOURCE_TUTORIAL, true);
	sources = ui_highlight_source_set_enabled(
	    sources, UI_HIGHLIGHT_SOURCE_AVAILABILITY, true);
	sources = ui_highlight_source_set_enabled(
	    sources, UI_HIGHLIGHT_SOURCE_TUTORIAL, false);
	assert(sources == UI_HIGHLIGHT_SOURCE_AVAILABILITY);
	sources = ui_highlight_source_set_enabled(
	    sources, UI_HIGHLIGHT_SOURCE_AVAILABILITY, false);
	assert(sources == 0);
}

static void test_operand_highlight_configuration(void)
{
	assert(!lp_are_operand_highlights_enabled());
	lp_configure(false, false, false, false, true);
	assert(lp_are_operand_highlights_enabled());
	lp_configure(false, false, false, false, false);
	assert(!lp_are_operand_highlights_enabled());
}

int main(void)
{
	test_operand_availability();
	test_highlight_source_composition();
	test_operand_highlight_configuration();
	puts("operand highlight tests passed");
	return 0;
}