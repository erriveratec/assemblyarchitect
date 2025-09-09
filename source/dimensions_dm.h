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
int dm_get_h_big_msg();
int dm_get_h_bottom_msg();
int dm_get_h_msg();
int dm_get_h_error_msg();
int dm_get_h_stage_titles();

SDL_Rect dm_get_studio_name_msg_box();
SDL_Rect dm_get_game_title_box();
SDL_Rect dm_get_press_space_box();
SDL_Rect dm_get_select_player_box();
SDL_Rect dm_get_select_level_box();

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

int dm_get_w_stage_rail();
int dm_get_w_stage_rail_end();
int dm_get_ofs_stage_buffer_value();
int dm_get_ofs_stage_reg_box();
int dm_get_ofs_code_number();
int dm_get_ofs_code_op1();
int dm_get_ofs_code_op2();

SDL_Rect dm_get_arrow_box();
SDL_Rect dm_get_arrow_ins_box();

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

/*******************************RESOLUTION*************************************/
#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 900

#define STR_BACK "Back"
#define STR_CONT "Continue"

#define CODE_BOX_OFFSET 25

#define TEXT_BOX_WIDTH 300
#define TEXT_BOX_HEIGHT 30

#define MOVEMENT_DELTA 30


/*************************INPUT/OUTPUT BUFFERS*********************************/
// The height of the text for the input/output buffers


#define BUFFER_TEXT_H 42

#define BUFFER_BOX_H 75

#define INPUT_BUFFER_TEXT_X (SCREEN_WIDTH*4/6)
#define INPUT_BUFFER_TEXT_Y 0

#define OUTPUT_BUFFER_TEXT_X (SCREEN_WIDTH*4/6)
#define OUTPUT_BUFFER_TEXT_Y (SCREEN_HEIGHT - BUFFER_TEXT_H)

// The dimensions of the input/output boxes
#define BUFFER_BOX_X (SCREEN_WIDTH*4/6)
#define INPUT_BUFFER_BOX_Y 40

#define OUTPUT_BUFFER_BOX_Y (SCREEN_HEIGHT - BUFFER_TEXT_H - BUFFER_BOX_H)
#define OUTPUT_BUFFER_WIN_X 10000


// x and y offsets for the numbers and the box
#define BUFFER_VALUE_OFFSET_X 20
#define BUFFER_VALUE_OFFSET_Y 20
#define BETWEEN_NUMBERS_OFFSET 30

#define BUFFER_MOVEMENT_DELTA 5

// The position of the register text
#define REG_TEXT_Y_OFFSET 5

#define REG_VBOX_X REG_BOX_X + 190
#define REG_VBOX_OFFSET 10

// The between registers offset
#define BETWEEN_REG_OFFSET 10

/*****************************CODE BUTTONS*************************************/
// Dimensions of the buttons that will be used in code
#define STAGE_NAME_H 55

#define ADDR_BUTTON_W 120

/******************************STAGE BUTTONS***********************************/
#define STAGE_BUTTON_W 40
#define STAGE_BUTTON_H 40
#define STAGE_BUTTON_X ((SCREEN_WIDTH*2/6) + ((SCREEN_WIDTH/6) - \
						4*STAGE_BUTTON_W)/4)
#define STAGE_BUTTON_Y SCREEN_HEIGHT - 60
#define STAGE_BUTTON_HIDDEN_Y (SCREEN_HEIGHT) + 150
#define STAGE_BUTTONS_MOVEMENT_DELTA 10
#define BUTTONS_SPACE 20

/*****************************POINTING ARROWS**********************************/
#define ARROW_H 30
#define ARROW_W 30
/******************************VALUE BOXES*************************************/

#define VALUE_BOX_H 40
#define VALUE_BOX_W 50
#define VALUE_H 35
#define VALUE_W 50


#endif
