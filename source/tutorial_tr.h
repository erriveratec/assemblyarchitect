#ifndef TUTORIAL_TR_H
#define TUTORIAL_TR_H

#include <stdbool.h>

#define TUTORIAL_STEP_NAME_LENGTH 64
#define TUTORIAL_STEP_TEXT_LENGTH 256
#define TUTORIAL_MAX_ARROWS 4

#include "gameplay/code_state_cs.h"

typedef enum tutorial_box_t {
    TUTORIAL_BOX_BIG,
    TUTORIAL_BOX_UPPER,
    TUTORIAL_BOX_UPPER_RIGHT,
    TUTORIAL_BOX_CENTER,
    TUTORIAL_BOX_CENTER_RIGHT,
    TUTORIAL_BOX_LOWER,
    TUTORIAL_BOX_CODE,
    TUTORIAL_BOX_INSTRUCTION
} tutorial_box_t;

typedef enum tutorial_header_t {
    TUTORIAL_HEADER_NONE,
    TUTORIAL_HEADER_SYSTEM_MESSAGE,
    TUTORIAL_HEADER_SYSTEM_NOTICE,
    TUTORIAL_HEADER_SYSTEM_WARNING,
    TUTORIAL_HEADER_INSTRUCTION
} tutorial_header_t;

typedef enum tutorial_dismiss_t {
    TUTORIAL_DISMISS_NONE,
    TUTORIAL_DISMISS_MOUSE_PRESS,
    TUTORIAL_DISMISS_MOUSE_RELEASE,
    TUTORIAL_DISMISS_OPERATION_ERROR
} tutorial_dismiss_t;

typedef enum tutorial_edit_exception_t {
    TUTORIAL_EDIT_EXCEPTION_UNSET = -1,
    TUTORIAL_EDIT_EXCEPTION_NONE,
    TUTORIAL_EDIT_EXCEPTION_INSTRUCTION,
    TUTORIAL_EDIT_EXCEPTION_LAST_LINE,
    TUTORIAL_EDIT_EXCEPTION_LAST_OPERAND_2
} tutorial_edit_exception_t;

#define TUTORIAL_MAX_ARROWS 4

typedef struct tutorial_step_t {
    char name[TUTORIAL_STEP_NAME_LENGTH];
    char text[TUTORIAL_STEP_TEXT_LENGTH];
    tutorial_box_t box;
    tutorial_header_t header;
    tutorial_dismiss_t dismiss;
    int arrow_ids[TUTORIAL_MAX_ARROWS];
    int arrow_count;
    int when_code_size;
    int when_code_size_max;
    int when_holding;
    int when_held_instruction_id;
    int when_held_instruction_not_id;
    int when_operand_pending;
    int when_operand_1_pending;
    int when_operand_2_pending;
    int when_first_operand_1_id;
    int when_first_operand_2_id;
    int when_last_operand_1_id;
    int when_last_operand_2_id;
    int when_last_line_state;
    int when_code_sorted;
    int when_play_state;
    int when_won;
    int when_operation_flag;
    int when_operation_error;
    int effect_code_editable;
    tutorial_edit_exception_t effect_code_editable_exception;
    int effect_buffer_selectable;
    int effect_register_selectable;
    int effect_arrange_enabled;
    int effect_delete_enabled;
    bool active;
} tutorial_step_t;

typedef struct tutorial_state_t {
    int code_size;
    bool holding;
    bool operand_pending;
    bool code_sorted;
    bool play_state;
    int operation_flag;
} tutorial_state_t;

bool tr_load_level(int level_id);
void tr_clear(void);
int tr_get_step_count(void);
const tutorial_step_t *tr_get_step(int index);
const tutorial_step_t *tr_get_named_step(const char *name);
bool tr_is_active(const char *name);
void tr_deactivate(const char *name);
void tr_render_step(const char *name);


bool tr_step_matches_current_state(
    const char *name,
    const cs_context_t *context
);

const tutorial_step_t *tr_get_matching_step(
    const cs_context_t *context
);

const tutorial_step_t *tr_update(
    const cs_context_t *context
);


#endif
