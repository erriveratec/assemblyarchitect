#include <assert.h>
#include <stdbool.h>
#include "levels_lv.h"
#include "list.h"
#include "aux.h"
#include "dbg.h"
#include "buffers_bf.h"
#include "dimensions.h"
#include "registers_rg.h"
#include "instruction_window_iw.h"
#include "code_window_cw.h"
#include "stage_buttons_sb.h"

#define TUT_TEXT_SELECT_INSTRUCTION_1 "Select and drag an instruction from the" \
									  " instruction box"
#define TUT_TEXT_SELECT_INSTRUCTION_2 "Select another instruction from the" \
									  " instruction box"

#define TUT_TEXT_DROP_INSTRUCTION_1 "Drop the instruction in the code box"
#define TUT_TEXT_DROP_INSTRUCTION_2 "Drop the instruction in the code box"\
									" below the previous instruction"
#define TUT_TEXT_PRESS_PLAY	"Press the play button"


#define TUT_TEXT_SELECT_OP1 "Select the destiny operand"
#define TUT_TEXT_SELECT_OP2 "Select the source operand"

#define TUT_TEXT_SELECT_LAST_INSTRUCTION "Select and drag the last instruction"\
									" by clicking in the instruction name"	

#define TUT_TEXT_ELIMINATE_INSTRUCTION "Drag the instruction out of the code"\
									" to delete it."

#define TUT_TEXT_CHANGE_OPERAND "Change any operand by clicking on it. Click"\
								" the source operand of the instruction."

#define TUT_TEXT_SELECT_IB "Select the Input Buffer as the source operand."

#define TUT_TEXT_MOV_INSTRUCTION "Select the second instruction and move in to"\
								  " the first position."

#define TUT_TEXT_X INS_BOX_X
#define TUT_TEXT_Y SCREEN_HEIGHT/2 - TUT_BOX_H

#define TUT_TEXT_H 30

#define TUT_BOX_W (INS_BOX_W + 100)
#define TUT_BOX_H (SCREEN_HEIGHT/6)

#define TUT_BOX_X INS_BOX_X
#define TUT_BOX_Y (SCREEN_HEIGHT/2 - TUT_BOX_H/2)

#define ARROW_INS_X (INS_BOX_X + INS_BOX_W)/4
#define ARROW_INS_Y INS_BOX_Y + (TUT_BOX_Y - INS_BOX_Y)*3/4

#define ARROW_CODE_X (INS_BOX_X + INS_BOX_W)/2
#define ARROW_CODE_Y (TUT_BOX_Y + TUT_BOX_H*6/5)

#define LEVEL_LINE_W 4

// At the end of the code execution, the output buffer must be compared 
// against this list.

static List *win_list = NULL;
static int level_instructions_limit;
arrow_t g_arrow_ins;

enum message{
	MESSAGE_1,
	MESSAGE_2
};

List *get_win_list();
void add_to_win_list(int value, int type);
static void level_1_tutorial(bool holding_line, bool play);
static void level_1_tutorial_instruction_select(int message);
static void level_1_tutorial_drop_instruction(int message);
static int level_1_tutorial_select_operand(int operand_pos);
static void tutorial_press_play();
static void level_2_tutorial_move_instruction(int ins_x);
static void level_2_tutorial(bool holding_line, bool play);
static void level_2_tutorial_select_instruction();
static void level_2_tutorial_eliminate_instruction();
static void level_2_tutorial_change_operand(int op_x);
static void level_2_tutorial_select_output_buffer();
static bool check_display_reg_arrow();
static int check_display_buf_arrow();
static void set_level_1_win_list();
static void set_level_2_win_list();
static void set_level_3_win_list();
static void set_level_4_win_list();
static void set_level_5_win_list();
static void set_level_6_win_list();
static void set_level_7_win_list();
static void set_level_8_win_list();
static void set_level_9_win_list();

