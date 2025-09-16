#ifndef DIMENSIONS_DM_H
#define DIMENSIONS_DM_H

#include<stdio.h>
#include<stdlib.h>
#include <SDL.h>

enum resolutionos{
	R1600X900	
};

void dm_set_screen_resolution(int resolution_id);

int get_sel_level_offset_y();
int dm_get_h_big_text();
int dm_get_h_bottom_msg();
int dm_get_h_msg();
int dm_get_h_error_msg();
int dm_get_h_stage_elements_titles();
int dm_get_h_stage_titles();
int dm_get_h_line_number();
int dm_get_w_code_box_text();

SDL_Rect dm_get_studio_name_msg_box();
SDL_Rect dm_get_game_title_box();
SDL_Rect dm_get_press_space_box();
SDL_Rect dm_get_upper_title_box();

SDL_Rect dm_get_p1_button_box();
SDL_Rect dm_get_p2_button_box();
SDL_Rect dm_get_p3_button_box();

SDL_Rect dm_get_escape_menu_box();
SDL_Rect dm_get_escape_b1_box();
SDL_Rect dm_get_escape_b2_box();
SDL_Rect dm_get_escape_b3_box();

SDL_Rect dm_get_box_msg_wh();
SDL_Rect dm_get_code_button_wh();
SDL_Rect dm_get_arrow_wh();
SDL_Rect dm_get_vbox_wh();
SDL_Rect dm_get_vbox_val_wh();
SDL_Rect dm_get_avatar_wh();

SDL_Rect dm_get_level_button_box();
SDL_Rect dm_get_return_button_box();

SDL_Rect dm_get_stage_registers_box();
SDL_Rect dm_get_stage_ibox();
SDL_Rect dm_get_stage_obox();
SDL_Rect dm_get_stage_code_box();
SDL_Rect dm_get_stage_instruction_box();
SDL_Rect dm_get_stage_input_buffer_box();
SDL_Rect dm_get_stage_output_buffer_box();
SDL_Rect dm_get_stage_ib_text_box();
SDL_Rect dm_get_stage_ob_text_box();
SDL_Rect dm_get_stage_buttons();

int dm_get_screen_width();
int dm_get_screen_height();
int dm_get_y_hidden_stage_buttons();
int dm_get_w_stage_rail();
int dm_get_w_stage_rail_end();
int dm_get_ofs_stage_reg_box();
int dm_get_ofs_code_box_border();
int dm_get_ofs_code_number();
int dm_get_ofs_code_op1();
int dm_get_ofs_code_op2();
int dm_get_ofs_code_comma();
int dm_get_ofs_buffer_value_box();
int dm_get_ofs_between_value_box();
int dm_get_ofs_reg_value_box();
int dm_get_ofs_space_stage_buttons();

SDL_Rect dm_get_text_box_big();
SDL_Rect dm_get_text_box_error();
SDL_Rect dm_get_text_box_upper();
SDL_Rect dm_get_text_box_center();
SDL_Rect dm_get_text_box_lower();
SDL_Rect dm_get_text_box_ins();
SDL_Rect dm_get_text_box_code();
SDL_Rect dm_get_text_box_stagebutton();

SDL_Rect dm_get_text_box_result();
SDL_Rect dm_get_text_box_result_text1();//Result win text
SDL_Rect dm_get_text_box_result_text2();//Error box
SDL_Rect dm_get_text_box_result_but1();//Back in win condition
SDL_Rect dm_get_text_box_result_but2();//Continue in win condition
SDL_Rect dm_get_text_box_result_but3();//Back in error condition

#endif
