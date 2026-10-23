#ifndef GAME_MECHANICS_MC_H
#define GAME_MECHANICS_MC_H
#include "draw_dw.h"

extern texture_t *g_exec_arrow_texture;

enum operations{
	NO_OPERATION,
	INPUT_BUFFER_EMPTY,
	REG_VALUE_INVALID,
	FLAG_VALUE_INVALID,
	INVALID_OUTPUT_VALUE,
	UNPROCESSED_IB_VALUES,
	EXCEEDS_CODE_LIMIT,
	OUTPUT_BUFFER_INCOMPLETE,
	MC_WIN, 
	OPERATION_MAX 
};

// Escape option menu variables

void mc_init_avatar(void);
void mc_draw_avatar(void);
void mc_reset_avatar(void);
void mc_run_code(void);
int mc_get_operation_flag(void);
void mc_reset_invalid_operation_flag(void);
void mc_destroy_avatar_textures(void);
void mc_set_run_ended(bool state);
bool mc_get_run_ended(void);
void mc_set_step_ended(bool state);
bool mc_get_step_ended(void);
void mc_set_operation_flag(int flag_id);
bool mc_is_executing(void);
void mc_start_execution(bool state);
#endif
