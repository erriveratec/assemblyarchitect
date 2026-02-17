#include <assert.h>
#include <stdbool.h>
#include "levels_lv.h"
#include "list.h"
#include "aux.h"
#include "dbg.h"
#include "buffers_bf.h"
#include "dimensions_dm.h"
#include "registers_rg.h"
#include "instruction_window_iw.h"
#include "code_window_cw.h"
#include "code_line_cl.h"
#include "stage_buttons_sb.h"
#include "text_tx.h"
#include "arrow_ar.h"
#include "immediates_im.h"
#include "file_fl.h"

//Strings for the win condition
#define STR_WIN1 "WIN1"
#define STR_WIN2 "WIN2"
#define STR_WIN3 "WIN3"
#define STR_WIN4 "WIN4"

#define WIN_CONDITION_LENGTH 30
#define LV_MSGS_QTY 15

// Exceptions of the selection of the code
#define NO_EXCEPTION -1
#define INS_EXCEPTION -2
#define OP1_LAST -3
#define OP2_LAST -4



static bool g_code_editable;
static int g_code_editable_exception;
static bool g_buf_selectable;
static bool g_reg_selectable;
static bool g_arng_enabled;
static bool g_del_enabled;

static List *g_win_list = NULL;
static int g_level_instructions_limit;
static char level_win_condition[WIN_CONDITION_LENGTH];
static bool g_lv_msg[LV_MSGS_QTY];

static code_line_t *g_hold_line;
static bool g_play;
static bool g_op_flag;

List *get_win_list();
void add_to_win_list(int value, int type);
static void level_1_tutorial();
static void level_2_tutorial();
static void level_3_tutorial();
static void level_4_tutorial();
static void level_5_tutorial();
static void level_6_tutorial();
static void level_7_tutorial();
static void level_8_tutorial();
static void level_9_tutorial();
static void level_10_tutorial();
static void level_11_tutorial();
static void level_12_tutorial();
static bool check_display_reg_lv_arrow();
static int check_display_buf_arrow();
static bool chk_display_imm_up_arrow();
static void win1_move_input_to_output(int rep, int mul,int sum, bool reversed);
static void win2_add_inputs_in_groups(int grp_size, bool between, int in_val);
static void win3_move_input_to_output_stop(int stop);
static void win4_count_values_till_stop(int element, int stop);
static void set_win_condition(char *win_condition);
static void draw_regs_arrow(bool show_arrows);
static void draw_bufs_arrow(int buf_id);
static void draw_im_up_arrow(bool show_arrows);
static void set_code_editable(bool state, int exception);
static void set_buf_selectable(bool state);
static void set_reg_selectable(bool state);
static void set_arrange_enabled(bool state);
static void init_lv_msgs();
static void chk_ms_pressed_clear_msg(int message_id, bool reset_mouse);
static void chk_ms_released_clear_msg(int message_id, bool reset_mouse);
static bool check_player_is_holding_line();
static code_line_t *get_hold_line();
static bool get_play_state();
static bool get_op_flag();

/* Function: get_play_state
 * ----------------------------------------------------------------------------
 * Return the boolean that indicates if there is an operation flag
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Void.
 */
static bool get_op_flag()
{
	return g_op_flag;
}

/* Function: lv_set_op_flag_state
 * ----------------------------------------------------------------------------
 * Sets the boolen if there is an operation flag active in the state
 *
 * Arguments:
 *	state: State to which the op_flag variable will be set
 *
 * Return:
 *	Void.
 */
void lv_set_op_flag_state(bool state)
{
	g_op_flag = state;
}

/* Function: get_play_state
 * ----------------------------------------------------------------------------
 * Returns the state of the play variables for the levels that requires it
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Void.
 */
static bool get_play_state()
{
	return g_play;
}

/* Function: lv_set_play_state
 * ----------------------------------------------------------------------------
 * Sets the play variables for the levels that requires it 
 *
 * Arguments:
 *	state: State to which the play variable will be set
 *
 * Return:
 *	Void.
 */
void lv_set_play_state(bool state)
{
	g_play = state;
}

/* Function: lv_set_hold_line
 * ----------------------------------------------------------------------------
 * This function has the pointer of the line being hold by the player.
 * Is NULL if the player is not currently holding any line
 *
 * Arguments:
 *	line: Pointer of the line being hold by the player
 *
 * Return:
 *	Void.
 */
void lv_set_hold_line(code_line_t *line)
{
	g_hold_line = line;
}

/* Function: check_player_is_holding_line
 * ----------------------------------------------------------------------------
 * Verifies if the player is holding a line by analizing the pointer
 *
 * Arguments:
 *	Void
 *
 * Return:
 *	Void to determine if the player is holding a line;
 */
static bool check_player_is_holding_line()
{
	bool hold_line = (g_hold_line == NULL) ? false : true;
	return hold_line;
}

/* Function: get_hold_line
 * ----------------------------------------------------------------------------
 * Returns the poiinter of the line being hold by the player. 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Pointer of the line being hold by the player
 */
static code_line_t *get_hold_line()
{
	return g_hold_line;
}

/* Function: lv_init_lv_msgs
 * ----------------------------------------------------------------------------
 * Sets all the spaces of g_lv_msgs to true
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Void.
 */
static void init_lv_msgs()
{
	for (int i = 0; i < LV_MSGS_QTY; i++){
		g_lv_msg[i] = true;
	}
}

/* Function: lv_get_win_list_size
 * ----------------------------------------------------------------------------
 * Returns the size of the win list
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Size of the win list
 */
int lv_get_win_list_size()
{
	return List_count(g_win_list);
}

/* Function: lv_is_reg_selectable
 * ----------------------------------------------------------------------------
 * Returns a boolean with the value to check if registers are selectable
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Boolean indicating if the regs is editable
 */
bool lv_is_reg_selectable()
{
	return g_reg_selectable;
}


/* Function: set_reg_selectable
 * ----------------------------------------------------------------------------
 * Sets the reg global variable to be selectable
 *
 * Arguments:
 *	state: boolean to which the variable will be set
 *
 * Return:
 *	Void.
 */
static void set_reg_selectable(bool state)
{
	g_reg_selectable = state;
}

/* Function: lv_is_buf_selectable
 * ----------------------------------------------------------------------------
 * Returns a boolean with the value to check if buffer are selectable
 *
 * Arguments:
 *	state: boolean to which the variable will be set
 *
 * Return:
 *	Boolean indicating if the code is editable
 */