/* Function: level_2_tutorial
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 1
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_2_tutorial(bool holding_line, bool play){
	bf_draw_buffers(NO_OPERAND);
	rg_display_registers(false);
	int code_size = cw_get_code_list_size();
	
	const int pos_one = 0;
	const int pos_two = 1;
	const int two_instructions = 2;
	bool change_op = false;
	bool mov_instruction = false;
	bool press_play = false;
	code_line_t *i1= NULL;
	code_line_t *i2 = NULL;

	if (code_size >= two_instructions && holding_line == false){
		i1 = cw_get_code_line_at_pos(pos_one);
		i2 = cw_get_code_line_at_pos(pos_two);
	}
	if (i2 != NULL && i2->op1 != NULL && i2->op2 != NULL){
		if (i2->op1->id == RAX && i2->op2->id == RAX){
			change_op = true;
		} 
	}
	if (i1 != NULL && i1->op1 != NULL && i1->op2 != NULL &&
	    i2 != NULL && i2->op1 != NULL && i2->op2 != NULL) {
		if (i1->op1->id == OB && i1->op2->id == RAX &&
		    i2->op1->id == RAX && i2->op2->id == IB){
			mov_instruction = true;
		} else if (i1->op1->id == RAX && i1->op2->id == IB &&
		    i2->op1->id == OB && i2->op2->id == RAX){
			press_play = true;
		}
	}
	if (code_size > level_instructions_limit && holding_line == false){
		level_2_tutorial_select_instruction();
	} else if (code_size > level_instructions_limit && holding_line == true){
		level_2_tutorial_eliminate_instruction();
	} else if (change_op == true && holding_line == false){
		if (i2->state != CHANGING_OP2){
			level_2_tutorial_change_operand(i2->op2->b->x);
		} else if (i2->state == CHANGING_OP2){
			level_2_tutorial_select_output_buffer();
			bf_draw_buffers(IB);
		}
	} else if (mov_instruction == true){
		level_2_tutorial_move_instruction(i1->ins->b->x);
	} else if (press_play == true){
		tutorial_press_play();		
	}
	
}

/* Function: level_2_tutorial_move_instruction
 * -----------------------------------------------------------------------------
 * Points the player to the selection of the operands
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_2_tutorial_move_instruction(int ins_x)
{
	int final_y = cw_get_line_y( cw_get_code_list_size()-1) + ARROW_H;
	int w = cw_get_code_box_member(MEMBER_W);
	int h = cw_get_code_box_member(MEMBER_H);
	int x = cw_get_code_box_member(MEMBER_X) + (w - TUT_BOX_W)/2;
	int y = final_y + 2*ARROW_H;

	dw_draw_filled_rectangle(x, y, TUT_BOX_W, TUT_BOX_H, COLOR_BLACK, 
																   COLOR_WHITE);
	dw_draw_wrapped_text_fits_height(x, y, TUT_BOX_W, TUT_BOX_H, TUT_TEXT_H, 
										 COLOR_WHITE, TUT_TEXT_MOV_INSTRUCTION);

	int startx =  ins_x + ARROW_W/2;
	int starty = y - 2*ARROW_H;
	static arrow_t arrow;
	static bool arrow_initialized = false;
	if (arrow_initialized == false){
		arrow.box.w = ARROW_W;	
		arrow.box.h = ARROW_H;
		arrow.box.x = startx;		
		arrow.box.y = starty;
		arrow.texture = g_arrow;
		arrow.in_place = false;
		arrow_initialized = true;
	}
	int travel = ARROW_W;	
	SDL_SetTextureColorMod(arrow.texture->texture, 255, 0, 255);
	dw_animate_arrow(startx, starty, &arrow, DW_UP, travel);
	SDL_SetTextureColorMod(arrow.texture->texture, 255, 255, 255);

	return;
}
/* Function: level_2_tutorial_select_output_buffer
 * -----------------------------------------------------------------------------
 * Points the player to the selection of the operands
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_2_tutorial_select_output_buffer()
{
	int final_y = cw_get_line_y( cw_get_code_list_size()-1) + ARROW_H;
	int w = cw_get_code_box_member(MEMBER_W);
	int h = cw_get_code_box_member(MEMBER_H);
	int x = SCREEN_WIDTH/2 - TUT_BOX_W/2;
	int y = SCREEN_HEIGHT/4 - TUT_BOX_H;

	dw_draw_filled_rectangle(x, y, TUT_BOX_W, TUT_BOX_H, COLOR_BLACK, 
																   COLOR_WHITE);
	dw_draw_wrapped_text_fits_height(x, y, TUT_BOX_W, TUT_BOX_H, TUT_TEXT_H, 
								   	 	  	   COLOR_WHITE, TUT_TEXT_SELECT_IB);
	return;
}
/* Function: level_2_tutorial_change_operand
 * -----------------------------------------------------------------------------
 * Points the player to the selection of the operands
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_2_tutorial_change_operand(int op_x)
{
	int final_y = cw_get_line_y( cw_get_code_list_size()-1) + ARROW_H;
	int w = cw_get_code_box_member(MEMBER_W);
	int h = cw_get_code_box_member(MEMBER_H);
	int x = cw_get_code_box_member(MEMBER_X) + (w - TUT_BOX_W)/2;
	int y = final_y + 2*ARROW_H;

	dw_draw_filled_rectangle(x, y, TUT_BOX_W, TUT_BOX_H, COLOR_BLACK, 
																   COLOR_WHITE);
	dw_draw_wrapped_text_fits_height(x, y, TUT_BOX_W, TUT_BOX_H, TUT_TEXT_H, 
								   	 	  COLOR_WHITE, TUT_TEXT_CHANGE_OPERAND);


	int startx =  op_x + ARROW_W/2;
	int starty = y - 2*ARROW_H;
	static arrow_t arrow;
	static bool arrow_initialized = false;
	if (arrow_initialized == false){
		arrow.box.w = ARROW_W;	
		arrow.box.h = ARROW_H;
		arrow.box.x = startx;		
		arrow.box.y = starty;
		arrow.texture = g_arrow;
		arrow.in_place = false;
		arrow_initialized = true;
	}
	int travel = ARROW_W;	
	SDL_SetTextureColorMod(arrow.texture->texture, 255, 0, 255);
	dw_animate_arrow(startx, starty, &arrow, DW_UP, travel);
	SDL_SetTextureColorMod(arrow.texture->texture, 255, 255, 255);

	return;
}
/* Function: level_2_tutorial_eliminate instruction
 * -----------------------------------------------------------------------------
 * Points the player to the selection of the operands
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_2_tutorial_eliminate_instruction()
{
	int final_y = cw_get_line_y( cw_get_code_list_size()-1);
	int w = cw_get_code_box_member(MEMBER_W);
	int h = cw_get_code_box_member(MEMBER_H);
	int x = cw_get_code_box_member(MEMBER_X) + (w - TUT_BOX_W)/2;
	int y = final_y + 2*ARROW_H;

	dw_draw_filled_rectangle(x, y, TUT_BOX_W, TUT_BOX_H, COLOR_BLACK, 
																   COLOR_WHITE);
	dw_draw_wrapped_text_fits_height(x, y, TUT_BOX_W, TUT_BOX_H, TUT_TEXT_H, 
								   COLOR_WHITE, TUT_TEXT_ELIMINATE_INSTRUCTION);

	int startx =  cw_get_code_box_member(MEMBER_X) + w;
	int starty = cw_get_text_box_member(MEMBER_Y);
	static arrow_t arrow;
	static bool arrow_initialized = false;
	if (arrow_initialized == false){
		arrow.box.w = ARROW_W;	
		arrow.box.h = ARROW_H;
		arrow.box.x = startx;		
		arrow.box.y = starty;
		arrow.texture = g_arrow;
		arrow.in_place = false;
		arrow_initialized = true;
	}
	int travel = ARROW_W;	
	SDL_SetTextureColorMod(arrow.texture->texture, 255, 0, 255);
	dw_animate_arrow(startx, starty, &arrow, DW_RIGHT, travel);
	SDL_SetTextureColorMod(arrow.texture->texture, 255, 255, 255);

	return;
}

/* Function: level_2_tutorial_select_instruction
 * -----------------------------------------------------------------------------
 * Points the player to the selection of the operands
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_2_tutorial_select_instruction()
{

	int final_y = cw_get_line_y( cw_get_code_list_size()-1);
	int w = cw_get_code_box_member(MEMBER_W);
	int h = cw_get_code_box_member(MEMBER_H);
	int x = cw_get_code_box_member(MEMBER_X) + (w - TUT_BOX_W)/2;
	int y = final_y + 2*ARROW_H;

	dw_draw_filled_rectangle(x, y, TUT_BOX_W, TUT_BOX_H, COLOR_BLACK, 
																   COLOR_WHITE);
	dw_draw_wrapped_text_fits_height(x, y, TUT_BOX_W, TUT_BOX_H, TUT_TEXT_H, 
								 COLOR_WHITE, TUT_TEXT_SELECT_LAST_INSTRUCTION);

	int startx = cw_get_code_line_x(MOV) + ARROW_W/2;
	int starty = y - ARROW_H;
	static arrow_t arrow;
	static bool arrow_initialized = false;
	if (arrow_initialized == false){
		arrow.box.w = ARROW_W;	
		arrow.box.h = ARROW_H;
		arrow.box.x = startx;		
		arrow.box.y = starty;
		arrow.texture = g_arrow;
		arrow.in_place = false;
		arrow_initialized = true;
	}
	int travel = starty - final_y;	
	SDL_SetTextureColorMod(arrow.texture->texture, 255, 0, 255);
	dw_animate_arrow(startx, starty, &arrow, DW_UP, travel);
	SDL_SetTextureColorMod(arrow.texture->texture, 255, 255, 255);

	return;
}

/* Function: level_1_tutorial
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 1
 *
 * Arguments:
 * 	holding_line: boolean that indicates if the player is holding a line
 * 	play: boolean indicating if the player pressed play
 *	
 * Return:
 *	Void.
 */
