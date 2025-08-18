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

#define MSG_SEL_INS1 "Select and drag an instruction from the"\
									  " instruction box"
#define MSG_SEL_INS2 "Select another instruction from the" \
									  " instruction box"
#define MSG_DROP_INS1 "Drop the instruction in the code box"
#define MSG_DROP_INS2 "Drop the instruction in the code box"\
									" below the previous instruction"
#define MSG_PRESS_PLAY	"Press the play button"
#define MSG_SEL_OP1 "Select the destiny operand"
#define MSG_SEL_OP2 "Select the source operand"
#define MSG_SEL_LAST_INS "Select and drag the last instruction"\
									" by clicking in the instruction name"	
#define MSG_DEL_INS "Drag the instruction out of the code"\
									" to delete it."
#define MSG_CHANGE_OP "Change any operand by clicking on it. Click"\
								" the source operand of the instruction."
#define MSG_SEL_IB "Select the Input Buffer as the source operand."
#define MSG_MOV_INS "Select the second instruction and move in to"\
								  " the first position."
#define MSG_AVAIL_OPS "When selecting an instruction operand the"\
									" available operands will be pointed with"\
									" an arrow."
#define MSG_COMPAT_OPS "Not all operands are compatible with"\
									 " each other, for instance the"\
									 " instruction:  mov [ob], [ib] have"\
									 " operands not compatible."
#define MSG_TRY_YOURSELF "Now try to solve the challeng for yourself."
#define MSG_NEW_REGS "Notice that now are more registers available"\
								" try using them to solve the change."
#define MSG_NEW_INS_ADD "There is a new instruction \"add\" which adds"\
							 " the contents of the two operands and it stores"\
							 " it in the destiny operand."
#define MSG_NEW_INS_JMP "The \"jmp\" instruction jumps to a position"\
							 " pointed by the label. The operand of the"\
							 " instruction is the label."

#define TUT_TEXT_X INS_BOX_X
#define TUT_TEXT_Y SCREEN_HEIGHT/2 - MSG_BOX_H

#define MSG_TEXT_H 30

#define MSG_BOX_W (INS_BOX_W + 100)
#define MSG_BOX_H 210
#define MSG_INS_BOX_X INS_BOX_X
#define MSG_INS_BOX_Y (SCREEN_HEIGHT/2 - MSG_BOX_H/3)

#define ARROW_INS_X (INS_BOX_X + INS_BOX_W)/4
#define ARROW_INS_Y INS_BOX_Y + (MSG_INS_BOX_Y - INS_BOX_Y)*3/4
#define ARROW_CODE_X (INS_BOX_X + INS_BOX_W)/2
#define ARROW_CODE_Y (MSG_INS_BOX_Y + MSG_BOX_H*6/5)

#define LEVEL_LINE_W 4

static List *win_list = NULL;
static int level_instructions_limit;

texture_t *g_lv_arrow;
arrow_t g_arrow_ins; 
arrow_t g_arrow_code;
arrow_t g_arrow_play;

enum message{
	MESSAGE_1,
	MESSAGE_2,
	TRY_YOURSELF,
	NEW_INS_ADD,
	NEW_INS_JMP
};

enum positions{
	INS_BOX,
	INFO_BOX,
	CODE_BOX,
	SB_BOX
};

enum arrow_id{
	INS_ARROW,
	DROP_CODE_ARROW,
	PLAY_ARROW
};


SDL_Rect g_msg_ins_box = {MSG_INS_BOX_X, MSG_INS_BOX_Y, MSG_BOX_W, MSG_BOX_H};
SDL_Rect g_msg_info_box = {SCREEN_WIDTH/2 - MSG_BOX_W/2, 
							 SCREEN_HEIGHT/5 - MSG_BOX_H, MSG_BOX_W, MSG_BOX_H};
SDL_Rect g_msg_code_box = {CODE_BOX_X + (CODE_BOX_W - MSG_BOX_W)/2, 
						   MSG_INS_BOX_Y, MSG_BOX_W, MSG_BOX_H};
SDL_Rect g_msg_sb_box = {CODE_BOX_X + (CODE_BOX_W - MSG_BOX_W)/2, 
			     (CODE_BOX_Y + CODE_BOX_H) - MSG_BOX_H/2, MSG_BOX_W, MSG_BOX_H};



