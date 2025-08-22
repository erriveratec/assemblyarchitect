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

#define MSG_SEL_INS1 "Let's move the value from the Input Buffer [IB] to the"\
					 " register. Select and drag an instruction from the"\
									  " instruction box"
#define MSG_SEL_INS2 "Select another instruction from the" \
									  " instruction box"
#define MSG_DROP_INS1 "Drop the instruction in the code box"
#define MSG_DROP_INS2 "Drop the instruction in the code box"\
									" below the previous instruction"
#define MSG_SEL_OP1 "The destiny operand is where the"\
					" recovered value will be stored."\
					" Select \"rax\" as the destiny operand."
#define MSG_SEL_OP2 "The secondd operand is from where the value will be"\
					" recovered. Select the Input Buffer [IB] as the source"\
					" operand."
#define MSG_PLAY_TUT "Let's see what that instruction does. Press the play"\
					 " button."
#define MSG_PRESS_PLAY	"Press the play button"
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
								" try using them to solve the challenge."
#define MSG_NEW_INS_ADD "There is a new instruction \"add\" which adds"\
							 " the contents of the two operands and it stores"\
							 " it in the destiny operand."
#define MSG_NEW_INS_JMP "The \"jmp\" instruction jumps to a position"\
							 " pointed by the label. The operand of the"\
							 " instruction is the label."

#define MSG_FST_CHALLENGE "Welcome to the first challenge, click"\
							" anywhere to continue"

#define MSG_OBJECTIVE1 "The objective of the challenge is to move items from"\
						" the Input Buffer [IB]. (Press click to continue)."
#define MSG_OBJECTIVE2 "To the Output Buffer [OB]. (Press click to continue)."

#define MSG_OBJECTIVE3 "All values recovered from the Input Buffer, must go"\
					   " to a register first, in this case \"rax\"."\
					   " (Press click to continue)."


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
#define ARROW_CODE_Y (MSG_INS_BOX_Y + MSG_BOX_H + ARROW_H)

#define BIG_MSG_BOX_X (SCREEN_WIDTH*3/5)
#define BIG_MSG_BOX_Y (SCREEN_HEIGHT/4)
#define BIG_MSG_BOX_W (SCREEN_WIDTH/3)
#define BIG_MSG_BOX_H (SCREEN_HEIGHT/2)
#define BIG_MSG_TEXT_H 50




//Strings for the win condition
#define STR_WIN1 "WIN1"
#define STR_WIN2 "WIN2"

#define WIN_CONDITION_LENGTH 30

static List *win_list = NULL;
static int level_instructions_limit;
static char level_win_condition[WIN_CONDITION_LENGTH];


texture_t *g_lv_arrow;
arrow_t g_arrow_ins; 
arrow_t g_arrow_code_box;
arrow_t g_arrow_play;
arrow_t g_arrow_code_line;
arrow_t g_arrow_del;
arrow_t g_arrow_op2;

enum message{
	MESSAGE_1,
	MESSAGE_2,
	TRY_YOURSELF,
	NEW_INS_ADD,
	NEW_INS_JMP
};

enum positions{
	INS_BOX,
	UPPER_BOX,
	LOWER_BOX,
	CODE_BOX,
	SB_BOX,
	BIG_BOX
};

enum arrow_id{
	INS_ARROW,
	DROP_CODE_ARROW,
	PLAY_ARROW,
	CODE_LINE_ARROW,
	DEL_ARROW,
	OP2_ARROW
};


SDL_Rect g_msg_ins_box = {MSG_INS_BOX_X, MSG_INS_BOX_Y, MSG_BOX_W, MSG_BOX_H};
SDL_Rect g_msg_upper_box = {SCREEN_WIDTH/2 - MSG_BOX_W/2, 
							 SCREEN_HEIGHT/4 - MSG_BOX_H, MSG_BOX_W, MSG_BOX_H};
SDL_Rect g_msg_lower_box = {SCREEN_WIDTH/2 - MSG_BOX_W/2, 
						   SCREEN_HEIGHT*8/9 - MSG_BOX_H, MSG_BOX_W, MSG_BOX_H};
SDL_Rect g_msg_code_box = {CODE_BOX_X + (CODE_BOX_W - MSG_BOX_W)/2, 
						   MSG_INS_BOX_Y, MSG_BOX_W, MSG_BOX_H};
SDL_Rect g_msg_sb_box = {CODE_BOX_X + (CODE_BOX_W - MSG_BOX_W)/2, 
			     (CODE_BOX_Y + CODE_BOX_H) - MSG_BOX_H/2, MSG_BOX_W, MSG_BOX_H};

