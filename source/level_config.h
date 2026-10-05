#ifndef LEVEL_CONFIG_H
#define LEVEL_CONFIG_H

#include <stddef.h>

int lc_load_level(int level_id);
/* Returns SUCCESS for a configured message; safely truncates to buffer_size. */
int lc_load_hover_message(int level_id, char *buffer, size_t buffer_size);

#endif
