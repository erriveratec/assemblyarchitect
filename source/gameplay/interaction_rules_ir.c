#include "interaction_rules_ir.h"
#include "code_window_cw.h"
#include "instruction_window_iw.h"
#include <assert.h>

static bool g_buffer_selectable = true;
static bool g_register_selectable = true;
static bool g_arrange_enabled = true;
static bool g_delete_enabled = true;
static bool g_code_editable = true;
static int g_code_editable_exception = IR_NO_EXCEPTION;

static ir_rules_t g_base_rules = {
    .instruction_limit = 0,
    .code_editable = true,
    .code_editable_exception = IR_NO_EXCEPTION,
    .buffer_selectable = true,
    .register_selectable = true,
    .arrange_enabled = true,
    .delete_enabled = true
};

static ir_rules_t g_current_rules = {
    .instruction_limit = 0,
    .code_editable = true,
    .code_editable_exception = IR_NO_EXCEPTION,
    .buffer_selectable = true,
    .register_selectable = true,
    .arrange_enabled = true,
    .delete_enabled = true
};

void ir_set_base_rules(const ir_rules_t *rules)
{
    assert(rules != NULL);

    g_base_rules = *rules;
    g_current_rules = *rules;
}

void ir_restore_base_rules(void)
{
    g_current_rules = g_base_rules;
}

const ir_rules_t *ir_get_base_rules(void)
{
    return &g_base_rules;
}

const ir_rules_t *ir_get_current_rules(void)
{
    return &g_current_rules;
}


void ir_set_instruction_limit(int limit)
{
    assert(limit >= 0);

    g_current_rules.instruction_limit = limit;
}

int ir_get_instruction_limit(void)
{
    return g_current_rules.instruction_limit;
}

bool ir_is_over_instruction_limit(int instruction_count)
{
    return instruction_count >
           g_current_rules.instruction_limit;
}

void ir_set_code_editable(bool editable, int exception)
{
    g_code_editable = editable;
    g_code_editable_exception = exception;
}

bool ir_is_code_editable(void)
{
    int exception = g_code_editable_exception;

    if (exception == IR_NO_EXCEPTION) {
        return g_code_editable;
    }

    if (exception == IR_INSTRUCTION_EXCEPTION &&
        iw_chk_click_ins()) {
        return true;
    }

    if (exception == IR_OPERAND_2_LAST) {
        int code_size =
            cw_get_code_list_size();

        return cw_chk_click_code_op2(
            code_size
        );
    }

    if (cw_chk_click_code()) {
        code_line_t *line =
            cw_get_clicked_code();

        if (line == NULL) {
            return g_code_editable;
        }

        int position =
            cw_get_code_line_pos_by_ptr(line);

        position++;

        if (position == exception) {
            return true;
        }
    }

    return g_code_editable;
}

void ir_set_delete_enabled(bool enabled)
{
    g_delete_enabled = enabled;
}

bool ir_is_delete_enabled(void)
{
    return g_delete_enabled;
}

void ir_set_arrange_enabled(bool enabled)
{
    g_arrange_enabled = enabled;
}

bool ir_is_arrange_enabled(void)
{
    return g_arrange_enabled;
}

void ir_set_register_selectable(bool selectable)
{
    g_register_selectable = selectable;
}

bool ir_is_register_selectable(void)
{
    return g_register_selectable;
}

void ir_set_buffer_selectable(bool selectable)
{
    g_buffer_selectable = selectable;
}

bool ir_is_buffer_selectable(void)
{
    return g_buffer_selectable;
}