#ifndef LEVELS_H
#define LEVELS_H

#include <stdbool.h>
#include "list.h"

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
	LV_QUIT,
	LV_STUDIO_SCREEN,
	LV_TITLE_SCREEN,
	LV_SELECT_PLAYER_SCREEN,
	LV_LEVEL_SELECTION

};

void lv_create_win_list();
void lv_destroy_win_list();
void lv_reset_win_list();
void lv_print_win_list();
bool lv_check_if_win();
void lv_generate_win_condition_list(int level);
bool lv_evaluate_output_correctness();
void lv_set_level_instructions_limit(int limit);
int lv_get_level_instructions_limit();
void lv_level_drawings(int level);
int lv_level_1_tutorial_instruction_select();
int lv_level_1_tutorial_drop_instruction();

#endif
