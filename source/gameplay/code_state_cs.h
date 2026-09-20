#ifndef  CODE_STATE_CS_H
#define  CODE_STATE_CS_H

#include <stdbool.h>
#include "code_line_cl.h"


typedef enum cs_state_t {
    CS_STATE_UNCLASSIFIED = 0,

    CS_STATE_CODE_EMPTY,
    CS_STATE_HOLDING_FIRST_INSTRUCTION,

    CS_STATE_ONE_LINE_UNSORTED,
    CS_STATE_ONE_LINE_HOLDING,
    CS_STATE_ONE_LINE_MISSING_OPERAND_1,
    CS_STATE_ONE_LINE_MISSING_OPERAND_2,
    CS_STATE_ONE_LINE_COMPLETE,

    CS_STATE_HOLDING_SECOND_INSTRUCTION,

    CS_STATE_TWO_LINES_UNSORTED,
    CS_STATE_TWO_LINES_HOLDING,
    CS_STATE_TWO_LINES_MISSING_OPERAND_1,
    CS_STATE_TWO_LINES_MISSING_OPERAND_2,
    CS_STATE_TWO_LINES_COMPLETE,

    CS_STATE_CODE_OVER_LIMIT,
    CS_STATE_PLAYING,
    CS_STATE_EXECUTION_ERROR,
    CS_STATE_LEVEL_COMPLETE
} cs_state_t;

typedef struct cs_context_t {
    int code_size;
    int instruction_limit;
    int operation_flag;

    bool holding_instruction;
    bool code_sorted;
    bool operand_pending;
    bool operand_1_pending;
    bool operand_2_pending;
    bool playing;
    bool won;
} cs_context_t;

cs_context_t cs_capture_context(void);
//cs_state_t cs_classify(const cs_context_t *context);
cs_state_t cs_get_state(void);


// For compilation purposes, this has to be removed later
bool chk_player_holds_line();
code_line_t *get_hold_line();
#endif
