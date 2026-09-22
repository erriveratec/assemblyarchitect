#ifndef INTERACTION_RULES_IR_H
#define INTERACTION_RULES_IR_H

#include <stdbool.h>


enum {
    IR_NO_EXCEPTION = -1,
    IR_INSTRUCTION_EXCEPTION = -2,
    IR_OPERAND_1_LAST = -3,
    IR_OPERAND_2_LAST = -4
};

typedef struct ir_rules_t {
    int instruction_limit;

    bool code_editable;
    int code_editable_exception;

    bool buffer_selectable;
    bool register_selectable;
    bool arrange_enabled;
    bool delete_enabled;
} ir_rules_t;

void ir_set_instruction_limit(int limit);
int ir_get_instruction_limit(void);
bool ir_is_over_instruction_limit(int instruction_count);

void ir_set_base_rules(const ir_rules_t *rules);
void ir_restore_base_rules(void);

const ir_rules_t *ir_get_base_rules(void);
const ir_rules_t *ir_get_current_rules(void);

void ir_set_code_editable(
    bool editable,
    int exception
);

bool ir_is_code_editable(void);

void ir_set_buffer_selectable(bool selectable);
bool ir_is_buffer_selectable(void);

void ir_set_register_selectable(bool selectable);
bool ir_is_register_selectable(void);

void ir_set_arrange_enabled(bool enabled);
bool ir_is_arrange_enabled(void);

void ir_set_delete_enabled(bool enabled);
bool ir_is_delete_enabled(void);

#endif