bool lv_is_buf_selectable()
{
	return g_buf_selectable;
}

/* Function: set_buf_selectable
 * ----------------------------------------------------------------------------
 * Sets the buf global variable 
 *
 * Arguments:
 *	void.
 *
 * Return:
 *	Void.
 */
static void set_buf_selectable(bool state)
{
	g_buf_selectable = state;
}

/* Function: lv_is_arrange_enabled
 * ----------------------------------------------------------------------------
 * Returns a boolean with the value to check if instruction arranging is enabled
 *
 * Arguments:
 *	state: boolean to which the variable will be set
 *
 * Return:
 *	Boolean indicating if the arrange is enabled
 */
bool lv_is_arrange_enabled()
{
	return g_arng_enabled;
}

/* Function: set_arrange_enabled
 * ----------------------------------------------------------------------------
 * Sets the arrange_enabled global variable
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Void.
 */
static void set_arrange_enabled(bool state)
{
	g_arng_enabled = state;
}

/* Function: lv_is_del_enabled
 * ----------------------------------------------------------------------------
 * Returns a boolean with the value to check if line deleting is enabled
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Boolean indicating if the delete is enabled
 */
bool lv_is_del_enabled()
{
	return g_del_enabled;
}

/* Function: set_del_enabled
 * ----------------------------------------------------------------------------
 * Sets the arrange_enabled global variable
 *
 * Arguments:
 *	state: state to which the global variable will be set
 *
 * Return:
 *	Void.
 */
static void set_del_enabled(bool state)
{
	g_del_enabled = state;
}

/* Function: lv_init_stage_code
 * ----------------------------------------------------------------------------
 * Generates the instructions of the levels that have starting code
 *
 * Arguments:
 *	level_id: id of the level that the instructions will be generated
 *
 * Return:
 *	Void.
 */
void lv_init_stage_code(int level_id)
{
	assert(level_id < LV_LEVEL_MAX 
		   && level_id > LV_LEVEL_MIN 
		   && "Invalid level value");

	if (level_id == LV_LEVEL_2){
			char i1[] = FL_L2_CODE_1;
			char i2[] = FL_L2_CODE_2;
			char i3[] = FL_L2_CODE_3;
			cw_add_saved_line(i1);
			cw_add_saved_line(i2);
			cw_add_saved_line(i3);
	} else if (level_id == LV_LEVEL_9){
			char i1[] = FL_L9_CODE_1;
			char i2[] = FL_L9_CODE_2;
			cw_add_saved_line(i1);
			cw_add_saved_line(i2);
	}
}


/* Function: lv_is_code_editable
 * ----------------------------------------------------------------------------
 * Returns a boolean with the value to check if the code could be editable
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Boolean indicating if the code is editable
 */
bool lv_is_code_editable()
{
	bool editable;
	int exception = g_code_editable_exception;
	
	if (exception == NO_EXCEPTION){
		editable = g_code_editable;
	} else if (iw_check_clicked_instruction() == true && 
													exception == INS_EXCEPTION){
		editable = true;	
	} else if (cw_is_code_clicked() == true) {
		code_line_t *line = cw_get_clicked_code();
		int pos = cw_get_code_line_pos_by_ptr(line);
		pos++;//instruction pos starts at zero;
		if (pos == exception){
			editable = true;	
		}
	} else if (exception == OP2_LAST){
		int code_size = cw_get_code_list_size();
		editable = cw_chk_click_code_op2(code_size);
	} else {
		editable = g_code_editable;
	}
	
	return editable;
}

/* Function: set_code_editable
 * ----------------------------------------------------------------------------
 * Sets the code global variable that indicates if code should be editable.
 * The editable does not allows selection from instruction nor code box.
 *
 * Arguments:
 *	state: true or false indicating if the code is editable
 *	exception: The exception of the line that will be editable
 *
 * Return:
 *	Void.
 */
static void set_code_editable(bool state, int exception)
{
	g_code_editable_exception = exception;
	g_code_editable = state;
}

/* Function: chk_ms_released_clear_msg
 * -----------------------------------------------------------------------------
 * Verifies if the mouse was released and sets the message variable to false.
 * It has an option for clearing the mouse state
 *
 * Arguments:
 * 	msg_id: The id of the message to be set on false
 *  rs_ms: Boolean to indicate if the mouse state is cleared
 *	
 * Return:
 *	Void.
 */
static void chk_ms_released_clear_msg(int message_id, bool reset_mouse)
{
	if (ms_left_released() == true){
		g_lv_msg[message_id] = false;
		if (reset_mouse == true){
			ms_reset_mouse_values();
		}
	}
}

/* Function: chk_ms_pressed_clear_msg
 * -----------------------------------------------------------------------------
 * Verifies if the mouse was pressed and sets the message variable to false.
 * It has an option for clearing the mouse state
 *
 * Arguments:
 * 	msg_id: The id of the message to be set on false
 *  rs_ms: Boolean to indicate if the mouse state is cleared
 *	
 * Return:
 *	Void.
 */
static void chk_ms_pressed_clear_msg(int message_id, bool reset_mouse)
{
	if (ms_left_pressed() == true){
		g_lv_msg[message_id] = false;
		if (reset_mouse == true){
			ms_reset_mouse_values();
		}
	}
}

