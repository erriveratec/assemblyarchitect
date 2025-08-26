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
#include "text_tx.h"
#include "arrow_ar.h"

//Strings for the win condition
#define STR_WIN1 "WIN1"
#define STR_WIN2 "WIN2"

#define WIN_CONDITION_LENGTH 30

static List *win_list = NULL;
static int level_instructions_limit;
static char level_win_condition[WIN_CONDITION_LENGTH];



List *get_win_list();
void add_to_win_list(int value, int type);
static void level_1_tutorial(bool holding_line, bool play, int flag);
static void level_2_tutorial(bool holding_line, bool play);
static void level_3_tutorial(bool holding_line, bool play);
static void level_4_tutorial(bool holding_line, bool play);
static void level_5_tutorial(bool holding_line, bool play);
static void level_6_tutorial(bool holding_line, bool play);
static void level_7_tutorial(bool holding_line, bool play);
static void level_8_tutorial(bool holding_line, bool play);
static void level_9_tutorial(bool holding_line, bool play);
static bool check_display_reg_lv_arrow();
static int check_display_buf_arrow();
static void win1_move_input_to_output(int rep, int mul, bool reversed);
static void win2_add_inputs_in_groups(int group_size);
static void set_win_condition(char *win_condition);


	


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

	
	const int pos_one = 0;
	int code_size = cw_get_code_list_size();
	static bool first_message = true;
	static bool second_message = true;
	static bool third_message = true;
	
	code_line_t *i1= NULL;

	if (code_size == 1 && holding_line == false){
		i1 = cw_get_code_line_at_pos(pos_one);
	}
	
	if (first_message == true && code_size == 0){
		tx_text_box(TX_BIG_BOX, L9_MSG_NINTH_CHALLENGE);
		if (ms_chk_mouse_left_pressed() == true){
			first_message = false;
			ms_reset_mouse_values();
		}
	} else if (second_message == true && code_size == 0){
		tx_text_box(TX_UPPER_BOX, L9_MSG_NEW_INS_JMP);
		if (ms_chk_mouse_left_pressed() == true){
			second_message = false;
			ms_reset_mouse_values();
		}
	} else if (third_message == true && code_size == 0){
		tx_text_box(TX_UPPER_BOX, L9_MSG_RESTRICTION);
		ar_display_arrow(AR_CHALLENGE);
		if (ms_chk_mouse_left_pressed() == true){
			third_message = false;
			ms_reset_mouse_values();
		}
	} else if (code_size == 0 && holding_line == false){
		tx_text_box(TX_INS_BOX, L9_MSG_SELECT_LABEL);
		ar_display_arrow(AR_INS);
	} else if (code_size == 0 && holding_line == true){
		tx_text_box(TX_INS_BOX, MSG_DROP_INS1);
		ar_display_arrow(AR_DROP);
	} 
}