static void level_1_tutorial(bool holding_line, bool play)
{
	int code_size = cw_get_code_list_size();
			bf_draw_buffers(NO_OPERAND);
			rg_display_registers(false);
		if (code_size == 0 && holding_line == false){
			level_1_tutorial_instruction_select(MESSAGE_1);
		} else if (code_size == 0 && holding_line == true){
			level_1_tutorial_drop_instruction(MESSAGE_1);
		} else if (code_size == 1 && cw_check_code_sorted() == true &&
									   cw_check_code_pending_op1() == true){
			level_1_tutorial_select_operand(OP1);
			rg_display_registers(true);
		} else if(code_size == 1 && cw_check_code_sorted() == true &&
				    				   cw_check_code_pending_operand() == true){
			level_1_tutorial_select_operand(OP2);
			rg_display_registers(false);
			bf_draw_buffers(IB);
		} else if(code_size == 1 && holding_line == false &&
 									  cw_check_code_pending_operand() == false){
			level_1_tutorial_instruction_select(MESSAGE_2);
		} else if(code_size == 1 && holding_line == true &&
 									  cw_check_code_pending_operand() == false){
			level_1_tutorial_drop_instruction(MESSAGE_2);
		} else if (code_size == 2 && cw_check_code_sorted() == true &&
									   cw_check_code_pending_op1() == true){
			level_1_tutorial_select_operand(OP1);
			bf_draw_buffers(OB);
		} else if(code_size == 2 && cw_check_code_sorted() == true &&
				    				   cw_check_code_pending_operand() == true){
			level_1_tutorial_select_operand(OP2);
			rg_display_registers(true);
		} else if(code_size == 2 && cw_check_code_pending_operand() == false &&
																 play == false){
			tutorial_press_play();
		} 

}