SDL_Rect g_msg_big_box = {BIG_MSG_BOX_X, BIG_MSG_BOX_Y, BIG_MSG_BOX_W, 
																 BIG_MSG_BOX_H};

List *get_win_list();
void add_to_win_list(int value, int type);
static void level_1_tutorial(bool holding_line, bool play);
static void level_2_tutorial(bool holding_line, bool play);
static void level_3_tutorial(bool holding_line, bool play);
static void level_5_tutorial(bool holding_line, bool play);
static void level_6_tutorial(bool holding_line, bool play);
static bool check_display_reg_lv_arrow();
static int check_display_buf_arrow();
static void win1_move_input_to_output(int rep, int mul, bool reversed);
static void win2_add_inputs_in_groups(int group_size);
static void message_box(int pos, char *msg);
static void set_win_condition(char *win_condition);

void lv_initialize_level_assets()
{
	g_arrow_ins.box.x = ARROW_INS_X; 
	g_arrow_ins.box.y = ARROW_INS_Y;
	g_arrow_ins.box.w = ARROW_W; 
	g_arrow_ins.box.h = ARROW_H;
	g_arrow_ins.texture =  g_lv_arrow;
	g_arrow_ins.in_place = false;
	g_arrow_ins.visible =  true;

	g_arrow_code_box.box.x = ARROW_CODE_X;
	g_arrow_code_box.box.y = ARROW_CODE_Y;
	g_arrow_code_box.box.w = ARROW_W;
	g_arrow_code_box.box.h = ARROW_H;
	g_arrow_code_box.texture = g_lv_arrow;
	g_arrow_code_box.in_place = false;
	g_arrow_code_box.visible = true;

	g_arrow_play.box.w = ARROW_W;	
	g_arrow_play.box.h = ARROW_H;
	g_arrow_play.box.x = sb_get_play_button_member(MEMBER_X);
	g_arrow_play.box.y = STAGE_BUTTON_Y - 2*ARROW_H;
	g_arrow_play.texture = g_lv_arrow;
	g_arrow_play.in_place = false;
	g_arrow_play.visible = true;

	g_arrow_code_line.box.w = ARROW_W;	
	g_arrow_code_line.box.h = ARROW_H;
	g_arrow_code_line.box.x = cw_get_code_line_x(MOV) + ARROW_W/2;
	g_arrow_code_line.box.y = cw_get_line_y(cw_get_code_list_size()-1) + 
													    CODE_BUTTON_H + ARROW_H;
	g_arrow_code_line.texture = g_lv_arrow;
	g_arrow_code_line.in_place = false;
	g_arrow_code_line.visible = true;

	g_arrow_del.box.x = cw_get_code_box_member(MEMBER_X) +
											   cw_get_code_box_member(MEMBER_W);
	g_arrow_del.box.y = cw_get_text_box_member(MEMBER_Y);
	g_arrow_del.box.w = ARROW_W;
	g_arrow_del.box.h = ARROW_H;
	g_arrow_del.texture = g_lv_arrow;
	g_arrow_del.in_place = false;
	g_arrow_del.visible = true;

	g_arrow_op2.box.x = cw_get_code_line_x(MOV) + OP2_X_OFFSET + ARROW_W/2;
	g_arrow_op2.box.y = cw_get_line_y(cw_get_code_list_size()-1) + ARROW_H;
	g_arrow_op2.box.w = ARROW_W;
	g_arrow_op2.box.h = ARROW_H;
	g_arrow_op2.texture = g_lv_arrow;
	g_arrow_op2.in_place = false;
	g_arrow_op2.visible = true;

	//The code box will accommodate according to the number of instructions
	g_msg_code_box.y = g_arrow_code_line.box.y + ARROW_H; 
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
			travel = cw_get_code_box_member(MEMBER_X) - 
													   (ARROW_CODE_X + ARROW_W); 
			dir = DW_RIGHT;
			aptr = &g_arrow_code_box;
			break;
		case PLAY_ARROW:
			x = STAGE_BUTTON_X;
			y = STAGE_BUTTON_Y - 2*ARROW_H;	
			travel = STAGE_BUTTON_Y - y - ARROW_H;	
			dir = DW_DOWN;
			aptr = &g_arrow_play;
			break;
		case CODE_LINE_ARROW:
			x = cw_get_code_line_x(MOV) + ARROW_W/2;
			y = cw_get_line_y(cw_get_code_list_size()-1) + CODE_BUTTON_H + 
																	    ARROW_H;
			travel = y - cw_get_line_y(cw_get_code_list_size()-1);	
			dir = DW_UP;
			aptr = &g_arrow_code_line;
			break;
		case DEL_ARROW:
			x = cw_get_code_box_member(MEMBER_X) + 
											   cw_get_code_box_member(MEMBER_W);
			y = cw_get_text_box_member(MEMBER_Y);
			travel = ARROW_W;
			dir = DW_RIGHT;
			aptr = &g_arrow_del;
			break;
		case OP2_ARROW:
			x = cw_get_code_line_x(MOV) + OP2_X_OFFSET + ARROW_W/2;
			y = cw_get_line_y(cw_get_code_list_size()-1) + CODE_BUTTON_H; 			
			travel = ARROW_W;
			dir = DW_UP;
			aptr = &g_arrow_op2;
			break;

		default:
			break;
	}
	dw_animate_arrow(x, y, aptr, dir, travel);
}