/* Function: level_13_tutorial
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 10
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_13_tutorial()
{
	sb_set_step_btns_avail(true);
	im_set_imm_up_avail(true);
	draw_regs_arrow(check_display_reg_lv_arrow());
	draw_bufs_arrow(check_display_buf_arrow());
	draw_im_up_arrow(chk_display_imm_up_arrow());
	rg_draw_flag_boxes();

	int size = cw_get_code_list_size();
	 
	 if (g_lv_msg[MSG1] == true && size == 0){
		tx_text_box(TX_BIG_BOX, MSG1); //Welcome msg
		tx_bottom_msg(TX_BIG_BOX, TX_MSG_CLICKANY);
		chk_ms_pressed_clear_msg(MSG1, true);
	} else if (g_lv_msg[MSG2] == true && size == 0){
		tx_text_box(TX_UPPER_BOX, MSG2);//New instruction je and cmp
		tx_bottom_msg(TX_UPPER_BOX, TX_MSG_CLICKANY);
		ar_display_arrow(AR_INS_MINUS);
		ar_display_arrow(AR_INS);
		chk_ms_pressed_clear_msg(MSG2, true);
	} else if (g_lv_msg[MSG3] == true && size == 0){
		tx_text_box(TX_CENTER_BOX, MSG3);//Explains CMP
		tx_bottom_msg(TX_CENTER_BOX, TX_MSG_CLICKANY);
		ar_display_arrow(AR_INS_MINUS);
		chk_ms_pressed_clear_msg(MSG3, true);
	} else if (g_lv_msg[MSG4] == true && size == 0){
		tx_text_box(TX_UPPER_BOX, MSG4);//CMP modifies ZF
		tx_bottom_msg(TX_UPPER_BOX, TX_MSG_CLICKANY);
		ar_display_arrow(AR_ZF);
		chk_ms_pressed_clear_msg(MSG4, true);
	} else if (g_lv_msg[MSG5] == true && size == 0){
		tx_text_box(TX_LOWER_BOX, MSG5);//Different operands 0 in ZF
		tx_bottom_msg(TX_LOWER_BOX, TX_MSG_CLICKANY);
		ar_display_arrow(AR_ZF);
		chk_ms_pressed_clear_msg(MSG5, true);
	} else if (g_lv_msg[MSG6] == true && size == 0){
		tx_text_box(TX_INS_BOX, MSG6);// How JE works
		tx_bottom_msg(TX_INS_BOX, TX_MSG_CLICKANY);
		ar_display_arrow(AR_INS);
		chk_ms_pressed_clear_msg(MSG6, true);
	} else if (g_lv_msg[MSG7] == true && size == 0){
		tx_text_box(TX_CENTER_BOX, MSG7);// How JE works
		tx_bottom_msg(TX_CENTER_BOX, TX_MSG_CLICKANY);
		ar_display_arrow(AR_CHALLENGE);
		chk_ms_pressed_clear_msg(MSG7, true);
	} 
}
/* Function: level_12_tutorial
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 10
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_12_tutorial()
{
	sb_set_step_btns_avail(true);
	im_set_imm_up_avail(true);
	draw_regs_arrow(check_display_reg_lv_arrow());
	draw_bufs_arrow(check_display_buf_arrow());
	draw_im_up_arrow(chk_display_imm_up_arrow());

	int size = cw_get_code_list_size();

	if (g_lv_msg[MSG1] == true && size == 0){
		tx_text_box(TX_BIG_BOX, MSG1); //Welcome msg
		tx_bottom_msg(TX_BIG_BOX, TX_MSG_CLICKANY);
		chk_ms_pressed_clear_msg(MSG1, true);
	} 
}

/* Function: level_12_tutorial
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 10
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_11_tutorial()
{
	sb_set_step_btns_avail(true);
	im_set_imm_up_avail(true);
	draw_regs_arrow(check_display_reg_lv_arrow());
	draw_bufs_arrow(check_display_buf_arrow());
	draw_im_up_arrow(chk_display_imm_up_arrow());

	int size = cw_get_code_list_size();

	if (g_lv_msg[MSG1] == true && size == 0){
		tx_text_box(TX_BIG_BOX, MSG1); //Welcome msg
		tx_bottom_msg(TX_BIG_BOX, TX_MSG_CLICKANY);
		chk_ms_pressed_clear_msg(MSG1, true);
	} else if (g_lv_msg[MSG2] == true && size == 0){
		tx_text_box(TX_CENTER_BOX, MSG2); //New section on the screen
		tx_bottom_msg(TX_CENTER_BOX, TX_MSG_CLICKANY);
		ar_display_arrow(AR_IMM_UP);
		chk_ms_pressed_clear_msg(MSG2, true);
	} else if (g_lv_msg[MSG3] == true && size == 0){
		tx_text_box(TX_UPPER_RIGHT_BOX, MSG3); //Can select immediate values
		tx_bottom_msg(TX_UPPER_RIGHT_BOX, TX_MSG_CLICKANY);
		ar_display_arrow(AR_IMM_UP);
		chk_ms_pressed_clear_msg(MSG3, true);
	} else if (g_lv_msg[MSG4] == true && size == 0){
		tx_text_box(TX_CENTER_BOX, MSG4); //Imm can only be read
		tx_bottom_msg(TX_CENTER_BOX, TX_MSG_CLICKANY);
		ar_display_arrow(AR_IMM_UP);
		chk_ms_pressed_clear_msg(MSG4, true);
	} else if (g_lv_msg[MSG5] == true && size == 0){
		tx_text_box(TX_UPPER_RIGHT_BOX, MSG5); //Imm can only be read
		tx_bottom_msg(TX_UPPER_RIGHT_BOX, TX_MSG_CLICKANY);
		ar_display_arrow(AR_IMM_UP);
		chk_ms_pressed_clear_msg(MSG5, true);
	} 
}

/* Function: level_10_tutorial
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 10
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_10_tutorial()
{
	sb_set_step_btns_avail(true);
	im_set_imm_up_avail(false);
	draw_regs_arrow(check_display_reg_lv_arrow());
	draw_bufs_arrow(check_display_buf_arrow());

	int code_size = cw_get_code_list_size();

	if (g_lv_msg[MSG1] == true && code_size == 0){
		tx_text_box(TX_BIG_BOX, MSG1); //Welcome msg
		tx_bottom_msg(TX_BIG_BOX, TX_MSG_CLICKANY);
		chk_ms_pressed_clear_msg(MSG1, true);
	}
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
static void level_9_tutorial()
{
	sb_set_step_btns_avail(true);
	im_set_imm_up_avail(false);
	draw_regs_arrow(check_display_reg_lv_arrow());
	draw_bufs_arrow(check_display_buf_arrow());
	
	int code_size = cw_get_code_list_size();
	bool jmp_pick = false;
	bool label_pick = false;
	code_line_t *hld_line = get_hold_line();

	if (hld_line != NULL){
		jmp_pick = (hld_line->ins->id == JMP) ? true : false;
		label_pick = (hld_line->ins->id == LABEL) ? true : false;
	}

	if (g_lv_msg[MSG1] == true && code_size == 2){
		tx_text_box(TX_BIG_BOX, MSG1); //Welcome msg
		tx_bottom_msg(TX_BIG_BOX, TX_MSG_CLICKANY);
		chk_ms_pressed_clear_msg(MSG1, true);
	} else if (g_lv_msg[MSG2] == true && code_size == 2){
		tx_text_box(TX_UPPER_BOX, MSG2); //Read challenge description
		tx_bottom_msg(TX_UPPER_BOX, TX_MSG_CLICKANY);
		ar_display_arrow(AR_CHALLENGE);
		chk_ms_pressed_clear_msg(MSG2, true);
	} else if (g_lv_msg[MSG3] == true && code_size == 2){
		tx_text_box(TX_LOWER_BOX, MSG3);// New instruction
		tx_bottom_msg(TX_LOWER_BOX, TX_MSG_CLICKANY);
		ar_display_arrow(AR_INS);
		chk_ms_pressed_clear_msg(MSG3, true);
	} else if (code_size == 2 && jmp_pick == false){
		tx_text_box(TX_INS_BOX, MSG4);// Select and drag jmp
		ar_display_arrow(AR_INS);
	} else if (code_size == 2 && jmp_pick == true){
		tx_text_box(TX_CODE_BOX, MSG5); // Drop jmp in the code box
		ar_display_arrow(AR_DROP);
	} else if (code_size == 4 && label_pick == true){
		tx_text_box(TX_UPPER_BOX, MSG6); // Place Label in the first pos
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
static void level_8_tutorial()
{
	sb_set_step_btns_avail(true);
	im_set_imm_up_avail(false);
	draw_regs_arrow(check_display_reg_lv_arrow());
	draw_bufs_arrow(check_display_buf_arrow());

	int code_size = cw_get_code_list_size();
	static bool msg_descrip2 = true;

	if (g_lv_msg[MSG1] == true && code_size == 0){
		tx_text_box(TX_BIG_BOX, MSG1);//Welcome
		tx_bottom_msg(TX_BIG_BOX, TX_MSG_CLICKANY);
		chk_ms_pressed_clear_msg(MSG1, true);
	} else if (g_lv_msg[MSG2] == true && code_size == 0){
		tx_text_box(TX_UPPER_BOX, MSG2);//Can use register as operands
		tx_bottom_msg(TX_UPPER_BOX, TX_MSG_CLICKANY);
		chk_ms_pressed_clear_msg(MSG2, true);
	} else if (g_lv_msg[MSG3] == true && code_size == 0){
		tx_text_box(TX_LOWER_BOX, MSG3);//Usage example
		tx_bottom_msg(TX_LOWER_BOX, TX_MSG_CLICKANY);
		chk_ms_pressed_clear_msg(MSG3, true);
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
static void level_7_tutorial()
{
	sb_set_step_btns_avail(true);
	im_set_imm_up_avail(false);
	draw_regs_arrow(check_display_reg_lv_arrow());
	draw_bufs_arrow(check_display_buf_arrow());

	int code_size = cw_get_code_list_size();

	if (g_lv_msg[MSG1] == true && code_size == 0){
		tx_text_box(TX_BIG_BOX, MSG1);//Welcome
		tx_bottom_msg(TX_BIG_BOX, TX_MSG_CLICKANY);
		chk_ms_pressed_clear_msg(MSG1, true);
	} else if (g_lv_msg[MSG2] == true && code_size == 0){
		tx_text_box(TX_UPPER_BOX, MSG2);//Both operands can be same register
		tx_bottom_msg(TX_UPPER_BOX, TX_MSG_CLICKANY);
		chk_ms_pressed_clear_msg(MSG2, true);
	} else if (g_lv_msg[MSG3] == true && code_size == 0){
		tx_text_box(TX_LOWER_BOX, MSG3);//Example of usage
		tx_bottom_msg(TX_LOWER_BOX, TX_MSG_CLICKANY);
		chk_ms_pressed_clear_msg(MSG3, true);
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
static void level_6_tutorial()
{
	sb_set_step_btns_avail(true);
	im_set_imm_up_avail(false);
	draw_regs_arrow(check_display_reg_lv_arrow());
	draw_bufs_arrow(check_display_buf_arrow());

	bool hold = check_player_is_holding_line();
	int code_size = cw_get_code_list_size();

	if (g_lv_msg[MSG1] == true && code_size == 0){
		tx_text_box(TX_BIG_BOX, MSG1);//Welcome 
		tx_bottom_msg(TX_BIG_BOX, TX_MSG_CLICKANY);
		chk_ms_pressed_clear_msg(MSG1, true);
	} else if (g_lv_msg[MSG2] == true && code_size == 0){
		tx_text_box(TX_UPPER_BOX, MSG2);//New instruction add
		tx_bottom_msg(TX_UPPER_BOX, TX_MSG_CLICKANY);
		ar_display_arrow(AR_INS);
		chk_ms_pressed_clear_msg(MSG2, true);
	} else if (g_lv_msg[MSG3] == true && code_size == 0){
		tx_text_box(TX_LOWER_BOX, MSG3); //Usage
		tx_bottom_msg(TX_LOWER_BOX, TX_MSG_CLICKANY);
		ar_display_arrow(AR_INS);
		chk_ms_pressed_clear_msg(MSG3, true);
	} else if (code_size == 0 && hold == false){
		tx_text_box(TX_INS_BOX, MSG4); //Solve using add and mov
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
static void level_5_tutorial()
{
	sb_set_step_btns_avail(true);
	im_set_imm_up_avail(false);
	draw_regs_arrow(check_display_reg_lv_arrow());
	draw_bufs_arrow(check_display_buf_arrow());
	bool miss_op = cw_is_operand_pending();

	int size = cw_get_code_list_size();

	if (g_lv_msg[MSG1] && size == 0){
		tx_text_box(TX_BIG_BOX, MSG1); //Welcome
		tx_bottom_msg(TX_BIG_BOX, TX_MSG_CLICKANY);
		chk_ms_pressed_clear_msg(MSG1, true);
	} else if (g_lv_msg[MSG2] == true && size == 0){
		tx_text_box(TX_UPPER_BOX, MSG2); //More registers available
		tx_bottom_msg(TX_UPPER_BOX, TX_MSG_CLICKANY);
		ar_display_arrow(AR_REG);
		chk_ms_pressed_clear_msg(MSG2, true);
	} else if (size == 1 && miss_op == false){
		tx_text_box(TX_CENTER_BOX, MSG3); // All operands are shown
		ar_display_arrow(AR_STEP);
		ar_display_arrow(AR_FAST);
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
static void level_4_tutorial()
{
	sb_set_step_btns_avail(false);
	im_set_imm_up_avail(false);
	draw_regs_arrow(check_display_reg_lv_arrow());
	draw_bufs_arrow(check_display_buf_arrow());

	int code_size = cw_get_code_list_size();

	if (g_lv_msg[MSG1] == true && code_size == 0){
		tx_text_box(TX_BIG_BOX, MSG1); //Welcome
		tx_bottom_msg(TX_BIG_BOX, TX_MSG_CLICKANY);
		chk_ms_pressed_clear_msg(MSG1, true);
	} else if (g_lv_msg[MSG2] == true && code_size == 0){
		tx_text_box(TX_UPPER_BOX, MSG2);//Solve the challenge by yourself
		tx_bottom_msg(TX_UPPER_BOX, TX_MSG_CLICKANY);
		chk_ms_pressed_clear_msg(MSG2, true);
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
static void level_3_tutorial()
{
	sb_set_step_btns_avail(false);
	im_set_imm_up_avail(false);
	draw_regs_arrow(check_display_reg_lv_arrow());
	draw_bufs_arrow(check_display_buf_arrow());

	int size = cw_get_code_list_size();
	bool hold = check_player_is_holding_line();
	bool miss_op1 = cw_check_code_pending_op1();
	bool miss_op2 = cw_check_code_pending_op2();
	bool miss_op = cw_is_operand_pending();
	bool sorted = cw_check_code_sorted();

	if (g_lv_msg[MSG1] == true && size == 0){
		tx_text_box(TX_BIG_BOX, MSG1); //Welcome
		tx_bottom_msg(TX_BIG_BOX, TX_MSG_CLICKANY);
		set_code_editable(false, NO_EXCEPTION);
		chk_ms_pressed_clear_msg(MSG1, true);
	} else if (g_lv_msg[MSG2] == true && size == 0){
		tx_text_box(TX_UPPER_BOX, MSG2);// IB can be read only once
		tx_bottom_msg(TX_UPPER_BOX, TX_MSG_CLICKANY);
		ar_display_arrow(AR_IB);
		chk_ms_pressed_clear_msg(MSG2, true);
	} else if (g_lv_msg[MSG3] == true && size == 0){
		tx_text_box(TX_LOWER_BOX, MSG3); //Reg can be read multiple times
		tx_bottom_msg(TX_LOWER_BOX, TX_MSG_CLICKANY);
		draw_regs_arrow(true);
		chk_ms_pressed_clear_msg(MSG3, true);
	} else if (size == 0 && hold == false){
		tx_text_box(TX_INS_BOX, MSG4);// Select and drag mov
		ar_display_arrow(AR_INS);
		set_code_editable(true, NO_EXCEPTION);
	} else if (size == 0 && hold == true){
		tx_text_box(TX_CODE_BOX, MSG5); // Drop in code box
		ar_display_arrow(AR_DROP);
	} else if (size == 1 && hold == true && miss_op == true){
		tx_text_box(TX_CODE_BOX, MSG5); // Drop in code box
		ar_display_arrow(AR_DROP);
	} else if (size == 1 
			   && sorted == true 
			   && hold == false 
			   && g_lv_msg[MSG6] == true){
		tx_text_box(TX_LOWER_BOX, MSG6); // All operands are shown
		tx_bottom_msg(TX_LOWER_BOX, TX_MSG_CLICKANY);
		chk_ms_released_clear_msg(MSG6, true);
	} else if (size == 1 && miss_op1 == true && sorted == true){
		tx_text_box(TX_CODE_BOX, MSG7); //Sel rax
		set_buf_selectable(false);
	} else if (size == 1 
			   && miss_op2 == true 
			   && sorted == true
			   && g_lv_msg[MSG8] == true){
		tx_text_box(TX_CENTER_BOX, MSG8); //Valid op combinations
		tx_bottom_msg(TX_CENTER_BOX, TX_MSG_CLICKANY);
		chk_ms_released_clear_msg(MSG8, true);
	} else if (size == 1 && miss_op2 == true && sorted == true){
		tx_text_box(TX_UPPER_BOX, MSG9); //Select IB
		set_buf_selectable(true);
		set_reg_selectable(false);
	} else if (size == 1 && miss_op == false && hold == false){
		tx_text_box(TX_INS_BOX, MSG10); //Select second instruction
		ar_display_arrow(AR_INS);
		set_reg_selectable(true);
	} else if (size == 1 && miss_op == false & hold == true){
		tx_text_box(TX_CODE_BOX, MSG5);//Drop in code box
		ar_display_arrow(AR_DROP);
	} else if (size == 2 && miss_op1 == true){
		tx_text_box(TX_CODE_BOX, MSG11);//Use mov [ob], rax several times
		set_reg_selectable(false);
		set_buf_selectable(true);
		ar_init_arrow(AR_CODE);
	} else if (size == 2 && miss_op1 == false && miss_op2 == true){
		tx_text_box(TX_CODE_BOX, MSG11);//Use mov [ob], rax several times
		set_buf_selectable(false);
		set_reg_selectable(true);
		ar_init_arrow(AR_CODE);
	} else if (size == 2 && miss_op == false){
		tx_text_box(TX_CODE_BOX, MSG12); //Right click a complete instruction
		set_reg_selectable(true);
		ar_display_arrow(AR_CODE);
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
static void level_2_tutorial()
{
	sb_set_step_btns_avail(false);
	im_set_imm_up_avail(false);
	int size = cw_get_code_list_size();
	bool hold = check_player_is_holding_line();
 	bool play = get_play_state();
	int limit = lv_get_level_instructions_limit();
	const int pos_one = 0;
	const int pos_two = 1;
	const int two_instructions = 2;
	bool change_op = false;
	bool mov_instruction = false;
	bool press_play = false;
	bool win = lv_check_if_win();
	code_line_t *i1= NULL;
	code_line_t *i2 = NULL;

	if (size >= two_instructions && hold == false){
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
	if (g_lv_msg[MSG1] == true && size == 3){
		set_code_editable(false, NO_EXCEPTION);
		set_arrange_enabled(false);
		tx_text_box(TX_BIG_BOX, MSG1); //Welcome
		tx_bottom_msg(TX_BIG_BOX, TX_MSG_CLICKANY);
		chk_ms_pressed_clear_msg(MSG1, true);
	} else if (g_lv_msg[MSG2] == true && size == 3){
		set_code_editable(false, NO_EXCEPTION);
		tx_text_box(TX_UPPER_BOX, MSG2); //Rearrange
		tx_bottom_msg(TX_UPPER_BOX, TX_MSG_CLICKANY);
		chk_ms_pressed_clear_msg(MSG2, true);
	} else if (size > limit && hold == false){
		set_code_editable(false, size);
		tx_text_box(TX_CODE_BOX, MSG3); //Select last instruction
		ar_display_arrow(AR_CODE);
	} else if (size > limit && hold == true){
		tx_text_box(TX_CODE_BOX, MSG4); //Delete the instruction
		ar_display_arrow(AR_DEL);
	} else if (change_op == true && hold == false){
		set_code_editable(false, OP2_LAST);
		if (i2->state != CHANGING_OP2){
			tx_text_box(TX_CODE_BOX, MSG5); // Select operand
			ar_display_arrow(AR_OP2);
		} else if (i2->state == CHANGING_OP2){
			tx_text_box(TX_UPPER_BOX, MSG6); // Select IB
			ar_display_arrow(AR_IB);
		}
	} else if (mov_instruction == true){
		set_code_editable(false, size);
		set_del_enabled(false);
		set_arrange_enabled(true);
		tx_text_box(TX_CODE_BOX, MSG7); //Mov ins to first pos
		ar_display_arrow(AR_CODE);
	} else if (press_play == true && play == false && win == false){
		set_code_editable(false, NO_EXCEPTION);
		tx_text_box(TX_CODE_BOX, MSG8); //Press play
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
static void level_1_tutorial()
{
	sb_set_step_btns_avail(false);
	im_set_imm_up_avail(false);
	draw_regs_arrow(false);
	bool flag = get_op_flag();
	bool play = get_play_state();
	bool hold = check_player_is_holding_line();
	int size = cw_get_code_list_size();
	bool sorted = cw_check_code_sorted();
	bool miss_op = cw_is_operand_pending();
	bool miss_op1 = cw_check_code_pending_op1();
	bool win = lv_check_if_win();

	if (g_lv_msg[MSG1] == true && size == 0){
		tx_text_box(TX_BIG_BOX, MSG1); //Welcome message
		tx_bottom_msg(TX_BIG_BOX, TX_MSG_CLICKANY);
		chk_ms_pressed_clear_msg(MSG1, true);
	} else if (g_lv_msg[MSG2] == true && size == 0){
		tx_text_box(TX_UPPER_BOX, MSG2); //Read challenge
		tx_bottom_msg(TX_UPPER_BOX, TX_MSG_CLICKANY);
		ar_display_arrow(AR_CHALLENGE);
		chk_ms_pressed_clear_msg(MSG2, true);
	} else if (g_lv_msg[MSG3] == true && size == 0){
		tx_text_box(TX_UPPER_BOX, MSG3); //Move from the Input Buffer
		tx_bottom_msg(TX_UPPER_BOX, TX_MSG_CLICKANY);
		ar_display_arrow(AR_IB);
		chk_ms_pressed_clear_msg(MSG3, true);
	} else if (g_lv_msg[MSG4] == true && size == 0){
		tx_text_box(TX_LOWER_BOX, MSG4); //To the Output Buffer
		tx_bottom_msg(TX_LOWER_BOX, TX_MSG_CLICKANY);
		ar_display_arrow(AR_OB);
		chk_ms_pressed_clear_msg(MSG4, true);
	} else if (g_lv_msg[MSG5] == true && size == 0){
		tx_text_box(TX_CENTER_BOX, MSG5);//To register first
		tx_bottom_msg(TX_CENTER_BOX, TX_MSG_CLICKANY);
		draw_regs_arrow(true);
		chk_ms_pressed_clear_msg(MSG5, true);
	} else if (size == 0 && hold == false){
		tx_text_box(TX_INS_BOX, MSG6);//Select and drag instruction
		ar_display_arrow(AR_INS);
	} else if (size == 0 && hold == true){
		tx_text_box(TX_CODE_BOX, MSG7); // Drop ins in code box
		ar_display_arrow(AR_DROP);
	} else if (size == 1 && hold == true && miss_op == true){
		tx_text_box(TX_CODE_BOX, MSG7); // Drop ins in code box
		ar_display_arrow(AR_DROP);
	} else if (size == 1 && sorted == true && miss_op1 == true){
		tx_text_box(TX_CENTER_BOX, MSG8);//Select rax
		set_code_editable(false, NO_EXCEPTION);
		set_buf_selectable(false);
		set_reg_selectable(true);
		draw_regs_arrow(true);
	} else if(size == 1 && sorted == true && miss_op == true){
		tx_text_box(TX_UPPER_BOX, MSG9);//Select input buffer
		set_buf_selectable(true);
		set_reg_selectable(false);
		draw_regs_arrow(false);
		ar_display_arrow(AR_IB);
	} else if(size == 1 && miss_op == false && g_lv_msg[MSG10] == true){
		set_code_editable(false, INS_EXCEPTION);
		tx_text_box(TX_CODE_BOX, MSG10);// Press play button
		ar_display_arrow(AR_PLAY);
		set_arrange_enabled(false);
		if (play == true){
			g_lv_msg[MSG10] = false;
		}
	} else if (flag != 0){
		tx_text_box(TX_CENTER_BOX, MSG11); //ERROR
		tx_bottom_msg(TX_CENTER_BOX, TX_MSG_PRESSBACK);
		ar_display_arrow(AR_ERROR);
	} else if(size == 1 && hold == false && play == false 
												   && g_lv_msg[MSG10] == false){
		tx_text_box(TX_INS_BOX, MSG12);	 //Select another mov instruction
		ar_display_arrow(AR_INS);
	} else if(size == 1 && hold == true && miss_op == false){
		tx_text_box(TX_CODE_BOX, MSG13); // Drop below instruction
		ar_display_arrow(AR_DROP);
	} else if(size == 2 && hold == true){
		tx_text_box(TX_CODE_BOX, MSG13); // Drop below instruction
		ar_display_arrow(AR_DROP);
	} else if (size == 2 && sorted == true && miss_op1 == true){
		tx_text_box(TX_CODE_BOX, MSG14);//Select OB	
		set_code_editable(false, NO_EXCEPTION);
		ar_display_arrow(AR_OB);
	} else if(size == 2 && sorted == true && miss_op == true){
		set_reg_selectable(true);
		tx_text_box(TX_CODE_BOX, MSG15); //Select rax
		draw_regs_arrow(true);
	} else if(size == 2 && miss_op == false && play == false && win == false){
		set_code_editable(false, NO_EXCEPTION);
		tx_text_box(TX_CODE_BOX, MSG16);	//Press play
		ar_display_arrow(AR_PLAY);
	} else if (lv_check_if_win() == true){
		tx_text_box(TX_CENTER_BOX, MSG17);//Congrats 
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
									   cw_is_operand_pending() == true){
		code_line_t *l = cw_get_code_line_pending_operand();
		if (l->ins->id != JMP && l->ins->id !=LABEL){
			operand_t o;
			o.id = IB;
			if (cl_is_op_compatible(&o, l) == true){
				display_ar = IB;
			} 
			o.id = OB;
			if (cl_is_op_compatible(&o, l) == true){
				display_ar = OB;
			}
		}
	}
	return display_ar;
}

/* Function: draw_bufs_arrow
 * -----------------------------------------------------------------------------
 * Function that verifies according to the flags if the bufs arrows must
 * be drawn
 * 
 * Arguments:
 * 	buf_id: true displays arros, false does not show arrows.
 *
 * Return:
 *	Void.
 */
