#ifndef STAGE_BUTTONS_SB_H
#define STAGE_BUTTONS_SB_H

#include <stdlib.h>
#include <stdio.h>
#include "button_bt.h"

enum stage_buttons_e {
	INVALID,
	STOP,
	STEP,
	PLAY,
	FAST
};

extern texture_t *stop_button;
extern texture_t *fast_button;
extern texture_t *play_button;
extern texture_t *step_forward_button;
extern texture_t *return_button;
extern texture_t *reset_button;


void sb_draw_ret_btn();
void sb_init_ret_btn();

void sb_init_stage_btns();
void sb_draw_stage_buttons(int code_size);
bool sb_stage_btn_clicked();
int identify_clicked_stage_button();
bool sb_check_clicked_ret_button();

int sb_get_play_button_member(int member);
SDL_Rect sb_get_sb_rect(int member);

void toggle_escape_menu();
void sb_display_escape_menu(bool menu_variable_state);
void set_quit_game();
bool get_quit_game_value();
bool sb_get_escape_state();
void sb_init_escape_menu();

void sb_set_step_btns_avail(bool state);

void sb_draw_rst_btn();
void sb_init_rst_btn();
bool sb_chk_click_rst_btn();
void sb_init_rst_menu();
void sb_set_rst_menu(bool state);
bool sb_chk_rst_menu_state();
void sb_display_rst_menu(bool show_menu);
bool sb_chk_rst_menu_btns(bool show_menu);
SDL_Rect sb_get_stage_btns();


#endif
