#include <assert.h>
#include <stdbool.h>
#include "levels_lv.h"

#include "registers_rg.h"
#include "instruction_window_iw.h"
#include "code_window_cw.h"
#include "code_line_cl.h"
#include "stage_buttons_sb.h"
#include "text_tx.h"
#include "arrow_ar.h"
#include "immediates_im.h"
#include "file_fl.h"
#include "game_mechanics_mc.h"
#include "mouse_ms.h"
#include "tutorial_tr.h"
#include "gameplay/code_state_cs.h"


#define LV_MSGS_QTY 15

// Exceptions of the selection of the code
#define NO_EXCEPTION -1
#define INS_EXCEPTION -2
#define OP1_LAST -3
#define OP2_LAST -4

const Uint32 LV_SECTOR_1_START = 8;
const Uint32 LV_SECTOR_2_START = 16;
const Uint32 LV_SECTOR_3_START = 24;
const Uint32 LV_SECTOR_4_START = 32;
const Uint32 LV_SECTOR_LV_QTY = 8;

static bool g_code_editable;
static int g_code_editable_exception;
static bool g_buf_selectable;
static bool g_reg_selectable;
static bool g_arng_enabled;
static bool g_del_enabled;
static int g_level_instructions_limit;

static bool g_lv_msg[LV_MSGS_QTY];

static lv_rules_t g_lv_rules = {0};

static void level_0();
static void level_1();
static void level_2();
static void level_3();
static void level_4();
static void level_5();
static void level_6();
static void level_7();
static void level_8();
static void level_9();
static void level_10();
static void level_11();
static void level_12();
static void level_13();
static void level_14();
static void level_15();
static void level_16();
static void level_17();
static bool check_display_reg_lv_arrow();
static int check_display_buf_arrow();
static bool chk_display_imm_up_arrow();
static void draw_regs_arrow(bool show_arrows);
static void draw_bufs_arrow(int buf_id);
static void draw_im_up_arrow(bool show_arrows);
static void set_code_editable(bool state, int exception);
static void set_buf_selectable(bool state);
static void set_reg_selectable(bool state);
static void set_arrange_enabled(bool state);
static void init_lv_msgs();
static void chk_ms_pressed_clear_msg(int message_id, bool reset_mouse);
static void chk_ms_rel_clear_msg(int message_id, bool reset_mouse);
static int get_level_state();

/* Function: get_level_state
 * ----------------------------------------------------------------------------
 * Get the state of the state machine of the level that is being played
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Identifier of the state of the level that is being played
 */