/* Function: tutorial_press_play
 * -----------------------------------------------------------------------------
 * Displays to the player to press the play button 
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void tutorial_press_play()
{
	int w = cw_get_code_box_member(MEMBER_W);
	int h = cw_get_code_box_member(MEMBER_H);
	int x = cw_get_code_box_member(MEMBER_X) + (w - TUT_BOX_W)/2;
	int y = (cw_get_code_box_member(MEMBER_Y)+ h) - TUT_BOX_H/2;

	static arrow_t arrow;
	static bool arrow_initialized = false;
	int startx = sb_get_play_button_member(MEMBER_X);
	int starty = STAGE_BUTTON_Y - 2*ARROW_H;
	if (arrow_initialized == false){
		arrow.box.w = ARROW_W;	
		arrow.box.h = ARROW_H;
		arrow.box.x = startx;
		arrow.box.y = starty;
		arrow.texture = g_arrow;
		arrow.in_place = false;
		arrow_initialized = true;
	}
	int travel = STAGE_BUTTON_Y - starty - ARROW_H;	
	dw_animate_arrow(startx, starty, &arrow, DW_DOWN, travel);
	

	dw_draw_filled_rectangle(x, y, TUT_BOX_W, TUT_BOX_H, COLOR_BLACK, 
																   COLOR_WHITE);
	dw_draw_wrapped_text_fits_height(x, y, TUT_BOX_W, TUT_BOX_H, TUT_TEXT_H, 
											  COLOR_WHITE, TUT_TEXT_PRESS_PLAY);
		return ;
}

/* Function: level_1_tutorial_instruction_select
 * -----------------------------------------------------------------------------
 * This function handles the tutorial part of level 1
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_1_tutorial_instruction_select(int message)
{
	static arrow_t arrow;
	static bool arrow_initialized = false;
	if (arrow_initialized == false){
		arrow.box.x = ARROW_INS_X;
		arrow.box.y = ARROW_INS_Y;
		arrow.box.w = ARROW_W;
		arrow.box.h = ARROW_H;
		arrow.texture = g_arrow;
		arrow.in_place = false;
		arrow_initialized = true;
	}
	int travel = ARROW_INS_Y - iw_get_instruction_y_by_id(MOV);
	dw_animate_arrow(ARROW_INS_X, ARROW_INS_Y, &arrow, DW_UP, travel);
	dw_draw_filled_rectangle(TUT_BOX_X, TUT_BOX_Y, TUT_BOX_W, TUT_BOX_H, 
							  						  COLOR_BLACK, COLOR_WHITE);
	if (message == MESSAGE_1){
		dw_draw_wrapped_text_fits_height(TUT_BOX_X, TUT_BOX_Y, TUT_BOX_W, 
			 TUT_BOX_H, TUT_TEXT_H, COLOR_WHITE, TUT_TEXT_SELECT_INSTRUCTION_1);
	} else if (message == MESSAGE_2){
		dw_draw_wrapped_text_fits_height(TUT_BOX_X, TUT_BOX_Y, TUT_BOX_W, 
			  TUT_BOX_H,TUT_TEXT_H, COLOR_WHITE, TUT_TEXT_SELECT_INSTRUCTION_2);
	}	
		return ;
}
/* Function: level_1_tutorial_drop_instruction
 * -----------------------------------------------------------------------------
 * Handles the drop instruction part of the tutorial
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_1_tutorial_drop_instruction(int message)
{
	static arrow_t arrow;
	static bool arrow_initialized = false;
	if (arrow_initialized == false){
		arrow.box.x = ARROW_CODE_X;
		arrow.box.y = ARROW_CODE_Y;
		arrow.box.w = ARROW_W;
		arrow.box.h = ARROW_H;
		arrow.texture = g_arrow;
		arrow.in_place = false;
		arrow_initialized = true;
	}
	int travel = cw_get_code_box_member(MEMBER_X) - ARROW_CODE_X - ARROW_W;
	dw_animate_arrow(ARROW_CODE_X, ARROW_CODE_Y, &arrow, DW_RIGHT, travel);
	dw_draw_filled_rectangle(TUT_BOX_X, TUT_BOX_Y, TUT_BOX_W, TUT_BOX_H, 
							 COLOR_BLACK, COLOR_WHITE);

	dw_draw_filled_rectangle(TUT_BOX_X, TUT_BOX_Y, TUT_BOX_W, TUT_BOX_H, 
							  						  COLOR_BLACK, COLOR_WHITE);
	if (message == MESSAGE_1){
		dw_draw_wrapped_text_fits_height(TUT_BOX_X, TUT_BOX_Y, TUT_BOX_W, 
			   TUT_BOX_H, TUT_TEXT_H, COLOR_WHITE, TUT_TEXT_DROP_INSTRUCTION_1);
	} else if (message == MESSAGE_2){
		dw_draw_wrapped_text_fits_height(TUT_BOX_X, TUT_BOX_Y, TUT_BOX_W, 
			   TUT_BOX_H, TUT_TEXT_H, COLOR_WHITE, TUT_TEXT_DROP_INSTRUCTION_2);
	}	
	return ;
}

/* Function: level_1_tutorial_select_operand
 * -----------------------------------------------------------------------------
 * Points the player to the selection of the operands
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static int level_1_tutorial_select_operand(int operand_pos)
{
	int w = cw_get_code_box_member(MEMBER_W);
	int h = cw_get_code_box_member(MEMBER_H);
	int x = cw_get_code_box_member(MEMBER_X) + (w - TUT_BOX_W)/2;
	int y = TUT_BOX_Y;//(cw_get_code_box_member(MEMBER_Y)+ h)/2;
	dw_draw_filled_rectangle(x, y, TUT_BOX_W, TUT_BOX_H, COLOR_BLACK, 
																   COLOR_WHITE);
	if (operand_pos == OP1){
		dw_draw_wrapped_text_fits_height(x, y, TUT_BOX_W, TUT_BOX_H, TUT_TEXT_H, 
										   	  COLOR_WHITE, TUT_TEXT_SELECT_OP1);
	} else if (operand_pos == OP2){
		dw_draw_wrapped_text_fits_height(x, y, TUT_BOX_W, TUT_BOX_H, TUT_TEXT_H, 
										   	  COLOR_WHITE, TUT_TEXT_SELECT_OP2);
	}
	return 0;
}

/* Function: check_display_reg_arrow
 * -----------------------------------------------------------------------------
 * Analize the state of the operands to determine if the register arrow should
 * be displayed
 * 
 * Arguments:
 * 	None.
 *
 * Return:
 *	The id of the operand that the arrow must be shown 
 */
