#include <stdio.h>
#include <stdlib.h>

#include "code_state_cs.h"
#include "code_window_cw.h"
#include "game_mechanics_mc.h"

static code_line_t *g_hold_line;

bool chk_player_holds_line(void);

void lv_set_hold_line(code_line_t *line)
{
    g_hold_line = line;
}

code_line_t *get_hold_line(void)
{
    return g_hold_line;
}

bool chk_player_holds_line(void)
{
    return g_hold_line != NULL;
}

cs_context_t cs_capture_context(void)
{
    cs_context_t context = {0};

    context.code_size = cw_get_code_list_size();
    context.operation_flag = mc_get_operation_flag();
    context.holding_instruction = chk_player_holds_line();
    context.held_instruction_id = -1;
    context.first_operand_1_id = -1;
    context.first_operand_2_id = -1;
    context.last_operand_1_id = -1;
    context.last_operand_2_id = -1;
    context.last_line_state = -1;

    code_line_t *held_line = get_hold_line();
    if (held_line != NULL && held_line->ins != NULL) {
        context.held_instruction_id = held_line->ins->id;
    }

    if (context.code_size > 0) {
        code_line_t *first_line = cw_get_code_line_at_pos(0);

        if (first_line != NULL) {
            if (first_line->op1 != NULL) {
                context.first_operand_1_id = first_line->op1->id;
            }

            if (first_line->op2 != NULL) {
                context.first_operand_2_id = first_line->op2->id;
            }
        }
    }

    if (context.code_size > 0) {
        code_line_t *last_line =
            cw_get_code_line_at_pos(context.code_size - 1);

        if (last_line != NULL) {
            context.last_line_state = last_line->state;

            if (last_line->op1 != NULL) {
                context.last_operand_1_id = last_line->op1->id;
            }

            if (last_line->op2 != NULL) {
                context.last_operand_2_id = last_line->op2->id;
            }
        }
    }

    context.code_sorted = cw_check_code_sorted();
    context.operand_pending = cw_is_operand_pending();
    context.operand_1_pending = cw_is_operand_1_pending();
    context.operand_2_pending = cw_is_operand_2_pending();
    context.playing = mc_is_executing();

    return context;
}

cs_state_t cs_get_state(void)
{
    cs_context_t context = cs_capture_context();
    cs_state_t state = CS_STATE_UNCLASSIFIED;

    return state;
}