static int get_level_state()
{
	return 0;	
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
	assert(level_id < LV_LEVEL_MAX && level_id >= 0 && "Invalid level value");

	if (level_id == LV_LEVEL_1){
			char i1[] = FL_L1_CODE_1;
			char i2[] = FL_L1_CODE_2;
			char i3[] = FL_L1_CODE_3;
			cw_add_saved_line(i1);
			cw_add_saved_line(i2);
			cw_add_saved_line(i3);
			ar_init_arrow(AR_CODE);
	} else if (level_id == LV_LEVEL_8){
			char i1[] = FL_L8_CODE_1;
			char i2[] = FL_L8_CODE_2;
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
	} else if (iw_chk_click_ins() == true && 
													exception == INS_EXCEPTION){
		editable = true;	
	} else if (cw_chk_click_code() == true) {
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

/* Function: chk_ms_rel_clear_msg
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
static void chk_ms_rel_clear_msg(int message_id, bool reset_mouse)
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
	if (sb_chk_hov_rst_ret_btns() == false){
		if (ms_left_pressed() == true){
			g_lv_msg[message_id] = false;
			if (reset_mouse == true){
				ms_reset_mouse_values();
			}
		}
	}
}

/* Function: level_16
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 10
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_16()
{
 if (sb_chk_rst_esc_menu_active() == true){
		return;
	}
}

/* Function: level_15
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 10
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_15()
{
	draw_regs_arrow(check_display_reg_lv_arrow());
	draw_bufs_arrow(check_display_buf_arrow());
	draw_im_up_arrow(chk_display_imm_up_arrow());
	rg_draw_flag_boxes();

	int size = cw_get_code_list_size();
	 
	if (size == 0 && tr_is_active("welcome")) tr_render_step("welcome");
}
/* Function: level_14
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 10
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_14()
{
	draw_regs_arrow(check_display_reg_lv_arrow());
	draw_bufs_arrow(check_display_buf_arrow());
	draw_im_up_arrow(chk_display_imm_up_arrow());
	rg_draw_flag_boxes();

	int size = cw_get_code_list_size();
	 
	if (size == 0 && tr_is_active("welcome")) tr_render_step("welcome");
	else if (size == 0 && tr_is_active("introduce_jump_not_equal")) tr_render_step("introduce_jump_not_equal");
	else if (size == 0 && tr_is_active("explain_jump_not_equal")) tr_render_step("explain_jump_not_equal");
	else if (size == 0 && tr_is_active("explain_zero_flag_clear")) tr_render_step("explain_zero_flag_clear");
	else if (size == 0 && tr_is_active("explain_zero_flag_set")) tr_render_step("explain_zero_flag_set");

}

/* Function: level_13
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 10
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_13()
{
	draw_regs_arrow(check_display_reg_lv_arrow());
	draw_bufs_arrow(check_display_buf_arrow());
	draw_im_up_arrow(chk_display_imm_up_arrow());
	rg_draw_flag_boxes();

	int size = cw_get_code_list_size();
	 
	if (size == 0 && tr_is_active("welcome")) tr_render_step("welcome");
	else if (size == 0 && tr_is_active("introduce_compare_and_equal")) tr_render_step("introduce_compare_and_equal");
	else if (size == 0 && tr_is_active("explain_compare")) tr_render_step("explain_compare");
	else if (size == 0 && tr_is_active("explain_zero_flag_set")) tr_render_step("explain_zero_flag_set");
	else if (size == 0 && tr_is_active("explain_zero_flag_clear")) tr_render_step("explain_zero_flag_clear");
	else if (size == 0 && tr_is_active("explain_jump_equal")) tr_render_step("explain_jump_equal");
	else if (size == 0 && tr_is_active("complete_compare_branch_program")) tr_render_step("complete_compare_branch_program");
}

/* Function: level_12
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 10
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_12()
{
	draw_regs_arrow(check_display_reg_lv_arrow());
	draw_bufs_arrow(check_display_buf_arrow());
	draw_im_up_arrow(chk_display_imm_up_arrow());
	//rg_draw_flag_boxes();

	int size = cw_get_code_list_size();
	 
	if (size == 0 && tr_is_active("welcome")) tr_render_step("welcome");
}

/* Function: level_11
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 10
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_11()
{
	draw_regs_arrow(check_display_reg_lv_arrow());
	draw_bufs_arrow(check_display_buf_arrow());
	draw_im_up_arrow(chk_display_imm_up_arrow());

	int size = cw_get_code_list_size();

	if (size == 0 && tr_is_active("welcome")) tr_render_step("welcome");
}

/* Function: level_10
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 10
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_10()
{
	draw_regs_arrow(check_display_reg_lv_arrow());
	draw_bufs_arrow(check_display_buf_arrow());
	draw_im_up_arrow(chk_display_imm_up_arrow());

	int size = cw_get_code_list_size();

	if (size == 0 && tr_is_active("welcome")) tr_render_step("welcome");
	else if (size == 0 && tr_is_active("introduce_immediates")) tr_render_step("introduce_immediates");
	else if (size == 0 && tr_is_active("select_immediate")) tr_render_step("select_immediate");
	else if (size == 0 && tr_is_active("explain_immediate_read_only")) tr_render_step("explain_immediate_read_only");
	else if (size == 0 && tr_is_active("show_immediate_examples")) tr_render_step("show_immediate_examples");
}

/* Function: level_9
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 10
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_9()
{
	draw_regs_arrow(check_display_reg_lv_arrow());
	draw_bufs_arrow(check_display_buf_arrow());

	int code_size = cw_get_code_list_size();

	if (g_lv_msg[MSG0] == true && code_size == 0){
		tx_text_box(TX_BIG_BOX, MSG0, TX_SYSMES); //Welcome msg
		tx_bottom_msg(TX_BIG_BOX, TX_MSG_CLICKANY);
		chk_ms_pressed_clear_msg(MSG0, true);
	}
}


/* Function: level_8
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 3
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_8(void)
{
    draw_regs_arrow(
        check_display_reg_lv_arrow()
    );

    draw_bufs_arrow(
        check_display_buf_arrow()
    );

    cs_context_t context =
        cs_capture_context();

    tr_update(&context);
}

/* Function: level_7
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 3
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_7(void)
{
    draw_regs_arrow(
        check_display_reg_lv_arrow()
    );

    draw_bufs_arrow(
        check_display_buf_arrow()
    );

    cs_context_t context =
        cs_capture_context();

    tr_update(&context);
}

/* Function: level_6
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 3
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_6(void)
{
    draw_regs_arrow(
        check_display_reg_lv_arrow()
    );

    draw_bufs_arrow(
        check_display_buf_arrow()
    );

    cs_context_t context =
        cs_capture_context();

    tr_update(&context);
}

/* Function: level_5
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 3
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_5(void)
{
    draw_regs_arrow(
        check_display_reg_lv_arrow()
    );

    draw_bufs_arrow(
        check_display_buf_arrow()
    );

    cs_context_t context = cs_capture_context();

    tr_update(&context);
}

/* Function: level_4
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 3
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_4(void)
{
    draw_regs_arrow(
        check_display_reg_lv_arrow()
    );

    draw_bufs_arrow(
        check_display_buf_arrow()
    );

    cs_context_t context =
        cs_capture_context();

    tr_update(&context);
}

/* Function: level_3
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 3
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_3(void)
{
    draw_regs_arrow(
        check_display_reg_lv_arrow()
    );

    draw_bufs_arrow(
        check_display_buf_arrow()
    );

    cs_context_t context = cs_capture_context();
    tr_update(&context);
}


/* Function: level_2
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 3
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_2()
{
	draw_regs_arrow(check_display_reg_lv_arrow());
	draw_bufs_arrow(check_display_buf_arrow());

	int size = cw_get_code_list_size();
	bool hold = chk_player_holds_line();
	bool miss_op1 = cw_is_operand_1_pending();
	bool miss_op2 = cw_is_operand_2_pending();
	bool miss_op = cw_is_operand_pending();
	bool sorted = cw_check_code_sorted();

	if (g_lv_msg[MSG0] == true && size == 0){
		tx_text_box(TX_BIG_BOX, MSG0, TX_SYSMES); //Welcome
		tx_bottom_msg(TX_BIG_BOX, TX_MSG_CLICKANY);
		set_code_editable(false, NO_EXCEPTION);
		//set_arrange_enabled(false);
		chk_ms_pressed_clear_msg(MSG0, true);
	} else if (g_lv_msg[MSG1] == true && size == 0){
		tx_text_box(TX_UPPER_BOX, MSG1, TX_INS);// IB can be read only once
		tx_bottom_msg(TX_UPPER_BOX, TX_MSG_CLICKANY);
		ar_display_arrow(AR_IB);
		chk_ms_pressed_clear_msg(MSG1, true);
	} else if (g_lv_msg[MSG2] == true && size == 0){
		tx_text_box(TX_LOWER_BOX, MSG2, TX_INS); //Reg can be read multiples
		tx_bottom_msg(TX_LOWER_BOX, TX_MSG_CLICKANY);
		draw_regs_arrow(true);
		chk_ms_pressed_clear_msg(MSG2, true);
	} else if (size == 0 && hold == false){
		tx_text_box(TX_INS_BOX, MSG3, TX_INS);// Select and drag mov
		ar_display_arrow(AR_INS);
		set_code_editable(true, NO_EXCEPTION);
	} else if (size == 0 && hold == true){
		tx_text_box(TX_CODE_BOX, MSG4, TX_INS); // Drop in code box
		ar_display_arrow(AR_DROP);
	} else if (size == 1 && hold == true && miss_op == true){
		tx_text_box(TX_CODE_BOX, MSG4, TX_INS); // Drop in code box
		ar_display_arrow(AR_DROP);
	} else if (size == 1 
			   && sorted == true 
			   && hold == false 
			   && g_lv_msg[MSG5] == true){
		tx_text_box(TX_LOWER_BOX, MSG5, TX_INS); // All operands are shown
		tx_bottom_msg(TX_LOWER_BOX, TX_MSG_CLICKANY);
		chk_ms_rel_clear_msg(MSG5, true);
	} else if (size == 1 && miss_op1 == true && sorted == true){
		tx_text_box(TX_CODE_BOX, MSG6, TX_INS); //Sel rax
		set_buf_selectable(false);
	} else if (size == 1 
			   && miss_op2 == true 
			   && sorted == true
			   && g_lv_msg[MSG7] == true){
		tx_text_box(TX_CENTER_BOX, MSG7, TX_INS); //Valid op combinations
		tx_bottom_msg(TX_CENTER_BOX, TX_MSG_CLICKANY);
		chk_ms_rel_clear_msg(MSG7, true);
	} else if (size == 1 && miss_op2 == true && sorted == true){
		tx_text_box(TX_UPPER_BOX, MSG8, TX_INS); //Select IB
		set_buf_selectable(true);
		set_reg_selectable(false);
	} else if (size == 1 && miss_op == false && hold == false){
		tx_text_box(TX_INS_BOX, MSG9, TX_INS); //Select second instruction
		ar_display_arrow(AR_INS);
		set_reg_selectable(true);
	} else if (size == 1 && miss_op == false & hold == true){
		tx_text_box(TX_CODE_BOX, MSG4, TX_INS);//Drop in code box
		ar_display_arrow(AR_DROP);
	} else if (size == 2 && miss_op1 == true){
		tx_text_box(TX_CODE_BOX, MSG10, TX_INS);//Use mov [ob], rax several 
		set_reg_selectable(false);
		set_buf_selectable(true);
		ar_init_arrow(AR_CODE);
	} else if (size == 2 && miss_op1 == false && miss_op2 == true){
		tx_text_box(TX_CODE_BOX, MSG10, TX_INS);//Use mov [ob], rax several 
		set_buf_selectable(false);
		set_reg_selectable(true);
		ar_init_arrow(AR_CODE);
	} else if (size == 2 && miss_op == false){
		tx_text_box(TX_CODE_BOX, MSG11, TX_INS); //Right click a complete 
		set_reg_selectable(true);
		//set_arrange_enabled(true);
		ar_display_arrow(AR_CODE);
	}
}

/* Function: level_1
 * -----------------------------------------------------------------------------
 * This functions handles all the special cases of the tutorial of level 1
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void level_1()
{
	int size = cw_get_code_list_size();
	bool hold = chk_player_holds_line();
 	bool play = mc_is_executing();
	int limit = lv_get_level_instructions_limit();
	const int pos_one = 0;
	const int pos_two = 1;
	const int two_instructions = 2;
	bool change_op = false;
	bool mov_instruction = false;
	bool press_play = false;
	bool win = wc_is_satisfied();
	code_line_t *i1= NULL;
	code_line_t *i2 = NULL;
	bool miss_op = cw_is_operand_pending();

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
	if (g_lv_msg[MSG0] == true && size == 3){
		set_code_editable(false, NO_EXCEPTION);
		set_arrange_enabled(false);
		tx_text_box(TX_BIG_BOX, MSG0, TX_SYSMES); //Welcome
		tx_bottom_msg(TX_BIG_BOX, TX_MSG_CLICKANY);
		chk_ms_pressed_clear_msg(MSG0, true);
	} else if (g_lv_msg[MSG1] == true && size == 3){
		set_code_editable(false, NO_EXCEPTION);
		tx_text_box(TX_UPPER_BOX, MSG1, TX_INS); //Rearrange
		tx_bottom_msg(TX_UPPER_BOX, TX_MSG_CLICKANY);
		chk_ms_pressed_clear_msg(MSG1, true);
	} else if (size > limit && hold == false){
		set_code_editable(false, size);
		tx_text_box(TX_CODE_BOX, MSG2, TX_INS); //Select last instruction
		ar_display_arrow(AR_CODE);
	} else if (size > limit && hold == true){
		tx_text_box(TX_CODE_BOX, MSG3, TX_INS); //Delete the instruction
		ar_display_arrow(AR_DEL);
	} else if (change_op == true && hold == false){
		set_code_editable(false, OP2_LAST);
		if (i2->state != CHANGING_OP2){
			tx_text_box(TX_CODE_BOX, MSG4, TX_INS); // Select operand
			ar_display_arrow(AR_OP2);
		} else if (i2->state == CHANGING_OP2){
			tx_text_box(TX_UPPER_BOX, MSG5, TX_INS); // Select IB
			ar_display_arrow(AR_IB);
			ar_init_arrow(AR_CODE);
	} else if (size == 2 && miss_op == false){
		}
	} else if (mov_instruction == true){
		set_code_editable(false, size);
		set_del_enabled(false);
		set_arrange_enabled(true);
		tx_text_box(TX_CODE_BOX, MSG6, TX_INS); //Mov ins to first pos
		ar_display_arrow(AR_CODE);
	} else if (press_play == true && play == false && win == false){
		set_code_editable(false, NO_EXCEPTION);
		tx_text_box(TX_CODE_BOX, MSG7, TX_INS); //Press play
		ar_display_arrow(AR_PLAY);
	}
}

/* Function: level_0
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
static void level_0()
{
	draw_regs_arrow(false);
	int flag = mc_get_operation_flag();

	bool play = mc_is_executing(); //ya
	bool hold = chk_player_holds_line(); //ya 
	int size = cw_get_code_list_size(); //ya
	bool sorted = cw_check_code_sorted();//ya
	bool miss_op = cw_is_operand_pending();
	bool miss_op1 = cw_is_operand_1_pending();
	bool miss_op2 = cw_is_operand_2_pending();
	bool win = wc_is_satisfied();

	if (g_lv_msg[MSG0] == true && size == 0){
		tx_text_box(TX_BIG_BOX, MSG0, TX_SYSMES); //Welcome message
		tx_bottom_msg(TX_BIG_BOX, TX_MSG_CLICKANY);
		chk_ms_pressed_clear_msg(MSG0, true);
	} else if (g_lv_msg[MSG1] == true && size == 0){
		tx_text_box(TX_UPPER_BOX, MSG1, TX_INS); //Read challenge
		tx_bottom_msg(TX_UPPER_BOX, TX_MSG_CLICKANY);
		ar_display_arrow(AR_CHALLENGE);
		chk_ms_pressed_clear_msg(MSG1, true);
	} else if (g_lv_msg[MSG2] == true && size == 0){
		tx_text_box(TX_UPPER_BOX, MSG2, TX_INS); //Move from the Input Buffer
		tx_bottom_msg(TX_UPPER_BOX, TX_MSG_CLICKANY);
		ar_display_arrow(AR_IB);
		chk_ms_pressed_clear_msg(MSG2, true);
	} else if (g_lv_msg[MSG3] == true && size == 0){
		tx_text_box(TX_LOWER_BOX, MSG3, TX_INS); //To the Output Buffer
		tx_bottom_msg(TX_LOWER_BOX, TX_MSG_CLICKANY);
		ar_display_arrow(AR_OB);
		chk_ms_pressed_clear_msg(MSG3, true);
	} else if (g_lv_msg[MSG4] == true && size == 0){
		tx_text_box(TX_CENTER_BOX, MSG4, TX_INS);//To register first
		tx_bottom_msg(TX_CENTER_BOX, TX_MSG_CLICKANY);
		draw_regs_arrow(true);
		chk_ms_pressed_clear_msg(MSG4, true);
	} else if (size == 0 && hold == false){
		tx_text_box(TX_INS_BOX, MSG5, TX_INS);//Select and drag instruction
		ar_display_arrow(AR_INS);
	} else if (size == 0 && hold == true){
		tx_text_box(TX_CODE_BOX, MSG6, TX_INS); // Drop ins in code box
		ar_display_arrow(AR_DROP);
	} else if (size == 1 && hold == true && miss_op == true){
		tx_text_box(TX_CODE_BOX, MSG6, TX_INS); // Drop ins in code box
		ar_display_arrow(AR_DROP);
	} else if (size == 1 && sorted == true && miss_op1 == true){
		tx_text_box(TX_CENTER_BOX, MSG7, TX_INS);//Select rax
		set_code_editable(false, NO_EXCEPTION);
		set_buf_selectable(false);
		set_reg_selectable(true);
		draw_regs_arrow(true);
	} else if(size == 1 && sorted == true && miss_op2 == true){
		tx_text_box(TX_UPPER_BOX, MSG8, TX_INS);//Select input buffer
		set_buf_selectable(true);
		set_reg_selectable(false);
		draw_regs_arrow(false);
		ar_display_arrow(AR_IB);
	} else if(size == 1 && miss_op == false && g_lv_msg[MSG9] == true){
		set_code_editable(false, INS_EXCEPTION);
		tx_text_box(TX_CODE_BOX, MSG9, TX_INS);// Press play button
		ar_display_arrow(AR_PLAY);
		set_arrange_enabled(false);
		if (play == true){
			g_lv_msg[MSG9] = false;
		}
	} else if (flag != MC_WIN && flag != NO_OPERATION ){
		tx_text_box(TX_CENTER_BOX, MSG10, TX_SYSWAR); //ERROR
		tx_bottom_msg(TX_CENTER_BOX, TX_MSG_PRESSBACK);
		ar_display_arrow(AR_ERROR);
	} else if(size == 1 && hold == false && play == false 
			  && g_lv_msg[MSG11] == true && miss_op == false){
		tx_text_box(TX_INS_BOX, MSG11, TX_INS);	 //Select another mov ins
		ar_display_arrow(AR_INS);
	} else if(size == 1 && hold == true && miss_op == false){
		tx_text_box(TX_CODE_BOX, MSG12, TX_INS); // Drop below instruction
		ar_display_arrow(AR_DROP);
	} else if(size == 2 && hold == true){
		tx_text_box(TX_CODE_BOX, MSG12, TX_INS); // Drop below instruction
		ar_display_arrow(AR_DROP);
	} else if (size == 2 && sorted == true && miss_op1 == true){
		tx_text_box(TX_CODE_BOX, MSG13, TX_INS);//Select OB	
		set_code_editable(false, NO_EXCEPTION);
		ar_display_arrow(AR_OB);
	} else if(size == 2 && sorted == true && miss_op == true){
		set_reg_selectable(true);
		tx_text_box(TX_CODE_BOX, MSG14, TX_INS); //Select rax
		draw_regs_arrow(true);
	} else if(size == 2 && miss_op == false && play == false && win == false){
		set_code_editable(false, NO_EXCEPTION);
		tx_text_box(TX_CODE_BOX, MSG15, TX_INS);	//Press play
		ar_display_arrow(AR_PLAY);
	} else if (wc_is_satisfied() == true){
		tx_text_box(TX_CENTER_BOX, MSG16, TX_SYSMES);//Congrats 
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
	assert(level < LV_LEVEL_MAX && level >= 0 && "Invalid level value");
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
	assert(level < LV_LEVEL_MAX && level >= 0 && "Invalid level value");
	
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
	assert(level < LV_LEVEL_MAX && level >= 0 && "Invalid level value");
	
	if (sb_chk_rst_esc_menu_active() == true){
		return;
	}
	switch(level){
		case LV_LEVEL_0:
			level_0();
			break;

		case LV_LEVEL_1:
			level_1();
			break;

		case LV_LEVEL_2:
			level_2();
			break;
	
		case LV_LEVEL_3:
			level_3();
			break;

		case LV_LEVEL_4:
			level_4();
			break;
	
		case LV_LEVEL_5:
			level_5();
			break;
	
		case LV_LEVEL_6:
			level_6();
			break;
		
		case LV_LEVEL_7:
			level_7();
			break;

		case LV_LEVEL_8:
			level_8();
			break;

		case LV_LEVEL_9:
			level_9();
			break;

		case LV_LEVEL_10:
			level_10();
			break;

		case LV_LEVEL_11:
			level_11();
			break;
		
		case LV_LEVEL_12:
			level_12();
			break;
		
		case LV_LEVEL_13:
			level_13();
			break;
		
		case LV_LEVEL_14:
			level_14();
			break;
		
		case LV_LEVEL_15:
			level_15();
			break;

		case LV_LEVEL_16:
			level_16();
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



