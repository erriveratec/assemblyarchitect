#ifndef LEVELS_LV_H
#define LEVELS_LV_H

#include <stdbool.h>
#include "list.h"
#include "draw_dw.h"
#include "code_line_cl.h"

// Multiple states of the game.

#define LV_LEVEL_QUANTITY 40


enum LevelStates{
	LV_LEVEL_MIN,
	LV_LEVEL_1,
	LV_LEVEL_2,
	LV_LEVEL_3,
	LV_LEVEL_4,
	LV_LEVEL_5,
	LV_LEVEL_6,
	LV_LEVEL_7,
	LV_LEVEL_8,
	LV_LEVEL_9,
	LV_LEVEL_10,
	LV_LEVEL_11,
	LV_LEVEL_12,
	LV_LEVEL_13,
	LV_LEVEL_14,
	LV_LEVEL_15,
	LV_LEVEL_16,
	LV_LEVEL_17,
	LV_LEVEL_18,
	LV_LEVEL_19,
	LV_LEVEL_20,
	LV_LEVEL_21,
	LV_LEVEL_22,
	LV_LEVEL_23,
	LV_LEVEL_24,
	LV_LEVEL_25,
	LV_LEVEL_26,
	LV_LEVEL_27,
	LV_LEVEL_28,
	LV_LEVEL_29,
	LV_LEVEL_30,
	LV_LEVEL_31,
	LV_LEVEL_32,
	LV_LEVEL_33,
	LV_LEVEL_34,
	LV_LEVEL_35,
	LV_LEVEL_36,
	LV_LEVEL_37,
	LV_LEVEL_38,
	LV_LEVEL_39,
	LV_LEVEL_40,
	LV_LEVEL_MAX,
	LV_NO_LEVEL,
	LV_NO_STAGE,
	LV_QUIT,
	LV_INIT_STUDIO_SCREEN,
	LV_STUDIO_SCREEN,
	LV_TITLE_SCREEN,
	LV_SELECT_PLAYER_SCREEN,
	LV_INIT_LEVEL_SELECTION,
	LV_LEVEL_SELECTION,
	LV_INITIALIZE_LEVEL,
	LV_PLAY_LEVEL
};


void lv_upd_level_assets(int level);
void lv_set_level_win_condition(char *win_condition);
void lv_reset_level_win_condition();
void lv_init_level_assets(int level);
void lv_create_win_list();
void lv_destroy_win_list();
void lv_reset_win_list();
void lv_print_win_list();
bool lv_check_if_win();
void lv_reset_win_condition();
bool lv_evaluate_output_correctness();
void lv_set_level_instructions_limit(int limit);
int lv_get_level_instructions_limit();
void lv_level_drawings(int level);
void init_level(int level_id);
bool lv_is_code_editable();
bool lv_is_buf_selectable();
bool lv_is_reg_selectable();
int lv_get_win_list_size();
void lv_set_hold_line(code_line_t *line);
void lv_set_play_state(bool state);
void lv_set_op_flag_state(bool state);

#endif
