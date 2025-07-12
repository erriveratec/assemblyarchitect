#ifndef GAME_MECHANICS_H
#define GAME_MECHANICS_H
#include "draw_dw.h"
#include "button_bt.h"
#include "code_window_cw.h"

enum invalid_operatios{
	NO_INVALID_OPERATION,
	REG_VALUE_NOT_INITIALIZED
};

void mc_draw_avatar();
void mc_reset_avatar();
bool mc_run_code();
int mc_get_invalid_operation_flag();

#endif
