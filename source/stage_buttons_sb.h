#ifndef STAGE_BUTTONS_SB_H
#define STAGE_BUTTONS_SB_H

#include <stdlib.h>
#include <stdio.h>
#include "button_bt.h"

void sb_draw_stage_buttons(int code_size);
void sb_draw_return_button();
void sb_initialize_stage_buttons();
void sb_init_return_button();
bool sb_check_clicked_stage_button();
bool sb_check_clicked_ret_button();
int identify_clicked_stage_button();
int sb_get_play_button_member(int member);
void toggle_escape_menu();
void display_escape_menu(bool menu_variable_state);
void set_quit_game();
bool get_quit_game_value();
bool get_escape_menu_state();
void init_escape_menu();

enum stage_buttons_e {
	INVALID,
	PLAY,
	STOP,
	FORWARD,
	BACKWARD
};

extern texture_t *stop_button;
extern texture_t *step_back_button;
extern texture_t *play_button;
extern texture_t *step_forward_button;
extern texture_t *return_button;

#endif
