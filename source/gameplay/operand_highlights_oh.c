#include "gameplay/operand_highlights_oh.h"

#include "buffers_bf.h"
#include "code_line_cl.h"
#include "code_window_cw.h"
#include "gameplay/interaction_rules_ir.h"
#include "gameplay/ui_highlight_source.h"
#include "immediates_im.h"
#include "list.h"
#include "registers_rg.h"

static bool operand_is_compatible(int operand_id, code_line_t *line)
{
	operand_t candidate = {.id = operand_id};
	return cl_is_op_compatible(&candidate, line);
}

/* Compatibility is the same instruction/operand predicate used by selection. */
operand_availability_t oh_get_operand_availability(
	code_line_t *line,
	const int *register_ids,
	size_t register_count,
	bool registers_selectable,
	bool buffers_selectable,
	bool immediates_visible)
{
	operand_availability_t availability = {0};
	if (line == NULL || line->ins == NULL) {
		return availability;
	}

	if (registers_selectable && register_ids != NULL) {
		for (size_t index = 0; index < register_count; index++) {
			if (operand_is_compatible(register_ids[index], line)) {
				availability.registers = true;
				break;
			}
		}
	}

	if (buffers_selectable) {
		availability.input_buffer = operand_is_compatible(IB, line);
		availability.output_buffer = operand_is_compatible(OB, line);
	}

	if (immediates_visible) {
		for (int index = 0; index < 20; index++) {
			if (operand_is_compatible(IMMUP0 + index, line)) {
				availability.immediates = true;
				break;
			}
		}
	}

	return availability;
}

void oh_clear(void)
{
	rg_set_register_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, false);
	bf_set_buffer_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY,
	                               false, false);
	im_set_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, false);
}

void oh_update(bool enabled, bool selection_possible)
{
	if (!enabled || !selection_possible || !cw_check_code_sorted()) {
		oh_clear();
		return;
	}

	code_line_t *line = cw_get_code_line_pending_operand();
	int register_ids[REG_MAX - REG_MIN - 1];
	size_t register_count = 0;
	List *registers = rg_get_register_list();
	if (registers != NULL) {
		LIST_FOREACH(registers, first, next, current) {
			reg_t *reg = current->value;
			if (register_count < sizeof(register_ids) / sizeof(register_ids[0])) {
				register_ids[register_count++] = reg->id;
			}
		}
	}
	operand_availability_t availability = oh_get_operand_availability(
	    line, register_ids, register_count, ir_is_register_selectable(),
	    ir_is_buffer_selectable(), im_are_imm_up_available());
	rg_set_register_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY,
	                                 availability.registers);
	bf_set_buffer_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY,
	                               availability.input_buffer,
	                               availability.output_buffer);
	im_set_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY,
	                        availability.immediates);
}