static int check_display_buf_arrow() 
{
	int display_arrow = NO_OPERAND;
	if (cw_check_code_sorted() == true && 
									   cw_check_code_pending_operand() == true){
		code_line_t *l = cw_get_code_line_pending_operand();
		operand_t o;
		o.id = IB;
		if (check_operand_compatilibity(&o, l) == true){
			display_arrow = IB;
		} 
		o.id = OB;
		if (check_operand_compatilibity(&o, l) == true){
			display_arrow = OB;
		}
	}
	return display_arrow;
}

/* Function: check_display_reg_arrow
 * -----------------------------------------------------------------------------
 * Analize the state of the operands to determine if the register arrow should
 * be displayedj
 * 
 * Arguments:
 * 	None.
 *
 * Return:
 *	true if the pointing arrow to the registers should be displayed
 */
static bool check_display_reg_arrow() 
{
	bool display_arrow = false;
	if (cw_check_code_sorted() == true && 
									   cw_check_code_pending_operand() == true){
		code_line_t *l = cw_get_code_line_pending_operand();
		operand_t o;
		o.id = RAX;
		display_arrow = check_operand_compatilibity(&o, l);
	}
	return display_arrow;
}
/* Function: lv_get_level_instructions_limit
 * -----------------------------------------------------------------------------
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	The limit number of instructions for the level
 */
int lv_get_level_instructions_limit()
{
	return level_instructions_limit;
}