List *get_win_list();
void add_to_win_list(int value, int type);
static void level_1_tutorial(bool holding_line, bool play);
static void level_2_tutorial_move_instruction(int ins_x);
static void level_2_tutorial(bool holding_line, bool play);
static void level_2_tutorial_select_instruction();
static void level_2_tutorial_eliminate_instruction();
static void level_2_tutorial_change_operand(int op_x);
static void level_2_tutorial_select_output_buffer();
static void level_3_tutorial(bool holding_line, bool play);
static void level_3_tutorial_show_available_operands(int op_case);
static void level_5_tutorial(bool holding_line, bool play);
static void level_6_tutorial(bool holding_line, bool play);
static void tutorial_press_play();
static bool check_display_reg_lv_arrow();
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
static void message_box(int pos, char *msg);


void lv_initialize_level_assets()
{
	g_arrow_ins.box.x = ARROW_INS_X; 
	g_arrow_ins.box.y = ARROW_INS_Y;
	g_arrow_ins.box.w = ARROW_W; 
	g_arrow_ins.box.h = ARROW_H;
	g_arrow_ins.texture =  g_lv_arrow;
	g_arrow_ins.in_place = false;
	g_arrow_ins.visible =  true;

	g_arrow_code.box.x = ARROW_CODE_X;
	g_arrow_code.box.y = ARROW_CODE_Y;
	g_arrow_code.box.w = ARROW_W;
	g_arrow_code.box.h = ARROW_H;
	g_arrow_code.texture = g_lv_arrow;
	g_arrow_code.in_place = false;
	g_arrow_code.visible = true;

	g_arrow_play.box.w = ARROW_W;	
	g_arrow_play.box.h = ARROW_H;
	g_arrow_play.box.x = STAGE_BUTTON_X;
	g_arrow_play.box.y = STAGE_BUTTON_Y - 2*ARROW_H;
	g_arrow_play.texture = g_lv_arrow;
	g_arrow_play.in_place = false;
	g_arrow_play.visible = true;
}
/* Function: display_arrow
 * -----------------------------------------------------------------------------
 * Animates the arrow that points to the first instruction
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void display_arrow(int arrow_id)
{
	int x;
	int y;
	int travel;
	int dir;
	arrow_t *aptr;
	switch(arrow_id){
		case INS_ARROW:
			x = ARROW_INS_X;
			y = ARROW_INS_Y;
			travel = ARROW_INS_Y - iw_get_instruction_y_by_id(MOV);
			dir = DW_UP;
			aptr = &g_arrow_ins;
			break;
	
		case DROP_CODE_ARROW:
			x = ARROW_CODE_X;
			y = ARROW_CODE_Y;
			travel = ARROW_INS_Y - iw_get_instruction_y_by_id(MOV);
			dir = DW_RIGHT;
			aptr = &g_arrow_code;
			break;
		case PLAY_ARROW:
			x = STAGE_BUTTON_X;
			y = STAGE_BUTTON_Y - 2*ARROW_H;	
			travel = STAGE_BUTTON_Y - y - ARROW_H;	
			dir = DW_DOWN;
			aptr = &g_arrow_play;
			break;

		default:
			break;
	}
	dw_animate_arrow(x, y, aptr, dir, travel);
}

/* Function: message_box
 * -----------------------------------------------------------------------------
 * This function displays a message box in differents part of the screen 
 * according to an identifies
 *
 * Arguments:
 * 	msg: The message id of what is going to be displayed.
 *	pos: The position id of where is gonna be displayed.
 *	
 * Return:
 *	Void.
 */
static void message_box(int pos, char *msg)
{
	SDL_Rect b;
	switch(pos){
		case INS_BOX:
			b.x = g_msg_ins_box.x; 
			b.y = g_msg_ins_box.y; 
			b.w = g_msg_ins_box.w; 
			b.h = g_msg_ins_box.h;	
			break;
		case INFO_BOX:
			b.x = g_msg_info_box.x; 
			b.y = g_msg_info_box.y; 
			b.w = g_msg_info_box.w; 
			b.h = g_msg_info_box.h;	
			break;
		case CODE_BOX:
			b.x = g_msg_code_box.x; 
			b.y = g_msg_code_box.y; 
			b.w = g_msg_code_box.w; 
			b.h = g_msg_code_box.h;	
			break;
		case SB_BOX:
			b.x = g_msg_sb_box.x; 
			b.y = g_msg_sb_box.y; 
			b.w = g_msg_sb_box.w; 
			b.h = g_msg_sb_box.h;	
			break;


	}
	dw_draw_filled_rectangle(b.x, b.y, b.w, b.h, COLOR_BLACK, COLOR_WHITE);
	dw_draw_wrapped_text_fits_height(b.x, b.y, b.w, b.h, MSG_TEXT_H, 
															  COLOR_WHITE, msg);
}

