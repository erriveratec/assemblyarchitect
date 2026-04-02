#ifndef GAME_MECHANICS_MC_H
#define GAME_MECHANICS_MC_H
#include "draw_dw.h"
#include "button_bt.h"
#include "code_window_cw.h"

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

enum WinMenuStates{
	NO_BTN_PRESSED,
	BACK_BTN_PRESSED,
	CONT_BTN_PRESSED
};

void mc_init_avatar();
void mc_draw_avatar();
void mc_reset_avatar();
void mc_run_code();
int mc_get_operation_flag();
void mc_reset_invalid_operation_flag();
void mc_display_operation_handler(int id);
void mc_init_errors_texture();
void mc_destroy_avatar_textures();
void mc_set_run_ended(bool state);
bool mc_get_run_ended();
void mc_set_step_ended(bool state);
bool mc_get_step_ended();
void mc_set_rst_lvl(bool state);
bool mc_get_rst_lvl();
void mc_set_operation_flag(int flag_id);
int mc_get_op_menu_btn_state();
void mc_set_op_menu_btn_state(int state);
#endif