/*	SDL_SetTextureColorMod(arrow.texture->texture, 255, 0, 255);
	dw_animate_arrow(startx, starty, &arrow, DW_UP, travel);
	SDL_SetTextureColorMod(arrow.texture->texture, 255, 255, 255);
*/

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
	int text_h;
	SDL_Color color;
	switch(pos){
		case INS_BOX:
			b.x = g_msg_ins_box.x; 
			b.y = g_msg_ins_box.y; 
			b.w = g_msg_ins_box.w; 
			b.h = g_msg_ins_box.h;
			text_h = MSG_TEXT_H;
			color = COLOR_WHITE;
			break;
		case UPPER_BOX:
			b.x = g_msg_upper_box.x; 
			b.y = g_msg_upper_box.y; 
			b.w = g_msg_upper_box.w; 
			b.h = g_msg_upper_box.h;	
			text_h = MSG_TEXT_H;
			color = COLOR_WHITE;
			break;
		case LOWER_BOX:
			b.x = g_msg_lower_box.x; 
			b.y = g_msg_lower_box.y; 
			b.w = g_msg_lower_box.w; 
			b.h = g_msg_lower_box.h;	
			text_h = MSG_TEXT_H;
			color = COLOR_WHITE;
			break;
		case CODE_BOX:
			b.x = g_msg_code_box.x; 
			b.y = g_msg_code_box.y; 
			b.w = g_msg_code_box.w; 
			b.h = g_msg_code_box.h;	
			text_h = MSG_TEXT_H;
			color = COLOR_WHITE;
			break;
		case SB_BOX:
			b.x = g_msg_sb_box.x; 
			b.y = g_msg_sb_box.y; 
			b.w = g_msg_sb_box.w; 
			b.h = g_msg_sb_box.h;	
			text_h = MSG_TEXT_H;
			color = COLOR_WHITE;
			break;
		case BIG_BOX:
			b.x = g_msg_big_box.x; 
			b.y = g_msg_big_box.y; 
			b.w = g_msg_big_box.w; 
			b.h = g_msg_big_box.h;	
			text_h = BIG_MSG_TEXT_H;
			color = COLOR_ORANGE;
			break;

	}
	dw_draw_filled_rectangle(b.x, b.y, b.w, b.h, COLOR_BLACK, COLOR_WHITE);
	dw_draw_wrapped_text_fits_height(b.x, b.y, b.w, b.h, text_h, color, msg);
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
	rg_draw_registers(check_display_reg_lv_arrow());

	int code_size = cw_get_code_list_size();

	if (code_size == 0 && holding_line == false){
		message_box(UPPER_BOX, MSG_NEW_INS_JMP);
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
	rg_draw_registers(check_display_reg_lv_arrow());

	int code_size = cw_get_code_list_size();

	if (code_size == 0 && holding_line == false){
		message_box(UPPER_BOX, MSG_NEW_INS_ADD);
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
	rg_draw_registers(check_display_reg_lv_arrow());

	int code_size = cw_get_code_list_size();

	if (code_size == 0 && holding_line == false){
		message_box(UPPER_BOX, MSG_NEW_REGS);
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
	rg_draw_registers(check_display_reg_lv_arrow());

	int code_size = cw_get_code_list_size();

	if (code_size == 0 && holding_line == false){
		message_box(INS_BOX, MSG_SEL_INS1);
		display_arrow(INS_ARROW);
	} else if (code_size == 0 && holding_line == true){
		message_box(INS_BOX, MSG_DROP_INS1);
		display_arrow(DROP_CODE_ARROW);
	} else if (code_size == 1 && cw_check_code_pending_op1() == true && 
												cw_check_code_sorted() == true){
		message_box(LOWER_BOX, MSG_AVAIL_OPS);
	} else if (code_size == 1 && cw_check_code_pending_op2() == true && 
												cw_check_code_sorted() == true){
		message_box(UPPER_BOX, MSG_COMPAT_OPS);
	} else if (code_size == 1 && cw_check_code_pending_operand() == false){
		message_box(UPPER_BOX, MSG_TRY_YOURSELF);
		display_arrow(INS_ARROW);
	}
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
	rg_draw_registers(false);
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
		message_box(CODE_BOX, MSG_SEL_LAST_INS);
		display_arrow(CODE_LINE_ARROW);
	} else if (code_size > level_instructions_limit && holding_line == true){
		message_box(CODE_BOX, MSG_DEL_INS);
		display_arrow(DEL_ARROW);
	} else if (change_op == true && holding_line == false){
		if (i2->state != CHANGING_OP2){
			message_box(CODE_BOX, MSG_CHANGE_OP);
			display_arrow(OP2_ARROW);
		} else if (i2->state == CHANGING_OP2){
			message_box(UPPER_BOX, MSG_SEL_IB);
			bf_draw_buffers(IB);
		}
	} else if (mov_instruction == true){
		message_box(CODE_BOX, MSG_MOV_INS);
		display_arrow(CODE_LINE_ARROW);
	} else if (press_play == true){
		message_box(SB_BOX, MSG_PRESS_PLAY);	
		display_arrow(PLAY_ARROW);
	}
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
	rg_draw_registers(false);
	static bool fst_message = true;	
	static bool snd_message = true;	
	static bool trd_message = true;	
	static bool fth_message = true;

	if (fst_message == true && code_size == 0){
		message_box(BIG_BOX, MSG_FST_CHALLENGE);
		if (ms_chk_mouse_left_pressed() == true){
			fst_message = false;
			ms_reset_mouse_values();
		}
	} else if (snd_message == true && code_size == 0){
		message_box(UPPER_BOX, MSG_OBJECTIVE1);
		if (ms_chk_mouse_left_pressed() == true){
			snd_message = false;
		}
		bf_draw_buffers(IB);
		ms_reset_mouse_values();
	} else if (trd_message == true && code_size == 0){
		message_box(LOWER_BOX, MSG_OBJECTIVE2);
		if (ms_chk_mouse_left_pressed() == true){
			trd_message = false;
		}
		bf_draw_buffers(OB);
		ms_reset_mouse_values();
	} else if (fth_message == true && code_size == 0){
		message_box(UPPER_BOX, MSG_OBJECTIVE3);
		if (ms_chk_mouse_left_pressed() == true){
			fth_message = false;
		}
		rg_draw_registers(true);
		ms_reset_mouse_values();
	} else if (code_size == 0 && holding_line == false){
		message_box(INS_BOX, MSG_SEL_INS1);
		display_arrow(INS_ARROW);
	} else if (code_size == 0 && holding_line == true){
		message_box(INS_BOX, MSG_DROP_INS1);
		display_arrow(DROP_CODE_ARROW);
	} else if (code_size == 1 && cw_check_code_sorted() == true &&
								   cw_check_code_pending_op1() == true){
		message_box(CODE_BOX, MSG_SEL_OP1);	
		rg_draw_registers(true);
	} else if(code_size == 1 && cw_check_code_sorted() == true &&
				    			   cw_check_code_pending_operand() == true){
		message_box(CODE_BOX, MSG_SEL_OP2);	
		rg_draw_registers(false);
		bf_draw_buffers(IB);
	} else if(code_size == 1 && holding_line == false &&
 								  cw_check_code_pending_operand() == false){
		message_box(SB_BOX, MSG_PLAY_TUT);	
		display_arrow(PLAY_ARROW);
		//message_box(INS_BOX, MSG_SEL_INS2);
		//display_arrow(INS_ARROW);
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
		rg_draw_registers(true);
	} else if(code_size == 2 && cw_check_code_pending_operand() == false &&
															 play == false){
		message_box(SB_BOX, MSG_PRESS_PLAY);	
		display_arrow(PLAY_ARROW);
	} 

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

/* Function: lv_reset_win_condition
 * -----------------------------------------------------------------------------
 * This function resets the win condition when the player resets the level
 *  
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
void lv_reset_level_win_condition()
{
	set_win_condition(level_win_condition);
}

/* Function: lv_level_win_condition
 * -----------------------------------------------------------------------------
 * Copies the win condition to the level variable and creates the win condition
 *  
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
void lv_set_level_win_condition(char *win_condition)
{
	strcpy(level_win_condition, win_condition);
	set_win_condition(level_win_condition);
}

/* Function: set_win_condition
 * -----------------------------------------------------------------------------
 * This function creates the win condition according to what is in the levels
 * file. 
 *
 * Arguments:
 * 	text: The text with the description of the win condition.
 *	
 * Return:
 *	void
 */
static void set_win_condition(char *win_condition)
{
	assert(win_condition != NULL && "NULL win condition text");
	char win_condition_cpy[WIN_CONDITION_LENGTH];
	strcpy(win_condition_cpy, win_condition);
	char *saveptr1;
	char *delim = char_space;
	char *win_cond;
	
	win_cond =  strtok_r(win_condition_cpy, delim, &saveptr1);
	if (strstr(win_cond, STR_WIN1) != NULL){
		char *rep_text = strtok_r(NULL, delim, &saveptr1);
		int rep = atoi(rep_text);
		char *mul_text = strtok_r(NULL, delim, &saveptr1);
		int mul = atoi(mul_text);
		char *reversed = strtok_r(NULL, delim, &saveptr1);
		bool rev;
		if (strstr(reversed, "true") != NULL){
			rev = true;
		} else if (strstr(reversed, "false") != NULL){
			rev = false;
		}
		win1_move_input_to_output(rep, mul, rev);
	} else if (strstr(win_cond, STR_WIN2) != NULL){
		char *group_size_text = strtok_r(NULL, delim, &saveptr1);
		int group_size = atoi(group_size_text);
		win2_add_inputs_in_groups(group_size);
	}
	return;
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
/* Function: win1_move_input_to_output
 *------------------------------------------------------------------------------
 * Generates a win condition that is achieved by moving the elements from the
 * input buffer to the output buffer in order. Can apply repetitions and 
 * transformations to the output list if needed as its implementation is easy.
 *
 * Arguments:
 *	rep: number of times a element of the IB will be copied to the OB.
 *  mul: multiplication transformation to the output buffer.
 *
 * Return:
 *	Void.
 */
static void win1_move_input_to_output(int rep, int mul, bool reversed)
{
	List *input_list = get_input_list();
	List *win_list = get_win_list();

	assert(rep > 0 && "The number of repetitions is less than 1");
	assert(mul > 0 && "The mul factor es less than 1");
	assert(input_list != NULL && "Input list pointer is NULL");
	assert(win_list != NULL && "Win list pointer is NULL");

	int input_list_size = List_count(input_list);
	int win_list_size = List_count(win_list);

	assert(input_list_size > 0 && "The size of the input list is incorrect");
	assert(win_list_size == 0 && "The win list has elements");

	LIST_FOREACH(input_list, first, next, cur){
		value_box_t *cur_input = cur->value;
		value_box_t *new_win; 
		for (int i = 0; i < rep; i++){
			new_win = malloc(sizeof(value_box_t));
			new_win->value = mul*cur_input->value;
			new_win->type = cur_input->type;
			if (reversed == false){
				List_push(win_list, new_win);
			} else if (reversed == true){
				List_unshift(win_list, new_win);
			}
		}
	}
}


/* Function: win2_add_inputs_in_groups
 *------------------------------------------------------------------------------
 * The solution of the challenge will be achieved is the player adds the inputs
 * in groups
 *
 * Arguments:
 *	group_size: The size of the input group tha will be added
 *
 * Return:
 *	Void.
 */
static void win2_add_inputs_in_groups(int group_size)
{
	List *input_list = get_input_list();
	List *win_list = get_win_list();

	assert(input_list != NULL && "Input list pointer is NULL");
	assert(win_list != NULL && "Win list pointer is NULL");

	int input_list_size = List_count(input_list);
	assert(input_list_size % group_size == 0 && 
					"The input size must be a multiple of group size");
	
	int win_list_size = List_count(win_list);
	assert(input_list_size > 0 && "The size of the input list is incorrect");
	assert(win_list_size == 0 && "The win list has elements");

	int res = 1;
	int array_index = 0;
	int *values = malloc(sizeof(int)*win_list_size);
			
	LIST_FOREACH(input_list, first, next, cur){
		value_box_t *cur_input = cur->value;
		values[array_index] = cur_input->value;
		if (res % group_size == 0){
			value_box_t *new_win = malloc(sizeof(value_box_t));
			int val = 0;
			for (int i = 0; i < group_size; i++){
				val += values[array_index - i];
			}
			new_win->value = val;
			new_win->type = cur_input->type;
			List_push(win_list, new_win);
		}
		res++;
		array_index++;
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
			rg_draw_registers(check_display_reg_lv_arrow());
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
