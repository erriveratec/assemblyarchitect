#ifndef GAME_MECHANICS_H
#define GAME_MECHANICS_H
#include "draw_dw.h"
#include "button_bt.h"
#include "code_window_cw.h"

extern texture_t *right_arrow;

enum invalid_operatios{
	NO_INVALID_OPERATION,
	INPUT_BUFFER_EMPTY,
	REG_VALUE_INVALID
};

void mc_draw_avatar();
void mc_reset_avatar();
bool mc_run_code();
int mc_get_operation_flag();
void mc_reset_invalid_operation_flag();
void mc_draw_execution_arrow();
void mc_reset_execution_arrow();
void mc_hide_execution_arrow();
bool mc_invalid_operation_handler(int id);
#endif
