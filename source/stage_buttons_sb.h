#ifndef STAGE_BUTTONS_SB_H
#define STAGE_BUTTONS_SB_H

#include <stdlib.h>
#include <stdio.h>
#include "draw_dw.h"

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
void sb_draw_stage_btns(int code_size);
bool sb_chk_click_stage_btn();
int identify_clicked_stage_button();
bool sb_chck_rel_ret_btn();
SDL_Rect sb_get_stage_btns();

int sb_get_play_button_member(int member);
SDL_Rect sb_get_sb_rect(int member);

void set_quit_game();
bool get_quit_game_value();

void sb_set_step_btns_avail(bool state);

void sb_draw_rst_btn();
void sb_init_rst_btn();

bool sb_chk_rel_rst_btn();

bool sb_chk_rst_esc_menu_active();
bool sb_chk_hov_rst_ret_btns();

#endif