/* Function: lv_set_level_instructions_limit
 * -----------------------------------------------------------------------------
 * Arguments:
 * 	Limit: The quantity of the instructions that the level will allow
 *	
 * Return:
 *	void
 */
void lv_set_level_instructions_limit(int limit)
{
	level_instructions_limit = limit;
}


/* Function: lv_generate_win_condition_list
 * -----------------------------------------------------------------------------
 * Arguments:
 * 	level: The level of the win condition that will be generated.
 *	
 * Return:
 *	void
 */
void lv_generate_win_condition_list(int level)
{
	switch (level){
		case LV_LEVEL_1:
			set_level_1_win_list();
			break;
		case LV_LEVEL_2:
			set_level_2_win_list();
			break;
		case LV_LEVEL_3:
			set_level_3_win_list();
			break;
		case LV_LEVEL_4:
			set_level_4_win_list();
			break;
		case LV_LEVEL_5:
			set_level_5_win_list();
			break;
		case LV_LEVEL_6:
			set_level_6_win_list();
			break;
		case LV_LEVEL_7:
			set_level_7_win_list();
			break;
		case LV_LEVEL_8:
			set_level_8_win_list();
			break;
		case LV_LEVEL_9:
			set_level_9_win_list();
			break;
		default:
			break;
	}

}
/* Function: add_to_win_list
 * -----------------------------------------------------------------------------
 * Arguments:
 * 	value: The input value to be added.
 *	type: The type of input to be added.
 *	
 * Return:
 *	void
 */
void add_to_win_list(int value, int type)
{
	List *win = get_win_list();

	assert(win != NULL && "win pointer is NULL");
	
	value_box_t *new_input = malloc(sizeof(value_box_t));
	check_mem(new_input);

	new_input->value = value;
	new_input->type = type;

	List_push(win, new_input);

error:
	return;
}

/* Function: lv_destroy_win_list
 *------------------------------------------------------------------------------
 * Destroys the win list when finishing a level.
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 *
 */
void lv_destroy_win_list()
{
	List_clear_destroy(win_list);
	win_list = NULL;
}


/* Function: lv_create_win_list
 *------------------------------------------------------------------------------
 * This function creates and initializes the input list.
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 *
 */
void lv_create_win_list()
{
	assert(win_list == NULL && "The win list is not NULL");
	win_list = List_create();
	check_mem(win_list);

error:
	return;
}

/* Function: lv_reset_win_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	void.
 *
 * Return:
 *	void.
 */
void lv_reset_win_list()
{
	List_clear_destroy(win_list);
	win_list = NULL;
	lv_create_win_list();
}

/* Function: get_win_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Pointer to input list.
 */
List *get_win_list()
{
	return win_list;
}

/* Function: lv_print_win_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	
 */
void lv_print_win_list()
{
	List *win_list = get_win_list();

	assert(win_list != NULL && "Win list pointer is NULL");

	int win_list_size = List_count(win_list);

	printf("The size of the list is %d\n", win_list_size);	

	LIST_FOREACH(win_list, first, next, cur){
		value_box_t *cur_input = cur->value;
		printf("List value: %d\n", cur_input->value);
	}
}
/* Function: set_level_1_win_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Pointer to the object that will be assigned as a default reg operand.
 */
static void set_level_1_win_list()
{
	List *input_list = get_input_list();
	List *win_list = get_win_list();

	assert(input_list != NULL && "Input list pointer is NULL");
	assert(win_list != NULL && "Win list pointer is NULL");

	int input_list_size = List_count(input_list);
	int win_list_size = List_count(win_list);

	assert(input_list_size > 0 && "The size of the input list is incorrect");
	assert(win_list_size == 0 && "The win list has elements");

	LIST_FOREACH(input_list, first, next, cur){
		value_box_t *cur_input = cur->value;
		value_box_t *new_win = malloc(sizeof(value_box_t));
		new_win->value = cur_input->value;
		new_win->type = cur_input->type;
		List_push(win_list, new_win);
	}
}

/* Function: set_level_2_win_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Pointer to the object that will be assigned as a default reg operand.
 */
static void set_level_2_win_list()
{
	List *input_list = get_input_list();
	List *win_list = get_win_list();

	assert(input_list != NULL && "Input list pointer is NULL");
	assert(win_list != NULL && "Win list pointer is NULL");

	int input_list_size = List_count(input_list);
	int win_list_size = List_count(win_list);

	assert(input_list_size > 0 && "The size of the input list is incorrect");
	assert(win_list_size == 0 && "The win list has elements");

	LIST_FOREACH(input_list, first, next, cur){
		value_box_t *cur_input = cur->value;
		value_box_t *new_win = malloc(sizeof(value_box_t));
		new_win->value = cur_input->value;
		new_win->type = cur_input->type;
		List_push(win_list, new_win);
	}
}

