#ifndef INSTRUCTION_FEEDBACK_IF_H
#define INSTRUCTION_FEEDBACK_IF_H

#include <stdbool.h>

#include "code_edit_ce.h"

/* Loads configured Instruction Feedback definitions and their styled textures. */
bool ifb_init(const char *configuration_path);
/* Releases cached styled-text textures and the owned configuration repository. */
void ifb_destroy(void);
/* Opens the configured case for a rejected operand choice. */
bool ifb_open(const ce_diagnostic_t *diagnostic);
bool ifb_is_active(void);
/* Captures all pointer input while active. Returns true if this frame was owned. */
bool ifb_update(bool input_enabled);
void ifb_draw(void);

#endif
