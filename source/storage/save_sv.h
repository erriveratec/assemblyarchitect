#ifndef SAVE_SV_H
#define SAVE_SV_H

#include <stdbool.h>

typedef enum architect_id_t {
	SV_ARCHITECT_NONE = 0,
	SV_ARCHITECT_EXECUTOR_X = 1,
	SV_ARCHITECT_OPERATIVE_Y = 2,
	SV_ARCHITECT_HANDLER_Z = 3
} architect_id_t;



void sv_load_architect(int player_id, bool *levels_array);
void sv_save_init_default();

#endif
