#include "gameplay/level_presentation_lp.h"

static bool g_register_hints_enabled = false;
static bool g_buffer_hints_enabled = false;
static bool g_immediate_hints_enabled = false;
static bool g_flag_boxes_visible = false;

void lp_configure(
    bool register_hints_enabled,
    bool buffer_hints_enabled,
    bool immediate_hints_enabled,
    bool flag_boxes_visible
)
{
    g_register_hints_enabled =
        register_hints_enabled;

    g_buffer_hints_enabled =
        buffer_hints_enabled;

    g_immediate_hints_enabled =
        immediate_hints_enabled;

    g_flag_boxes_visible =
        flag_boxes_visible;
}

bool lp_are_register_hints_enabled(void)
{
    return g_register_hints_enabled;
}

bool lp_are_buffer_hints_enabled(void)
{
    return g_buffer_hints_enabled;
}

bool lp_are_immediate_hints_enabled(void)
{
    return g_immediate_hints_enabled;
}

bool lp_are_flag_boxes_visible(void)
{
    return g_flag_boxes_visible;
}