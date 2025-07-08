#ifndef LEVELS_H
#define LEVELS_H

#include <stdbool.h>
#include "list.h"

// Multiple states of the game.

#define LV_LEVEL_QUANTITY 10

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
	LV_LEVEL_MAX,
	LV_QUIT,
	LV_STUDIO_SCREEN,
	LV_TITLE_SCREEN,
	LV_SELECT_PLAYER_SCREEN,
	LV_LEVEL_SELECTION

};

void lv_create_win_list();
void lv_destroy_win_list();
void reset_win_list();
void print_win_list();
bool check_if_win();
void lv_generate_win_condition_list(int level);


#endif
