#ifndef GAME_MECHANICS_MC_H
#define GAME_MECHANICS_MC_H
#include "draw_dw.h"
#include "button_bt.h"
#include "code_window_cw.h"

extern texture_t *g_exec_arrow_texture;

enum invalid_operations{
	NO_INVALID_OPERATION,
	INPUT_BUFFER_EMPTY,
	REG_VALUE_INVALID,
	INVALID_OUTPUT_VALUE,
	UNPROCESSED_IB_VALUES,
	EXCEEDS_CODE_LIMIT,
	OUTPUT_BUFFER_INCOMPLETE,
	INVALID_OPERATION_MAX
};

void mc_init_avatar();
void mc_draw_avatar();
void mc_reset_avatar();
bool mc_run_code();
int mc_get_operation_flag();
void mc_reset_invalid_operation_flag();
bool mc_invalid_operation_handler(int id);
void mc_init_errors_texture();
void mc_destroy_avatar_textures();
#endif
