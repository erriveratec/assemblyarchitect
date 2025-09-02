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

SDL_Rect dm_get_code_button_box();
SDL_Rect dm_get_box_msg();

SDL_Rect dm_get_level_button_box();
SDL_Rect dm_get_return_button_box();

SDL_Rect dm_get_stage_registers_box();
SDL_Rect dm_get_stage_code_box();
SDL_Rect dm_get_stage_instruction_box();
SDL_Rect dm_get_stage_input_buffer_box();
SDL_Rect dm_get_stage_output_buffer_box();
SDL_Rect dm_get_stage_ib_text_box();
SDL_Rect dm_get_stage_ob_text_box();

SDL_Rect dm_get_arrow_box();
SDL_Rect dm_get_arrow_ins_box();

SDL_Rect dm_get_text_box_big();
SDL_Rect dm_get_text_box_error();
SDL_Rect dm_get_text_box_result();
SDL_Rect dm_get_text_box_upper();
SDL_Rect dm_get_text_box_lower();
SDL_Rect dm_get_text_box_ins();
SDL_Rect dm_get_text_box_code();
SDL_Rect dm_get_text_box_stagebutton();

/*******************************RESOLUTION*************************************/
#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 900

/*******************************RETURN BUTTON*********************************/
#define MESSAGE_TEXT_H 40
#define MESSAGE_TEXT_TOTAL_H 160

/*******************************EXECUTION ARROW********************************/
#define EXEC_ARROW_X_COORD_OFFSET - 50
#define EXEC_ARROW_Y_COORD_OFFSET 10
#define EXEC_ARROW_W 30
#define EXEC_ARROW_H 30


/*******************************INSTRUCTION BOX********************************/
// The offset of the borders of the instruction box
#define INS_BOX_OFFSET 25

// TO DELETE
#define INS_BOX_W ((SCREEN_WIDTH*2/6)/3)
#define INS_BOX_H 250
#define INS_BOX_X 0
#define INS_BOX_Y ((SCREEN_HEIGHT/2) - (INS_BOX_H))

#define CODE_BOX_X (INS_BOX_X + INS_BOX_W)
#define CODE_BOX_Y 50
#define CODE_BOX_W ((SCREEN_WIDTH*2/6)*2/3)
#define CODE_BOX_H 625

#define REG_BOX_X (CODE_BOX_X + CODE_BOX_W)
#define REG_BOX_W (SCREEN_WIDTH/6)

// Instruction box text
#define INS_BOX_TEXT_W ((SCREEN_WIDTH*2/6)/3)

/*******************************RESULT BOX*************************************/
#define RES_BOX_W 500
#define RES_BOX_H 300
#define RES_BOX_X (SCREEN_WIDTH - RES_BOX_W)/2 
#define RES_BOX_Y 200

#define RES_BOX_TEXT_BORDER_OFFSET 25
#define RES_BOX_TEXT_W (RES_BOX_W - 2*RES_BOX_TEXT_BORDER_OFFSET)
#define RES_BOX_TEXT_X RES_BOX_X + (RES_BOX_W - RES_BOX_TEXT_W)/2
#define RES_BOX_TEXT_Y RES_BOX_Y + RES_BOX_TEXT_BORDER_OFFSET 
#define RES_BOX_TEXT_H 30

#define BACK_BUTTON_W 100
#define CONT_BUTTON_W 200

#define BACK_CONT_BUTTON_H 60

#define WIN_MENU_BUTTON1_X RES_BOX_X + (RES_BOX_W - (BACK_BUTTON_W + \
						   CONT_BUTTON_W))/3
#define WIN_MENU_BUTTON2_X RES_BOX_X + BACK_BUTTON_W + 2*(RES_BOX_W - \
						   (BACK_BUTTON_W + CONT_BUTTON_W))/3
#define WIN_MENU_BUTTON_Y RES_BOX_Y + RES_BOX_H - BACK_CONT_BUTTON_H - \
						  RES_BOX_TEXT_BORDER_OFFSET 

/****************************BACK-CONTINUE BUTTONS*****************************/
#define STR_BACK "Back"
#define STR_CONT "Continue"

/***********************************CODE***************************************/

#define CODE_BOX_NUMBER_WIDTH 35

#define CODE_LINES_SIZE 5
#define LAST_INSTRUCTION_DISTANCE 75

#define LINE_NUMBER_OFFSET 15
#define CODE_BOX_OFFSET 25
#define ADJUSTING_OFFSET 6

#define MAXIMUM_CODE_LINES 99

#define TEXT_BOX_WIDTH 300
#define TEXT_BOX_HEIGHT 30

#define MOVEMENT_DELTA 30

#define OP1_X_OFFSET (CODE_BUTTON_W + LINE_NUMBER_OFFSET)
#define OP2_X_OFFSET (2*CODE_BUTTON_W + 2*LINE_NUMBER_OFFSET)

/*************************INPUT/OUTPUT BUFFERS*********************************/
// The height of the text for the input/output buffers




#define BUFFER_TEXT_H 42

// The height and width of the contents of the buffers

// The position of the text of the input/output buffers

//#define	BUFFER_BOX_W 950
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

/***************************REGISTER BOX***************************************/
// The offset of the borders of the register box
#define REG_BOX_OFFSET 25
#define REG_BOX_X_OFFSET 20
#define REG_BOX_Y_OFFSET 50

// The height of the register text
#define REG_TEXT_H 40

// The position of the register text
#define REG_TEXT_Y_OFFSET 5

#define REG_VBOX_X REG_BOX_X + 190
#define REG_VBOX_OFFSET 10

// The between registers offset
#define BETWEEN_REG_OFFSET 10

/*****************************CODE BUTTONS*************************************/
// Dimensions of the buttons that will be used in code
#define STAGE_NAME_H 55
#define CODE_BUTTON_W 75
#define CODE_BUTTON_H 40
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

//#define VALUE_BOX_H 56
//#define VALUE_BOX_W 64
#define VALUE_BOX_H 40
#define VALUE_BOX_W 50
#define VALUE_H 35
#define VALUE_W 50


#endif
