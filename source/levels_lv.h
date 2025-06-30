#ifndef LEVELS_H
#define LEVELS_H

#include <stdbool.h>
#include "list.h"

// Multiple states of the game.

#define LV_LEVEL_QUANTITY 10

enum LevelStates{
	LEVEL_MIN,
	LEVEL_1,
	LEVEL_MAX,
	QUIT,
	STUDIO_SCREEN,
	TITLE_SCREEN,
	SELECT_PLAYER_SCREEN,
	LEVEL_SELECTION

};

void lv_create_win_list();
void lv_destroy_win_list();
void reset_win_list();
void print_win_list();
bool check_if_win();
void lv_generate_win_condition_list(int level);


#endif