static void draw_bufs_arrow(int buf_id) 
{
	switch(buf_id){
		case IB:
			ar_display_arrow(AR_IB);
			break;
		case OB:
			ar_display_arrow(AR_OB);
			break;
		default:
			break;
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
 *	true if the pointing arrow to the registers should be displayed
 */
static bool check_display_reg_lv_arrow() 
{
	bool display_ar = false;
	if (cw_check_code_sorted() == true && cw_is_operand_pending() == true){
		code_line_t *l = cw_get_code_line_pending_operand();
		if (l->ins->id != JMP && l->ins->id != LABEL){
			operand_t o;
			o.id = RAX;
			display_ar = cl_is_op_compatible(&o, l);
		}
	}
	return display_ar;
}

/* Function: chk_display_imm_up_arrow
 * -----------------------------------------------------------------------------
 * Analize the state of the operands to determine if the immediate arrow should
 * be displayed
 * 
 * Arguments:
 * 	None.
 *
 * Return:
 *	true if the pointing arrow to the registers should be displayed
 */
static bool chk_display_imm_up_arrow() 
{
	bool display_ar = false;
	if (cw_check_code_sorted() == true && cw_is_operand_pending() == true){
		code_line_t *l = cw_get_code_line_pending_operand();
		if (l->ins->id != JMP && l->ins->id != LABEL){
			operand_t o;
			o.id = IMMUP0 ;
			display_ar = cl_is_op_compatible(&o, l);
		}
	}
	return display_ar;
}

/* Function: draw_regs_arrow
 * -----------------------------------------------------------------------------
 * Function that verifies according to the flags if the register arrows must
 * be drawn
 * 
 * Arguments:
 * 	show_arrow: true displays arros, false does not show arrows.
 *
 * Return:
 *	Void.
 */
static void draw_regs_arrow(bool show_arrows) 
{
	if (show_arrows == true){
		ar_display_arrow(AR_REG);
	}
}

/* Function: draw_im_up_arrow
 * -----------------------------------------------------------------------------
 * Function that verifies according to the flags if the register arrows must
 * be drawn
 * 
 * Arguments:
 * 	show_arrow: true displays arros, false does not show arrows.
 *
 * Return:
 *	Void.
 */
static void draw_im_up_arrow(bool show_arrows) 
{
	if (show_arrows == true){
		ar_display_arrow(AR_IMM_UP);
	}
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
	return g_level_instructions_limit;
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
	g_level_instructions_limit = limit;
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
void lv_set_level_win_condition_text(char *win_condition)
{
	strcpy(level_win_condition, win_condition);
}

/* Function: set_win_condition
 * -----------------------------------------------------------------------------
 * This function creates the win condition according to what is in the levels
 * file. 
 * Nomeclature of the win conditions
 * WIN1: win1_move_input_to_output(rep, mul, rev) 
 * WIN2: win2_add_inputs_in_groups(group_size)
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
	char *delim = ax_char_space;
	char *win_cond;
	
	win_cond =  strtok_r(win_condition_cpy, delim, &saveptr1);
	if (strstr(win_cond, STR_WIN1) != NULL){
		char *rep_text = strtok_r(NULL, delim, &saveptr1);
		int rep = atoi(rep_text);
		char *mul_text = strtok_r(NULL, delim, &saveptr1);
		int mul = atoi(mul_text);
		char *sum_text = strtok_r(NULL, delim, &saveptr1);
		int sum = atoi(sum_text);
		char *reversed = strtok_r(NULL, delim, &saveptr1);
		bool rev;
		if (strstr(reversed, "true") != NULL){
			rev = true;
		} else if (strstr(reversed, "false") != NULL){
			rev = false;
		}
		win1_move_input_to_output(rep, mul, sum, rev);
	} else if (strstr(win_cond, STR_WIN2) != NULL){
		char *group_size_text = strtok_r(NULL, delim, &saveptr1);
		int group_size = atoi(group_size_text);
		char *bet_text = strtok_r(NULL, delim, &saveptr1);
		bool between;
		if (strstr(bet_text, "true") != NULL){
			between = true;
		} else if (strstr(bet_text, "false") != NULL){
			between = false;
		}
		char *in_val_text = strtok_r(NULL, delim, &saveptr1);
		int in_val = atoi(in_val_text);
		win2_add_inputs_in_groups(group_size, between, in_val);
	} else if (strstr(win_cond, STR_WIN3) != NULL){
		char *stop_text = strtok_r(NULL, delim, &saveptr1);
		int stop = atoi(stop_text);
		win3_move_input_to_output_stop(stop);
	} else if (strstr(win_cond, STR_WIN4) != NULL){
		char *element_text = strtok_r(NULL, delim, &saveptr1);
		int element = atoi(element_text);
		char *stop_text = strtok_r(NULL, delim, &saveptr1);
		int stop = atoi(stop_text);
		win4_count_values_till_stop(element, stop);
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
	List_clear_destroy(g_win_list);
	g_win_list = NULL;
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
	assert(g_win_list == NULL && "The win list is not NULL");
	g_win_list = List_create();
	check_mem(g_win_list);

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
	List_clear_destroy(g_win_list);
	g_win_list = NULL;
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
	return g_win_list;
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
 *	sum: value that will be added to the output
 *  reversed: if the order of the inputs is reversed
 *
 * Return:
 *	Void.
 */
static void win1_move_input_to_output(int rep, int mul, int sum, bool rev)
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
			new_win->value = mul*cur_input->value + sum;
			new_win->type = cur_input->type;
			if (rev == false){
				List_push(win_list, new_win);
			} else if (rev == true){
				List_unshift(win_list, new_win);
			}
		}
	}
}