/* Function: set_level_3_win_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Pointer to the object that will be assigned as a default reg operand.
 */
static void set_level_3_win_list()
{
	List *input_list = get_input_list();
	List *win_list = get_win_list();

	assert(input_list != NULL && "Input list pointer is NULL");
	assert(win_list != NULL && "Win list pointer is NULL");

	int input_list_size = List_count(input_list);
	int win_list_size = List_count(win_list);

	assert(input_list_size > 0 && "The size of the input list is incorrect");
	assert(win_list_size == 0 && "The win list has elements");

	LIST_FOREACH(input_list, first, next, cur){
		value_box_t *cur_input = cur->value;
		value_box_t *new_win = malloc(sizeof(value_box_t));
		new_win->value = cur_input->value;
		new_win->type = cur_input->type;
		List_push(win_list, new_win);
//		new_win = malloc(sizeof(value_box_t));
//		new_win->value = cur_input->value;
//		new_win->type = cur_input->type;
//		List_push(win_list, new_win);
//		new_win = malloc(sizeof(value_box_t));
//		new_win->value = cur_input->value;
//		new_win->type = cur_input->type;
//		List_push(win_list, new_win);
	}
}
/* Function: set_level_4_win_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Pointer to the object that will be assigned as a default reg operand.
 */
static void set_level_4_win_list()
{
	List *input_list = get_input_list();
	List *win_list = get_win_list();

	assert(input_list != NULL && "Input list pointer is NULL");
	assert(win_list != NULL && "Win list pointer is NULL");

	int input_list_size = List_count(input_list);
	int win_list_size = List_count(win_list);

	assert(input_list_size > 0 && "The size of the input list is incorrect");
	assert(win_list_size == 0 && "The win list has elements");

	LIST_FOREACH(input_list, first, next, cur){
		value_box_t *cur_input = cur->value;
		value_box_t *new_win = malloc(sizeof(value_box_t));
		new_win->value = cur_input->value;
		new_win->type = cur_input->type;
		List_push(win_list, new_win);
	}
}

/* Function: set_level_5_win_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Pointer to the object that will be assigned as a default reg operand.
 */
static void set_level_5_win_list()
{
	List *input_list = get_input_list();
	List *win_list = get_win_list();

	assert(input_list != NULL && "Input list pointer is NULL");
	assert(win_list != NULL && "Win list pointer is NULL");

	int input_list_size = List_count(input_list);
	int win_list_size = List_count(win_list);

	assert(input_list_size > 0 && "The size of the input list is incorrect");
	assert(win_list_size == 0 && "The win list has elements");

	LIST_FOREACH(input_list, first, next, cur){
		value_box_t *cur_input = cur->value;
		value_box_t *new_win = malloc(sizeof(value_box_t));
		new_win->value = cur_input->value;
		new_win->type = cur_input->type;
		List_unshift(win_list, new_win);
	}
}

/* Function: set_level_6_win_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Pointer to the object that will be assigned as a default reg operand.
 */
static void set_level_6_win_list()
{
	List *input_list = get_input_list();
	List *win_list = get_win_list();

	assert(input_list != NULL && "Input list pointer is NULL");
	assert(win_list != NULL && "Win list pointer is NULL");

	int input_list_size = List_count(input_list);
	assert(input_list_size % 2 == 0 && "For this challenge the list size must\
		   be an even number");
	
	int win_list_size = List_count(win_list);
	assert(input_list_size > 0 && "The size of the input list is incorrect");
	assert(win_list_size == 0 && "The win list has elements");

	int i = 1;
	int prev_val = 0;
	LIST_FOREACH(input_list, first, next, cur){
		value_box_t *cur_input = cur->value;
		
		if (i % 2 == 0){
			value_box_t *new_win = malloc(sizeof(value_box_t));
			new_win->value = cur_input->value + prev_val;
			new_win->type = cur_input->type;
			List_push(win_list, new_win);
		}
		prev_val = cur_input->value;
		i++;
	}
}

/* Function: set_level_7_win_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 */
static void set_level_7_win_list()
{
	List *input_list = get_input_list();
	List *win_list = get_win_list();

	assert(input_list != NULL && "Input list pointer is NULL");
	assert(win_list != NULL && "Win list pointer is NULL");

	int input_list_size = List_count(input_list);
	
	int win_list_size = List_count(win_list);
	assert(input_list_size > 0 && "The size of the input list is incorrect");
	assert(win_list_size == 0 && "The win list has elements");

	
	LIST_FOREACH(input_list, first, next, cur){
		value_box_t *cur_input = cur->value;
		
		value_box_t *new_win = malloc(sizeof(value_box_t));
		new_win->value = 5*cur_input->value;
		new_win->type = cur_input->type;
		List_push(win_list, new_win);
	}
}