/* Function: level_9_tutorial
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 3
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_9_tutorial(bool holding_line, bool play)
{
	bf_draw_buffers(check_display_buf_arrow());
	rg_display_registers(check_display_reg_lv_arrow());

	int code_size = cw_get_code_list_size();

	if (code_size == 0 && holding_line == false){
		message_box(INFO_BOX, MSG_NEW_INS_JMP);
	} 
}

/* Function: level_6_tutorial
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 3
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_6_tutorial(bool holding_line, bool play)
{
	bf_draw_buffers(check_display_buf_arrow());
	rg_display_registers(check_display_reg_lv_arrow());

	int code_size = cw_get_code_list_size();

	if (code_size == 0 && holding_line == false){
		message_box(INFO_BOX, MSG_NEW_INS_ADD);
	} 
}

/* Function: level_5_tutorial
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 3
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_5_tutorial(bool holding_line, bool play)
{
	bf_draw_buffers(check_display_buf_arrow());
	rg_display_registers(check_display_reg_lv_arrow());

	int code_size = cw_get_code_list_size();

	if (code_size == 0 && holding_line == false){
		message_box(INFO_BOX, MSG_NEW_REGS);
	} 
}

/* Function: level_3_tutorial
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 3
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_3_tutorial(bool holding_line, bool play)
{
	bf_draw_buffers(check_display_buf_arrow());
	rg_display_registers(check_display_reg_lv_arrow());

	int code_size = cw_get_code_list_size();

	if (code_size == 0 && holding_line == false){
		message_box(INS_BOX, MSG_SEL_INS1);
		display_arrow(INS_ARROW);
	} else if (code_size == 0 && holding_line == true){
		message_box(INS_BOX, MSG_DROP_INS1);
		display_arrow(DROP_CODE_ARROW);
	} else if (code_size == 1 && cw_check_code_pending_op1() == true && 
												cw_check_code_sorted() == true){
		level_3_tutorial_show_available_operands(OP1);
	} else if (code_size == 1 && cw_check_code_pending_op2() == true && 
												cw_check_code_sorted() == true){
		level_3_tutorial_show_available_operands(OP2);
	} else if (code_size == 1 && cw_check_code_pending_operand() == false){
		message_box(INFO_BOX, MSG_TRY_YOURSELF);
	}
}

/* Function: level_3_tutorial_show_available_operands
 * -----------------------------------------------------------------------------
 * Points the player to the selection of the operands
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_3_tutorial_show_available_operands(int op_case)
{
	int final_y = cw_get_line_y(cw_get_code_list_size()-1) + ARROW_H;
	int h;	
	int x = SCREEN_WIDTH/2 - MSG_BOX_W/2;
	int y;

	if (op_case == OP1){
		y = SCREEN_HEIGHT*3/4 - MSG_BOX_H/2;
		h = ax_get_wrapped_text_height(MSG_BOX_W, MSG_TEXT_H, 
												   MSG_AVAIL_OPS);
		dw_draw_filled_rectangle(x, y, MSG_BOX_W, h, COLOR_BLACK, COLOR_WHITE);
		dw_draw_wrapped_text_fits_height(x, y, MSG_BOX_W, h, MSG_TEXT_H, 
								   	  COLOR_WHITE, MSG_AVAIL_OPS);
	} else if (op_case == OP2){
		y = SCREEN_HEIGHT/4 - MSG_BOX_H;
		h = ax_get_wrapped_text_height(MSG_BOX_W, MSG_TEXT_H, 
												  MSG_COMPAT_OPS);
		dw_draw_filled_rectangle(x, y, MSG_BOX_W, h, COLOR_BLACK, COLOR_WHITE);
		dw_draw_wrapped_text_fits_height(x, y, MSG_BOX_W, h, MSG_TEXT_H, 
								   	 COLOR_WHITE, MSG_COMPAT_OPS);
	}
	return;
}

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
static void level_2_tutorial(bool holding_line, bool play)
{
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
	int x = cw_get_code_box_member(MEMBER_X) + (w - MSG_BOX_W)/2;
	int y = final_y + 2*ARROW_H;

	dw_draw_filled_rectangle(x, y, MSG_BOX_W, MSG_BOX_H, COLOR_BLACK, 
																   COLOR_WHITE);
	dw_draw_wrapped_text_fits_height(x, y, MSG_BOX_W, MSG_BOX_H, MSG_TEXT_H, 
										 COLOR_WHITE, MSG_MOV_INS);

	int startx =  ins_x + ARROW_W/2;
	int starty = y - 2*ARROW_H;
	static arrow_t arrow;
	static bool arrow_initialized = false;
	if (arrow_initialized == false){
		arrow.box.w = ARROW_W;	
		arrow.box.h = ARROW_H;
		arrow.box.x = startx;		
		arrow.box.y = starty;
		arrow.texture = g_lv_arrow;
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
	int x = SCREEN_WIDTH/2 - MSG_BOX_W/2;
	int y = SCREEN_HEIGHT/4 - MSG_BOX_H;

	dw_draw_filled_rectangle(x, y, MSG_BOX_W, MSG_BOX_H, COLOR_BLACK, 
																   COLOR_WHITE);
	dw_draw_wrapped_text_fits_height(x, y, MSG_BOX_W, MSG_BOX_H, MSG_TEXT_H, 
								   	 	  	   COLOR_WHITE, MSG_SEL_IB);
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
	int x = cw_get_code_box_member(MEMBER_X) + (w - MSG_BOX_W)/2;
	int y = final_y + 2*ARROW_H;

	dw_draw_filled_rectangle(x, y, MSG_BOX_W, MSG_BOX_H, COLOR_BLACK, 
																   COLOR_WHITE);
	dw_draw_wrapped_text_fits_height(x, y, MSG_BOX_W, MSG_BOX_H, MSG_TEXT_H, 
								   	 	  COLOR_WHITE, MSG_CHANGE_OP);


	int startx =  op_x + ARROW_W/2;
	int starty = y - 2*ARROW_H;
	static arrow_t arrow;
	static bool arrow_initialized = false;
	if (arrow_initialized == false){
		arrow.box.w = ARROW_W;	
		arrow.box.h = ARROW_H;
		arrow.box.x = startx;		
		arrow.box.y = starty;
		arrow.texture = g_lv_arrow;
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
	int x = cw_get_code_box_member(MEMBER_X) + (w - MSG_BOX_W)/2;
	int y = final_y + 2*ARROW_H;

	dw_draw_filled_rectangle(x, y, MSG_BOX_W, MSG_BOX_H, COLOR_BLACK, 
																   COLOR_WHITE);
	dw_draw_wrapped_text_fits_height(x, y, MSG_BOX_W, MSG_BOX_H, MSG_TEXT_H, 
								   COLOR_WHITE, MSG_DEL_INS);

	int startx =  cw_get_code_box_member(MEMBER_X) + w;
	int starty = cw_get_text_box_member(MEMBER_Y);
	static arrow_t arrow;
	static bool arrow_initialized = false;
	if (arrow_initialized == false){
		arrow.box.w = ARROW_W;	
		arrow.box.h = ARROW_H;
		arrow.box.x = startx;		
		arrow.box.y = starty;
		arrow.texture = g_lv_arrow;
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
	int x = cw_get_code_box_member(MEMBER_X) + (w - MSG_BOX_W)/2;
	int y = final_y + 2*ARROW_H;

	dw_draw_filled_rectangle(x, y, MSG_BOX_W, MSG_BOX_H, COLOR_BLACK, 
																   COLOR_WHITE);
	dw_draw_wrapped_text_fits_height(x, y, MSG_BOX_W, MSG_BOX_H, MSG_TEXT_H, 
								 COLOR_WHITE, MSG_SEL_LAST_INS);

	int startx = cw_get_code_line_x(MOV) + ARROW_W/2;
	int starty = y - ARROW_H;
	static arrow_t arrow;
	static bool arrow_initialized = false;
	if (arrow_initialized == false){
		arrow.box.w = ARROW_W;	
		arrow.box.h = ARROW_H;
		arrow.box.x = startx;		
		arrow.box.y = starty;
		arrow.texture = g_lv_arrow;
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
		message_box(INS_BOX, MSG_SEL_INS1);
		display_arrow(INS_ARROW);
	} else if (code_size == 0 && holding_line == true){
		message_box(INS_BOX, MSG_DROP_INS1);
		display_arrow(DROP_CODE_ARROW);
	} else if (code_size == 1 && cw_check_code_sorted() == true &&
								   cw_check_code_pending_op1() == true){
		message_box(CODE_BOX, MSG_SEL_OP1);	
		rg_display_registers(true);
	} else if(code_size == 1 && cw_check_code_sorted() == true &&
				    			   cw_check_code_pending_operand() == true){
		message_box(CODE_BOX, MSG_SEL_OP2);	
		rg_display_registers(false);
		bf_draw_buffers(IB);
	} else if(code_size == 1 && holding_line == false &&
 								  cw_check_code_pending_operand() == false){
		message_box(INS_BOX, MSG_SEL_INS2);
		display_arrow(INS_ARROW);
	} else if(code_size == 1 && holding_line == true &&
 								  cw_check_code_pending_operand() == false){
		message_box(INS_BOX, MSG_DROP_INS2);
		display_arrow(DROP_CODE_ARROW);
	} else if (code_size == 2 && cw_check_code_sorted() == true &&
								   cw_check_code_pending_op1() == true){
		message_box(CODE_BOX, MSG_SEL_OP1);	
		bf_draw_buffers(OB);
	} else if(code_size == 2 && cw_check_code_sorted() == true &&
				    			   cw_check_code_pending_operand() == true){
		message_box(CODE_BOX, MSG_SEL_OP2);	
		rg_display_registers(true);
	} else if(code_size == 2 && cw_check_code_pending_operand() == false &&
															 play == false){
		message_box(SB_BOX, MSG_PRESS_PLAY);	
		display_arrow(PLAY_ARROW);
	//	tutorial_press_play();
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
	int x = cw_get_code_box_member(MEMBER_X) + (w - MSG_BOX_W)/2;
	int y = (cw_get_code_box_member(MEMBER_Y)+ h) - MSG_BOX_H/2;

	static arrow_t arrow;
	static bool arrow_initialized = false;
	int startx = sb_get_play_button_member(MEMBER_X);
	int starty = STAGE_BUTTON_Y - 2*ARROW_H;
	if (arrow_initialized == false){
		arrow.box.w = ARROW_W;	
		arrow.box.h = ARROW_H;
		arrow.box.x = startx;
		arrow.box.y = starty;
		arrow.texture = g_lv_arrow;
		arrow.in_place = false;
		arrow_initialized = true;
	}
	int travel = STAGE_BUTTON_Y - starty - ARROW_H;	
	dw_animate_arrow(startx, starty, &arrow, DW_DOWN, travel);
	

	dw_draw_filled_rectangle(x, y, MSG_BOX_W, MSG_BOX_H, COLOR_BLACK, 
																   COLOR_WHITE);
	dw_draw_wrapped_text_fits_height(x, y, MSG_BOX_W, MSG_BOX_H, MSG_TEXT_H, 
											  COLOR_WHITE, MSG_PRESS_PLAY);
		return ;
}




/* Function: check_display_reg_lv_arrow
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
		if (l->ins->id != JMP && l->ins->id !=LABEL){
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
	}
	return display_arrow;
}

/* Function: check_display_reg_lv_arrow
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
static bool check_display_reg_lv_arrow() 
{
	bool display_arrow = false;
	if (cw_check_code_sorted() == true && 
									   cw_check_code_pending_operand() == true){
		code_line_t *l = cw_get_code_line_pending_operand();
		if (l->ins->id != JMP && l->ins->id != LABEL){
			operand_t o;
			o.id = RAX;
			display_arrow = check_operand_compatilibity(&o, l);
		}
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
		new_win = malloc(sizeof(value_box_t));
		new_win->value = cur_input->value;
		new_win->type = cur_input->type;
		List_push(win_list, new_win);
		new_win = malloc(sizeof(value_box_t));
		new_win->value = cur_input->value;
		new_win->type = cur_input->type;
		List_push(win_list, new_win);
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
	
	case LV_LEVEL_3:
			level_3_tutorial(holding_line, play);
			break;
	
	case LV_LEVEL_5:
			level_5_tutorial(holding_line, play);
			break;
	
	case LV_LEVEL_6:
			level_6_tutorial(holding_line, play);
			break;
	
	case LV_LEVEL_9:
			level_9_tutorial(holding_line, play);
			break;

		default:
			bf_draw_buffers(check_display_buf_arrow());
			rg_display_registers(check_display_reg_lv_arrow());
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