/* Function: win3_move_input_to_output_stop
 *------------------------------------------------------------------------------
 * Generates a win condition that is achieved by moving the elements from the
 * input buffer to the output buffer up to a stop element
 *
 * Arguments:
 *	stop: stop element that will be used to stop the movement
 *
 * Return:
 *	Void.
 */
static void win3_move_input_to_output_stop(int stop)
{
	List *input_list = get_input_list();
	List *win_list = get_win_list();

	int input_list_size = List_count(input_list);
	int win_list_size = List_count(win_list);

	assert(input_list_size > 0 && "The size of the input list is incorrect");
	assert(win_list_size == 0 && "The win list has elements");

	LIST_FOREACH(input_list, first, next, cur){
		value_box_t *cur_input = cur->value;
		value_box_t *new_win; 
		if (cur_input->value != stop){
			new_win = malloc(sizeof(value_box_t));
			new_win->value = cur_input->value;
			new_win->type = cur_input->type;
			List_push(win_list, new_win);
		} else {
			break;
		}
	}
}

/* Function: win4_count_values_till_stop
 *------------------------------------------------------------------------------
 * Generates a win condition that is achieved by counting the number of 
 * appearances of an element until a stop  condition
 *
 * Arguments:
 *	element: element that will be counted
 *	stop: stop element that will be used to stop the movement
 *
 * Return:
 *	Void.
 */
