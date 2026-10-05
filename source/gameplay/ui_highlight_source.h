#ifndef UI_HIGHLIGHT_SOURCE_H
#define UI_HIGHLIGHT_SOURCE_H

#include <stdbool.h>

/* Separate owners can add or clear their bit without disturbing other sources. */
typedef enum ui_highlight_source_t {
	UI_HIGHLIGHT_SOURCE_TUTORIAL = 1u << 0,
	UI_HIGHLIGHT_SOURCE_AVAILABILITY = 1u << 1
} ui_highlight_source_t;

static inline unsigned int ui_highlight_source_set_enabled(
	unsigned int sources,
	ui_highlight_source_t source,
	bool enabled)
{
	if (enabled) {
		return sources | (unsigned int)source;
	}
	return sources & ~(unsigned int)source;
}

#endif