/* Function: level_8_tutorial
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 3
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_8_tutorial(bool holding_line, bool play)
{
	bf_draw_buffers(check_display_buf_arrow());
	rg_draw_registers(check_display_reg_lv_arrow());

	int code_size = cw_get_code_list_size();
	static bool first_message = true;
	static bool second_message = true;

	if (first_message == true && code_size == 0){
		tx_text_box(TX_BIG_BOX, L8_MSG_EIGHTH_CHALLENGE);
		if (ms_chk_mouse_left_pressed() == true){
			first_message = false;
			ms_reset_mouse_values();
		}
	} else if (second_message == true && code_size == 0){
		tx_text_box(TX_UPPER_BOX, L8_MSG_ADD);
		if (ms_chk_mouse_left_pressed() == true){
			second_message = false;
			ms_reset_mouse_values();
		}
	} 
}
/* Function: level_7_tutorial
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 3
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_7_tutorial(bool holding_line, bool play)
{
	bf_draw_buffers(check_display_buf_arrow());
	rg_draw_registers(check_display_reg_lv_arrow());

	int code_size = cw_get_code_list_size();
	static bool first_message = true;
	static bool second_message = true;

	if (first_message == true && code_size == 0){
		tx_text_box(TX_BIG_BOX, L7_MSG_SEVENTH_CHALLENGE);
		if (ms_chk_mouse_left_pressed() == true){
			first_message = false;
			ms_reset_mouse_values();
		}
	} else if (second_message == true && code_size == 0){
		tx_text_box(TX_UPPER_BOX, L7_MSG_ADD);
		if (ms_chk_mouse_left_pressed() == true){
			second_message = false;
			ms_reset_mouse_values();
		}
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
	static bool first_message = true;
	static bool second_message = true;

	if (first_message == true && code_size == 0){
		tx_text_box(TX_BIG_BOX, L6_MSG_SIXTH_CHALLENGE);
		if (ms_chk_mouse_left_pressed() == true){
			first_message = false;
			ms_reset_mouse_values();
		}
	} else if (second_message == true && code_size == 0){
		tx_text_box(TX_UPPER_BOX, L6_MSG_ADD);
		if (ms_chk_mouse_left_pressed() == true){
			second_message = false;
			ms_reset_mouse_values();
		}
	} else if (code_size == 0 && holding_line == false){
		tx_text_box(TX_INS_BOX, L6_MSG_SOLVE);
		ar_display_arrow(AR_INS);
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
	static bool first_message = true;

	if (first_message == true && code_size == 0){
		tx_text_box(TX_BIG_BOX, L5_MSG_FIFTH_CHALLENGE);
		if (ms_chk_mouse_left_pressed() == true){
			first_message = false;
			ms_reset_mouse_values();
		}
	} 
}

/* Function: level_4_tutorial
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 3
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_4_tutorial(bool holding_line, bool play)
{
	bf_draw_buffers(check_display_buf_arrow());
	rg_draw_registers(check_display_reg_lv_arrow());

	static bool first_message = true;
	int code_size = cw_get_code_list_size();

	if (first_message == true && code_size == 0){
		tx_text_box(TX_BIG_BOX, L4_MSG_FOURTH_CHALLENGE);
		if (ms_chk_mouse_left_pressed() == true){
			first_message = false;
			ms_reset_mouse_values();
		}
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
	static bool first_message = true;

	if (first_message == true && code_size == 0){
		tx_text_box(TX_BIG_BOX, L3_MSG_THIRD_CHALLENGE);
		if (ms_chk_mouse_left_pressed() == true){
			first_message = false;
			ms_reset_mouse_values();
		}
	} else if (code_size == 0 && holding_line == false){
		tx_text_box(TX_INS_BOX, MSG_SEL_INS1);
		ar_display_arrow(AR_INS);
	} else if (code_size == 0 && holding_line == true){
		tx_text_box(TX_INS_BOX, MSG_DROP_INS1);
		ar_display_arrow(AR_DROP);
	} else if (code_size == 1 && cw_check_code_pending_op1() == true && 
												cw_check_code_sorted() == true){
		tx_text_box(TX_LOWER_BOX, L3_MSG_AVAIL_OPS_1);
	} else if (code_size == 1 && cw_check_code_pending_op2() == true && 
												cw_check_code_sorted() == true){
		tx_text_box(TX_UPPER_BOX, L3_MSG_AVAIL_OPS_2);
	} else if (code_size == 1 && cw_check_code_pending_operand() == false &&
														 holding_line == false){
		tx_text_box(TX_INS_BOX, L3_MSG_RECOVERED);
		ar_display_arrow(AR_INS);
	} else if (code_size == 1 && cw_check_code_pending_operand() == false &&
														  holding_line == true){
		tx_text_box(TX_INS_BOX, MSG_DROP_INS1);
		ar_display_arrow(AR_DROP);
	}else if (code_size == 2 && cw_check_code_pending_operand() == true){
		tx_text_box(TX_CODE_BOX, L3_MSG_READ_RAX);
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
	static bool fst_message = true;

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
	if (fst_message == true && code_size == 3){
		tx_text_box(TX_BIG_BOX, L2_MSG_SECOND_CHALLENGE);
		if (ms_chk_mouse_left_pressed() == true){
			fst_message = false;
			ms_reset_mouse_values();
		}
	}
	if (fst_message == true && code_size == 0){
		tx_text_box(TX_BIG_BOX, L1_MSG_FIRST_CHALLENGE);
		if (ms_chk_mouse_left_pressed() == true){
			fst_message = false;
			ms_reset_mouse_values();
		}
	}else if (code_size > level_instructions_limit && holding_line == false){
		tx_text_box(TX_CODE_BOX, L2_MSG_SEL_LAST_INS);
		ar_display_arrow(AR_CODE);
	} else if (code_size > level_instructions_limit && holding_line == true){
		tx_text_box(TX_CODE_BOX, L2_MSG_DEL_INS);
		ar_display_arrow(AR_DEL);
	} else if (change_op == true && holding_line == false){
		if (i2->state != CHANGING_OP2){
			tx_text_box(TX_CODE_BOX, L2_MSG_CHANGE_OP);
			ar_display_arrow(AR_OP2);
		} else if (i2->state == CHANGING_OP2){
			tx_text_box(TX_UPPER_BOX, L2_MSG_SEL_IB);
			bf_draw_buffers(IB);
		}
	} else if (mov_instruction == true){
		tx_text_box(TX_CODE_BOX, L2_MSG_MOV_INS);
		ar_display_arrow(AR_CODE);
	} else if (press_play == true){
		tx_text_box(TX_SB_BOX, MSG_PRESS_PLAY);	
		ar_display_arrow(AR_PLAY);
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
static void level_1_tutorial(bool holding_line, bool play, int flag)
{
	int code_size = cw_get_code_list_size();
	bf_draw_buffers(NO_OPERAND);
	rg_draw_registers(false);
	static bool first_message = true;
	static bool second_message = true;	
	static bool third_message = true;	
	static bool fourth_message = true;	
	static bool fifth_message = true;
	static bool play_message = true;

	if (flag != 0){
		tx_text_box(TX_ERROR_BOX, L1_MSG_ERROR);
		ar_display_arrow(AR_ERROR);
	} else if (first_message == true && code_size == 0){
		tx_text_box(TX_BIG_BOX, L1_MSG_FIRST_CHALLENGE);
		if (ms_chk_mouse_left_pressed() == true){
			first_message = false;
			ms_reset_mouse_values();
		}
	} else if (second_message == true && code_size == 0){
		tx_text_box(TX_UPPER_BOX, L1_MSG_CHALLENGE_DESCRIPTION);
		ar_display_arrow(AR_CHALLENGE);
		if (ms_chk_mouse_left_pressed() == true){
			second_message = false;
			ms_reset_mouse_values();
		}
	} else if (third_message == true && code_size == 0){
		tx_text_box(TX_UPPER_BOX, MSG_OBJECTIVE1);
		if (ms_chk_mouse_left_pressed() == true){
			third_message = false;
		}
		bf_draw_buffers(IB);
		ms_reset_mouse_values();
	} else if (fourth_message == true && code_size == 0){
		tx_text_box(TX_LOWER_BOX, MSG_OBJECTIVE2);
		if (ms_chk_mouse_left_pressed() == true){
			fourth_message = false;
		}
		bf_draw_buffers(OB);
		ms_reset_mouse_values();
	} else if (fifth_message == true && code_size == 0){
		tx_text_box(TX_UPPER_BOX, MSG_OBJECTIVE3);
		if (ms_chk_mouse_left_pressed() == true){
			fifth_message = false;
		}
		rg_draw_registers(true);
		ms_reset_mouse_values();
	} else if (code_size == 0 && holding_line == false){
		tx_text_box(TX_INS_BOX, MSG_SEL_INS1);
		ar_display_arrow(AR_INS);
	} else if (code_size == 0 && holding_line == true){
		tx_text_box(TX_INS_BOX, MSG_DROP_INS1);
		ar_display_arrow(AR_DROP);
	} else if (code_size == 1 && cw_check_code_sorted() == true &&
								   cw_check_code_pending_op1() == true){
		tx_text_box(TX_CODE_BOX, L1_MSG_SEL_OP1);	
		rg_draw_registers(true);
	} else if(code_size == 1 && cw_check_code_sorted() == true &&
				    			   cw_check_code_pending_operand() == true){
		tx_text_box(TX_CODE_BOX, L1_MSG_SEL_OP2_1);	
		rg_draw_registers(false);
		bf_draw_buffers(IB);
	} else if(code_size == 1 && holding_line == false &&
 			  cw_check_code_pending_operand() == false && play_message == true){
		tx_text_box(TX_SB_BOX, L1_MSG_PLAY_TUT);	
		ar_display_arrow(AR_PLAY);
		if (play == true){
			play_message = false;
		}
	} else if(code_size == 1 && holding_line == false && play == false &&
 	         cw_check_code_pending_operand() == false && play_message == false){
		tx_text_box(TX_INS_BOX, L1_MSG_SEL_INS2);	
		ar_display_arrow(AR_INS);
	}else if(code_size == 1 && holding_line == true &&
 								  cw_check_code_pending_operand() == false){
		tx_text_box(TX_INS_BOX, L1_MSG_DROP_INS);
		ar_display_arrow(AR_DROP);
	} else if (code_size == 2 && cw_check_code_sorted() == true &&
								   cw_check_code_pending_op1() == true){
		tx_text_box(TX_CODE_BOX, L1_MSG_SEL_OP1_2);	
		bf_draw_buffers(OB);
	} else if(code_size == 2 && cw_check_code_sorted() == true &&
				    			   cw_check_code_pending_operand() == true){
		tx_text_box(TX_CODE_BOX, L1_MSG_SEL_OP2_2);	
		rg_draw_registers(true);
	} else if(code_size == 2 && cw_check_code_pending_operand() == false &&
															 play == false){
		tx_text_box(TX_SB_BOX, MSG_PRESS_PLAY);	
		ar_display_arrow(AR_PLAY);
	} else if (lv_check_if_win() == true){
		tx_text_box(TX_UPPER_BOX, L1_MSG_CONGRATS);	
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
	int display_ar = NO_OPERAND;
	if (cw_check_code_sorted() == true && 
									   cw_check_code_pending_operand() == true){
		code_line_t *l = cw_get_code_line_pending_operand();
		if (l->ins->id != JMP && l->ins->id !=LABEL){
			operand_t o;
			o.id = IB;
			if (check_operand_compatilibity(&o, l) == true){
				display_ar = IB;
			} 
			o.id = OB;
			if (check_operand_compatilibity(&o, l) == true){
				display_ar = OB;
			}
		}
	}
	return display_ar;
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
	bool display_ar = false;
	if (cw_check_code_sorted() == true && 
									   cw_check_code_pending_operand() == true){
		code_line_t *l = cw_get_code_line_pending_operand();
		if (l->ins->id != JMP && l->ins->id != LABEL){
			operand_t o;
			o.id = RAX;
			display_ar = check_operand_compatilibity(&o, l);
		}
	}
	return display_ar;
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
void lv_level_drawings(int level, bool holding_line, bool play, int flag)
{
	assert(level < LV_LEVEL_MAX && level > LV_LEVEL_MIN && 
			          								"Invalid level value");
	
	switch(level){
		case LV_LEVEL_1:
			level_1_tutorial(holding_line, play, flag);
			break;

		case LV_LEVEL_2:
			level_2_tutorial(holding_line, play);
			break;
	
		case LV_LEVEL_3:
			level_3_tutorial(holding_line, play);
			break;

		case LV_LEVEL_4:
			level_4_tutorial(holding_line, play);
			break;
	
		case LV_LEVEL_5:
			level_5_tutorial(holding_line, play);
			break;
	
		case LV_LEVEL_6:
			level_6_tutorial(holding_line, play);
			break;
		
		case LV_LEVEL_7:
			level_7_tutorial(holding_line, play);
			break;

		case LV_LEVEL_8:
			level_8_tutorial(holding_line, play);
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
 * input list. 
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