static void win4_count_values_till_stop(int element, int stop)
{
	List *input_list = get_input_list();
	List *win_list = get_win_list();

	int input_list_size = List_count(input_list);
	int win_list_size = List_count(win_list);

	assert(input_list_size > 0 && "The size of the input list is incorrect");
	assert(win_list_size == 0 && "The win list has elements");

	value_box_t *new_win = malloc(sizeof(value_box_t));
	LIST_FOREACH(input_list, first, next, cur){
		value_box_t *cur_input = cur->value;
		if (cur_input->value == element){
			new_win->value++;
		} else if (cur_input->value == stop){
			new_win->type = cur_input->type;
			List_push(win_list, new_win);
			break;
		}
	}
}

/* Function: win2_add_inputs_in_groups
 *------------------------------------------------------------------------------
 * The solution of the challenge will be achieved is the player adds the inputs
 * in groups
 *
 * Arguments:
 *	grp_size: The size of the input group tha will be added
 *  between: If active the function inserts a value between each output
 *	in_value: Value that will be inserted between each output
 *
 * Return:
 *	Void.
 */
static void win2_add_inputs_in_groups(int grp_size, bool between, int in_val)
{
	List *input_list = get_input_list();
	List *win_list = get_win_list();

	assert(input_list != NULL && "Input list pointer is NULL");
	assert(win_list != NULL && "Win list pointer is NULL");

	int input_list_size = List_count(input_list);
	assert(input_list_size % grp_size == 0 && 
					"The input size must be a multiple of group size");
	
	int win_list_size = List_count(win_list);
	assert(input_list_size > 0 && "The size of the input list is incorrect");
	assert(win_list_size == 0 && "The win list has elements");

	int res = 1;
	int array_index = 0;
	int *values = malloc(sizeof(int)*input_list_size);
			
	LIST_FOREACH(input_list, first, next, cur){
		value_box_t *cur_input = cur->value;
		values[array_index] = cur_input->value;
		if (res % grp_size == 0){
			value_box_t *new_win = malloc(sizeof(value_box_t));
			int val = 0;
			for (int i = 0; i < grp_size; i++){
				val += values[array_index - i];
			}
			new_win->value = val;
			new_win->type = cur_input->type;
			List_push(win_list, new_win);
			if (between == true && array_index != (input_list_size - 1)){
				value_box_t *inserted = malloc(sizeof(value_box_t));
				inserted->value = in_val;
				inserted->type = cur_input->type;
				List_push(win_list, inserted);
			}
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

	if (output_list_size > win_list_size){
		return false;
	}
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

/* Function: lv_init_level_assets
 *------------------------------------------------------------------------------
 * Initializes the assets and textures required for a level
 *
 * Arguments:
 *	level: The level which the assets will be initialized.
 *
 * Return:
 *	Void.
 */
void lv_init_level_assets(int level)
{
	assert(level < LV_LEVEL_MAX && level > LV_LEVEL_MIN && 
			          								"Invalid level value");
	set_arrange_enabled(true);
	set_del_enabled(true);
	set_code_editable(true, NO_EXCEPTION);
	set_buf_selectable(true);
	set_reg_selectable(true);
	init_lv_msgs();
}

/* Function: lv_upd_level_assets
 *------------------------------------------------------------------------------
 * Updates the selected game assets when a change in game state requires it.
 *
 * Arguments:
 *	level: The speficic level that is goin to have the drawing.
 *
 * Return:
 *	Void.
 */
void lv_upd_level_assets(int level)
{
	assert(level < LV_LEVEL_MAX && level > LV_LEVEL_MIN && 
			          								"Invalid level value");
	
	switch(level){
		case LV_LEVEL_2:
			ar_init_arrow(AR_CODE);
			break;
		default:
			break;
	}
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
void lv_level_drawings(int level)
{
	assert(level < LV_LEVEL_MAX && level > LV_LEVEL_MIN && 
			          								"Invalid level value");
	
	switch(level){
		case LV_LEVEL_1:
			level_1_tutorial();
			break;

		case LV_LEVEL_2:
			level_2_tutorial();
			break;
	
		case LV_LEVEL_3:
			level_3_tutorial();
			break;

		case LV_LEVEL_4:
			level_4_tutorial();
			break;
	
		case LV_LEVEL_5:
			level_5_tutorial();
			break;
	
		case LV_LEVEL_6:
			level_6_tutorial();
			break;
		
		case LV_LEVEL_7:
			level_7_tutorial();
			break;

		case LV_LEVEL_8:
			level_8_tutorial();
			break;

		case LV_LEVEL_9:
			level_9_tutorial();
			break;

		case LV_LEVEL_10:
			level_10_tutorial();
			break;

		case LV_LEVEL_11:
			level_11_tutorial();
			break;
		
		case LV_LEVEL_12:
			level_12_tutorial();
			break;
		
		case LV_LEVEL_13:
			level_13_tutorial();
			break;

		default:
			rg_draw_flag_boxes();
			sb_set_step_btns_avail(true);
		 	im_set_imm_up_avail(true);
			draw_bufs_arrow(check_display_buf_arrow());
			draw_im_up_arrow(chk_display_imm_up_arrow());
			draw_regs_arrow(check_display_reg_lv_arrow());
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