/* Function: set_level_8_win_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 */
static void set_level_8_win_list()
{
	List *input_list = get_input_list();
	List *win_list = get_win_list();

	assert(input_list != NULL && "Input list pointer is NULL");
	assert(win_list != NULL && "Win list pointer is NULL");

	int input_list_size = List_count(input_list);
	
	int win_list_size = List_count(win_list);
	assert(input_list_size > 0 && "The size of the input list is incorrect");
	assert(win_list_size == 0 && "The win list has elements");

	
	LIST_FOREACH(input_list, first, next, cur){
		value_box_t *cur_input = cur->value;
		
		value_box_t *new_win = malloc(sizeof(value_box_t));
		new_win->value = 8*cur_input->value;
		new_win->type = cur_input->type;
		List_push(win_list, new_win);
	}
}

/* Function: set_level_9_win_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 */
static void set_level_9_win_list()
{
	List *input_list = get_input_list();
	List *win_list = get_win_list();

	assert(input_list != NULL && "Input list pointer is NULL");
	assert(win_list != NULL && "Win list pointer is NULL");

	int input_list_size = List_count(input_list);
	
	int win_list_size = List_count(win_list);
	assert(input_list_size > 0 && "The size of the input list is incorrect");
	assert(win_list_size == 0 && "The win list has elements");

	
	LIST_FOREACH(input_list, first, next, cur){
		value_box_t *cur_input = cur->value;
		value_box_t *new_win = malloc(sizeof(value_box_t));
		new_win->value = cur_input->value;
		new_win->type = cur_input->type;
		List_push(win_list, new_win);
	}
}

/* Function: lv_evaluate_output_correctness
 *------------------------------------------------------------------------------
 * Evaluates the correctness of the output as values are being added
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	boolean stating if the output buffer contents are correct
 */
bool lv_evaluate_output_correctness()
{
	List *output_list = get_output_list();
	List *win_list = get_win_list();

	assert(output_list != NULL && "Output list pointer is NULL");
	assert(win_list != NULL && "Win list pointer is NULL");

	int output_list_size = List_count(output_list);
	int win_list_size = List_count(win_list);

	assert(win_list_size > 0 && "The win list has no elements");

	if (output_list_size == 0){
		return true;
	}

	ListNode *win_node = win_list->first;
	int i = 0;
	LIST_FOREACH(output_list, first, next, cur){
		if (i == output_list_size){
			break;
		}
		value_box_t *output = cur->value;
		value_box_t *win_val = win_node->value;
		if (output->value != win_val->value){
			return false;
		}
		win_node = win_node->next;
		i++;
	}
	return true;
}

/* Function: lv_level_drawings
 *------------------------------------------------------------------------------
 * This functions draws the specific characteristics of a level, like where
 * each of the avatars are able to reach.
 *
 * Arguments:
 *	level: The speficic level that is goin to have the drawing.
 *
 * Return:
 *	Void.
 */
void lv_level_drawings(int level, bool holding_line, bool play)
{
	assert(level < LV_LEVEL_MAX && level > LV_LEVEL_MIN && 
			          								"Invalid level value");

	int x = REG_BOX_X + rg_get_register_box_member(MEMBER_W);
	int y = SCREEN_HEIGHT/2 - LEVEL_LINE_W/2; 
	int w = SCREEN_WIDTH - x;
	int h = LEVEL_LINE_W;
	
	dw_draw_filled_rectangle(x, y, w, h, COLOR_GREY, COLOR_GREY);

	switch(level){
		case LV_LEVEL_1:
			level_1_tutorial(holding_line, play);
			break;

	case LV_LEVEL_2:
			level_2_tutorial(holding_line, play);
			break;

		default:
			bf_draw_buffers(check_display_buf_arrow());
			rg_display_registers(check_display_reg_arrow());
	}

}


/* Function: lv_check_if_win
 *------------------------------------------------------------------------------
 * Evaluates the whole output list against the win list to verify if the 
 * result is correct. Does not verifies if there are still elements in the
 * output list. 
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	True if the win condition is met, false if otherwise
 */
bool lv_check_if_win()
{
	List *output_list = get_output_list();
	List *win_list = get_win_list();

	assert(output_list != NULL && "Output list pointer is NULL");
	assert(win_list != NULL && "Win list pointer is NULL");

	int output_list_size = List_count(output_list);
	int win_list_size = List_count(win_list);

	assert(win_list_size > 0 && "The win list has no elements");

	if (output_list_size != win_list_size || output_list_size == 0){
		return false;
	}

	ListNode *win_node = win_list->first;

	LIST_FOREACH(output_list, first, next, cur){
		value_box_t *output = cur->value;
		value_box_t *win_val = win_node->value;

		if (output->value != win_val->value){
			return false;
		}
		win_node = win_node->next;
	}
	return true;
}
