#ifndef LEVEL_PRESENTATION_LP_H
#define LEVEL_PRESENTATION_LP_H

#include <stdbool.h>

void lp_configure(
    bool register_hints_enabled,
    bool buffer_hints_enabled,
    bool immediate_hints_enabled,
    bool flag_boxes_visible
);

bool lp_are_register_hints_enabled(void);
bool lp_are_buffer_hints_enabled(void);
bool lp_are_immediate_hints_enabled(void);
bool lp_are_flag_boxes_visible(void);

#endif