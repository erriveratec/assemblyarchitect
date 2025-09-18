#include <assert.h>
#include <stdbool.h>
#include "game_mechanics_mc.h"
#include "aux.h"
#include "button_bt.h"
#include "stages.h"
#include "list.h" 
#include "code_window_cw.h"
#include "instruction_window_iw.h"
#include "buffers_bf.h"
#include "registers_rg.h"
#include "file_fl.h"
#include "dimensions_dm.h"
#include "sdl_config.h"
#include "stage_buttons_sb.h"
#include "levels_lv.h"
#include "mouse_ms.h"
#include "dbg.h"
#include "arrow_ar.h"

#define STUDIO_SCREEN_DELAY 1000 // 1 sec
#define SELECT_PLAYER_TEXT "Which hacker are you?"
#define SELECT_LEVEL_TEXT "Select Level"
#define PLAYER_1_TEXT "HACKER W"
#define PLAYER_2_TEXT "HACKER X"
#define PLAYER_3_TEXT "HACKER Y"



// Escape option menu variables
enum WinMenuStates{
	NO_BUTTON_PRESSED,
	BACK_BUTTON_PRESSED,
	CONT_BUTTON_PRESSED
};

typedef struct level_flags_t{
	bool play;
	bool stop;
	bool stop_enabled;
	bool forward;
	bool backward;
	bool backward_enabled;
	bool non_stop;
} level_flags_t;


int g_player = FL_NO_PLAYER;
static SDL_Rect result_box;

texture_t *g_studio_name = NULL;
texture_t *g_game_title = NULL;
texture_t *g_press_space = NULL;
texture_t *g_win_text = NULL;

void stage_drawings(int level);
static code_line_t *pending_operand_handler();
static void flag_handler(level_flags_t *flags, int clicked_button);
static code_line_t *edit_code(int level_id);
static void reset_level(int level_id, level_flags_t *flags, bool *run_finished);
static int display_run_result(bool win_check);
static void destroy_level(level_flags_t *flags);
static void init_stage_assets();
static void code_updated_actions(int level_id);
static void set_code_editable();
static void reset_code_editable();

/* Function: stage_select_player
 * ----------------------------------------------------------------------------
 * This function displays the screen where the player is selected
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	SELECT_PLAYER_SCREEN if the user have not selected a player. 
 *	LEVEL_SELECTION if a player has been chosen
 */
int stage_select_player()
{
	static texture_array_t *select_player = NULL;
	static bool buttons_created = false;
	static button_t *player_1;
	static button_t *player_2;
	static button_t *player_3;
	int ret_val = LV_SELECT_PLAYER_SCREEN;
	bool player_chosen = false;

	SDL_Rect b = dm_get_upper_title_box();
	int text_h = dm_get_h_stage_titles();

	if (buttons_created == false){
	
		buttons_created = true;
		
		select_player = dw_new_text_texture_by_h(b.w, text_h, C_WHITE, 
															SELECT_PLAYER_TEXT);
		texture_t *b1_texture = dw_create_text_texture(PLAYER_1_TEXT, C_BLACK);
		check_mem(b1_texture);
		texture_t *b2_texture = dw_create_text_texture(PLAYER_2_TEXT, C_BLACK);
		check_mem(b2_texture);
		texture_t *b3_texture = dw_create_text_texture(PLAYER_3_TEXT, C_BLACK);
		check_mem(b3_texture);
		
		SDL_Rect b = dm_get_p1_button_box();
		player_1 = bt_create_button(b, true, true, true, C_WHITE, C_WHITE, 
																	b1_texture);
		check_mem(player_1);
		
		b = dm_get_p2_button_box();
		player_2 = bt_create_button(b, true, true, true, C_WHITE, C_WHITE, 
																	b2_texture);
		check_mem(player_2);

		b = dm_get_p3_button_box();
		player_3 = bt_create_button(b, true, true, true, C_WHITE, C_WHITE, 
																	b3_texture);
		check_mem(player_3);
	}
	dw_draw_wrapped_texture_by_h(b, text_h, select_player);
	
	bt_draw_button(player_1);
	bt_draw_button(player_2);
	bt_draw_button(player_3);

	if (true == bt_check_mouse_click_button(player_1)){
		player_chosen = true;
		g_player = FL_PLAYER_1;
	} else if (true == bt_check_mouse_click_button(player_2)){
		player_chosen = true;
		g_player = FL_PLAYER_2;
	} else if (true == bt_check_mouse_click_button(player_3)){
		player_chosen = true;
		g_player = FL_PLAYER_3;
	}
	if (player_chosen == true){
		ret_val = LV_LEVEL_SELECTION;		
		bt_destroy_button(player_1);
		bt_destroy_button(player_2);
		bt_destroy_button(player_3);
		buttons_created = false;
	}
	
	display_escape_menu(get_escape_menu_state());
	error:
	ms_reset_mouse_values();
	return ret_val;
}

/* Function: stage_title
 * -----------------------------------------------------------------------------
 * This function renders the title screen of the game
 *
 * Arguments:
 *	None;
 *
 * Return:
 *	TITLE_SCREEN if user has not pressed spacebar, 
 *	LEVEL_SELECTION if otherwise
 */
int stage_title(const Uint8 *keystate)
{
	int ret_val;
	SDL_Rect t = dm_get_game_title_box();
	dw_draw_texture_fits_width(t, g_game_title);

	SDL_Rect s = dm_get_press_space_box();
	dw_draw_texture_fits_width(s, g_press_space);

	if (keystate[SDL_SCANCODE_SPACE]){
		ret_val = LV_SELECT_PLAYER_SCREEN;
	} else {
		ret_val = LV_TITLE_SCREEN;
	}

	display_escape_menu(get_escape_menu_state());
	return ret_val;
}

/* Function: stage_studio
 * ----------------------------------------------------------------------------
 * This function displays the stage where the name of the studio is shown.
 *
 * Arguments:
 *	start_time: starting time that will be used for the delay of the screen.
 *	cur_time: the current time used to calculate the delay of the screen.
 *
 * Return:
 *	TITLE_SCREEN if the delay time is already finished
 *	LEVEL_SELECTION if otherwise
 */
int stage_studio(Uint64 start_time, Uint64 cur_time)
{
	int delay = cur_time - start_time;	
	
	SDL_Rect b = dm_get_studio_name_msg_box();
	dw_draw_texture_fits_width(b, g_studio_name);
	
	if (delay > STUDIO_SCREEN_DELAY){
		dw_free_texture(g_studio_name);
		return LV_TITLE_SCREEN;
	} else {
		return LV_STUDIO_SCREEN;
	}
}

/* Function: reset_level_flags
 * -------------------------------------
 * Arguments:
 * 	flags: the flag levels that will be reset
 *
 * Return:
 *	void.	
 */
void reset_level_flags(level_flags_t *flags)
{
	flags->play = false;
	flags->stop = false;
	flags->stop_enabled = false;
	flags->forward = false;
	flags->backward = false;
	flags->backward_enabled = false;
	flags->non_stop = false;
}

/* Function: init_stage_assets
 * ----------------------------------------------------------------------------
 * Initializes several aspects of the stages that were initialized 
 * as the level was created. Independly of the level that player is playing.
 * the stages should be initialized. Code box and instruction box should not
 * be initialized here.
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	void.	
 */
static void init_stage_assets()
{
	sb_initialize_stage_buttons();

	SDL_Rect r0 = dm_get_stage_input_buffer_box();
	bf_set_input_box(r0);
	
	SDL_Rect r1 = dm_get_stage_output_buffer_box();
	bf_set_output_box(r1);

	SDL_Rect ib = dm_get_stage_ib_text_box();
	SDL_Rect r2 = {.x = r0.x, .y = ib.y, .w = r0.w, .h = ib.h + r0.h};
	bf_set_input_buffer_button(r2);

	SDL_Rect ob = dm_get_stage_ob_text_box();
	SDL_Rect r3 = {.x = r1.x, .y = r1.y, .w = r1.w, .h = ob.h + r1.h};
	bf_set_output_buffer_button(r3);

	bf_initialize_buffer_operands();
	tx_init_text_boxes();
}

/* Function: init_level
 * -------------------------------------
 * Arguments:
 * stage_id: the id for the specific stage that is going to be played
 *
 * Return:
 *	true: used for the caller of the function.
 */
void init_level(int level_id)
{
	assert(level_id > LV_LEVEL_MIN && level_id < LV_LEVEL_MAX && 	
		   "Invalid stage id");
	
	
	init_stage_assets();
	bf_create_input_list();
	bf_create_output_list();
	bf_reset_win_condition();

	fl_load_level_msgs(level_id);
	lv_create_win_list();
	lv_init_level_assets(level_id);

	//goes before the load level
	SDL_Rect r0 = dm_get_stage_registers_box();
	rg_set_register_box(r0); 	
	
	fl_file_initialize_level(level_id);
	
	// must go after level loading
	rg_init_value_boxes(); 	

	SDL_Rect r1 = dm_get_stage_code_box();
	cw_set_code_box(r1);
	
	SDL_Rect r2 = dm_get_stage_instruction_box();
	iw_set_instruction_box(r2);
	
	cw_create_code_list();	
	fl_load_save_file(g_player, level_id);
	mc_init_avatar();

	ar_initialize_arrows();
	return;
}

/* Function: destroy_level
 * -----------------------------------------------------------------------------
 * This function should be called each time the player leaves a level as it
 * should clean everything for the next invocation of the level.
 * 
 * Arguments:
 * stage_id: the id for the specific stage that is going to be played
 *
 * Return:
 *	void
 */
static void destroy_level(level_flags_t *flags)
{
	bf_destroy_buffer_lists(); 
	lv_destroy_win_list(); 
	cw_destroy_code_window_assets(); 
	iw_destroy_instruction_list(); 
	tx_free_level_text_textures();
	rg_destroy_register_list();
	rg_destroy_value_boxes();
	reset_level_flags(flags);
	bf_reset_input_list_x_pos();
	mc_destroy_avatar_textures();
	return ;
}
/* Function: stages_drawings
 * -----------------------------------------------------------------------------
 * Does all the drawings regarding a level
 *
 * Arguments:
 *	level: id of the level that is being rendered
 *
 * Return:
 *	Void.
 */
void stage_drawings(int level)
{
	iw_display_instructions();
	cw_draw_code_window();	
	sb_draw_stage_buttons(cw_get_code_list_size());
	mc_draw_avatar();
	ar_display_arrow(AR_EXEC);
	bf_draw_buffers();
	rg_draw_registers();
	lv_level_drawings(level);
	sb_draw_return_button();
	return;
}

/* Function: create_select_level_buttons
 * -----------------------------------------------------------------------------
 * Creates the total of buttons according to the numbers of levels that will be
 * present in the game.
 * 	
 * Arguments:
 * 	buttons_array: an array of buttons that will be created.
 *  player_levels: the list of levels that the player will have available
 *
 * Return:
 *	void
 */
static void create_select_level_buttons(button_t **buttons, bool *levels)
{
	assert(buttons != NULL && "The buttons pointer is NULL");
	assert(levels != NULL && "The levels pointer is NULL");
	
	SDL_Rect r = dm_get_level_button_box();
	SDL_Rect b = r;
	int y_offset = get_sel_level_offset_y();
	for (int i = 1; i <= LV_LEVEL_QUANTITY; i++){
		char *button_text = create_string_append_number(level_text, i);
		texture_t *button_texture = NULL;
		if (levels[i-1] == true){
			button_texture = dw_create_text_texture(button_text, 
							 C_BLACK);
			buttons[i-1] = bt_create_button(b, true, true, true, C_WHITE, 
											C_WHITE, button_texture);
		} else {
			button_texture = dw_create_text_texture(button_text, 
							 C_GREY);
			buttons[i-1] = bt_create_button(b, false, true, false, C_BLACK,
											C_WHITE, button_texture);
		}
		b.y += y_offset;	
		if (i != 0 && i%8 == 0){
			b.x += r.x + r.w;
			b.y = r.y;
		}
	}
}

/* Function: stage_select_level
 * -----------------------------------------------------------------------------
 * Displays the levels that a player has available according to its save file
 * 	
 * Arguments:
 * 	None.
 *
 * Return:
 *	The number of the level selected, if not valid press was performed
 *	the number of the select label stage will be returned.
 */
int stage_select_level()
{
	static texture_array_t *select_level = NULL;

	static bool level_initialized = false;
	static button_t *level_buttons[40];
	static bool player_levels[LV_LEVEL_QUANTITY];
	int ret_val = LV_LEVEL_SELECTION;
	SDL_Rect r = dm_get_upper_title_box();

	if (level_initialized == false){
		fl_load_player_levels(g_player, player_levels);
		level_initialized = true;
		create_select_level_buttons(level_buttons, player_levels);
		select_level = dw_new_text_texture_by_h(r.w, r.h, C_WHITE, 
															 SELECT_LEVEL_TEXT);
	}
	dw_draw_wrapped_texture_by_h(r, r.h, select_level);

	for (int i = 0; i < LV_LEVEL_QUANTITY; i++){
		bt_draw_button(level_buttons[i]);
		if (bt_check_mouse_click_button(level_buttons[i]) == true){
			ret_val = LV_LEVEL_1 + i;
			level_initialized = false;
			bt_destroy_button(level_buttons[i]);
		}
	}

	sb_draw_return_button();
	display_escape_menu(get_escape_menu_state());

	if (sb_check_clicked_ret_button() == true){
		ret_val = LV_SELECT_PLAYER_SCREEN;	
		level_initialized = false;
		for (int i = 0; i < LV_LEVEL_QUANTITY; i++){
			bt_destroy_button(level_buttons[i]);
		}
	}
	ms_reset_mouse_values();
	return ret_val;
}

/* Function: stage_button_handler
 * ----------------------------------------------------------------------------
 * Arguments:
 * 	None.
 *
 * Return:
 *	void.
 */
void stage_button_handler()
{
	int clicked_button = identify_clicked_stage_button();
	
	switch(clicked_button){
		case STOP:
			
			break;
		case BACKWARD:

			break;

		case PLAY:

			break;

		case FORWARD:

			break;

		case INVALID:

			break;
	}
}

/* Function: flag_handler
 * ----------------------------------------------------------------------------
 * Sets the value of flags of the state of the game
 *
 * Arguments:
 * 	flags: level flags.
 *	clicked_button: ide of the stage button clicked by the player
 *
 * Return:
 *	void.
 */
static void flag_handler(level_flags_t *flags, int clicked_button)
{
	assert(clicked_button != INVALID && "Invalid clicked button");
	assert(flags != NULL && "The flags pointer is NULL");
	
	switch(clicked_button){
		case PLAY: 
			if (flags->play != true){
				flags->play = true;
				flags->non_stop = true;
				flags->stop = false;
				flags->stop_enabled = true;
				flags->forward = false;
				flags->backward = false;
				flags->backward_enabled = true;
				ar_reset_execution_arrow();
			}
			break;
		case STOP:
			flags->stop = true;
			flags->non_stop = false;
			flags->play = false;
			flags->forward = false;
			flags->backward = false;
			flags->backward_enabled = false;
			mc_reset_invalid_operation_flag();	
			break;
		case FORWARD:
			flags->forward = true;
			flags->play = true;
			flags->stop = false;
			flags->non_stop = true;
			flags->stop_enabled = true;
			flags->backward = false;
			flags->backward_enabled = true;
			break;
		case BACKWARD:
			flags->backward = true;
			flags->stop = false;
			flags->non_stop = true;
			flags->stop_enabled = true;
			flags->play = false;
			flags->forward = false;
			break;
	}
}
/* Function: code_updated_actions
 * ----------------------------------------------------------------------------
 * Performs all the actions that need to be done in the game logic when
 * the code has been updated with a new line or deleted line
 *
 * Arguments:
 * 	Void.
 *
 * Return:
 *	Void.
 */
static void code_updated_actions(int level_id)
{
	fl_save_level(g_player, level_id);
	lv_upd_level_assets(level_id);
}

/* Function: pending_operand_handler
 * ----------------------------------------------------------------------------
 * This function is called when an instruction is pending an operand.
 * In case that the instruction that is pending an operand is a jump, it
 * generates the label operand to be put on the code
 * 
 * Arguments:
 * 	Void.
 *
 * Return:
 *	NULL, if the peding operand is not a line, LABEL if the operand is a line.
 */
static code_line_t *pending_operand_handler()
{
	bool left_released = ms_chk_mouse_left_released();
	bool register_selected = rg_check_mouse_released_in_register();
	bool buffer_selected = bf_check_released_in_buffer();
	bool label_selected = cw_check_released_in_label();
	cw_highlight_code_pending_operand();
	code_line_t *l = cw_get_code_line_pending_operand();
	code_line_t *r = NULL;

	if (l->ins->id == JMP){
		r = cw_create_label_code_line();
		cw_player_holding_instruction(r);
		operand_t *a = cw_create_jump_operand(r);
		cl_assign_operand_to_line(a, l);
	} else if (left_released == true && register_selected == true &&
												lv_is_reg_selectable() == true){
		operand_t *r = rg_create_operand_of_selected_register();
		cl_assign_operand_to_line(r, l);
	} else if (left_released == true && buffer_selected == true && 
												lv_is_buf_selectable() == true){
		operand_t *b = bf_create_operand_of_selected_buffer();
		if (cl_check_operand_compatibility(b, l) == true){
			cl_assign_operand_to_line(b, l);
		} else {
			cl_destroy_operand(b);
		}
	} else if (left_released == true && register_selected == false && 
			   buffer_selected == false){
		if (l->state == CHANGING_OP1 || l->state == CHANGING_OP2){
			l->state = COMPLETE;	
		}
	}
	return r;
}

/* Function: edit_code
 * ----------------------------------------------------------------------------
 * This function is called when the player is able to edit or alter the code
 * that is present in the code window.
 *
 * Arguments:
 * 	level_id: Required for the save file when the code is saved.
 *
 * Return:
 *	true if the player is holding a line, false if otherwise
 */
static code_line_t *edit_code(int level_id)
{
	assert(level_id > 0 && level_id <= LV_LEVEL_QUANTITY && 
		   											"Incorrect level_id value");
		
	static code_line_t *line = NULL;
	bool left_pressed = ms_chk_mouse_left_pressed();
	bool left_released = ms_chk_mouse_left_released();
	static bool hold_line = false;
	
	if (cw_check_code_pending_operand() == true && line == NULL &&
		cw_check_code_sorted() == true && cw_check_clicked_code() == false){
		line = pending_operand_handler();	
		if (cw_check_code_pending_operand() == false){
			code_updated_actions(level_id);
		}
		if (line != NULL){
			hold_line = true;
		}
	} else if (cw_check_clicked_code_operand() == true && line == NULL){
		cw_change_clicked_code_line_state();	
	} else if (iw_check_clicked_instruction() == true && line == NULL &&
												 lv_is_code_editable() == true){
		line = cl_new_code_line(iw_get_clicked_instruction());
	} else if (cw_check_clicked_code() == true && line == NULL && 
												 lv_is_code_editable() == true){
		line = cw_get_clicked_code();
	} else if ((left_pressed == true || hold_line == true) && line != NULL){
		cw_player_holding_instruction(line);
		hold_line = (left_released == true) ? false : true;
	}  else if (left_pressed == false && NULL != line){
		if (cw_check_if_in_code_list(line) == false){
			cl_destroy_code_line(line);
		} 
		if (cw_check_code_pending_operand() == false){
			code_updated_actions(level_id);
		}
		line = NULL;
	}
	return line; // I must change this as it is not required
}

/* Function: reset_level
 * ----------------------------------------------------------------------------
 * Arguments:
 * 	level_id: the level number that is going to be reset.
 *	flags: the flags of the level that will be reset.
 *
 * Return:
 *	void.
 */
static void reset_level(int level_id, level_flags_t *flags, bool *run_finished)
{
	mc_reset_avatar();			
	reset_level_flags(flags);
	rg_reset_register_values();
	bf_reset_input_list();
	bf_reset_output_list();
	bf_reset_win_condition();
	lv_reset_win_list();
	lv_reset_level_win_condition();
	cw_reset_code_execution();
	ar_hide_execution_arrow();
	mc_reset_invalid_operation_flag();
	*run_finished = false;
	ms_reset_mouse_values();
	rg_reset_ibox();
	rg_reset_obox();
}

int stage_level(int level_id)
{
	int ret_val = LV_PLAY_LEVEL;
	static bool run_finished = false;
	static bool reset = false;
	static code_line_t *hold_line = NULL;
	bool back_to_level_selection = sb_check_clicked_ret_button(); 
	static level_flags_t flags;
	display_escape_menu(get_escape_menu_state());
	
	if (sb_check_clicked_stage_button() == true && 
									  cw_check_code_pending_operand() == false){
		flag_handler(&flags, identify_clicked_stage_button());
	}
	
	lv_set_op_flag_state(mc_get_operation_flag());
	lv_set_play_state(flags.play);
	lv_set_hold_line(hold_line);
	stage_drawings(level_id);
	cw_sort_code();
	
	if (flags.non_stop == false || cw_check_code_pending_operand() == true){
		hold_line = edit_code(level_id);
	}
	if (mc_get_operation_flag() != NO_INVALID_OPERATION){
		reset = mc_invalid_operation_handler(mc_get_operation_flag());
		flags.play = false;
	} else if (run_finished == true && flags.play == true &&
													 lv_check_if_win() == true){
		int action_selected = display_run_result(lv_check_if_win());
		if (action_selected == BACK_BUTTON_PRESSED){
			reset_level(level_id, &flags, &run_finished);		
			fl_enable_next_level(g_player, level_id + 1);
		} else if (action_selected == CONT_BUTTON_PRESSED){
			fl_enable_next_level(g_player, level_id + 1);
			back_to_level_selection = true;
		} 
	}
	if (flags.play == true && cw_check_code_pending_operand() == false){
		run_finished = mc_run_code();
	} else if ((flags.stop == true && flags.stop_enabled == true) ||
				reset == true){
		reset_level(level_id, &flags, &run_finished);	
		reset = false;
	}
	if (back_to_level_selection == true){
		ret_val = LV_LEVEL_SELECTION;	
		reset_level(level_id, &flags, &run_finished);		
		destroy_level(&flags);
		run_finished = false;
	}
	display_escape_menu(get_escape_menu_state());
	return ret_val;
}


/* Function: display_run_result
 * 	
 * Arguments:
 *	win_check: the condition checking if the run was successful
 *
 * Return:
 *	The id of the button pressed by the player.
 */
static int display_run_result(bool win_check)
{

	bf_set_win_condition();
	SDL_Rect r = dm_get_text_box_result();	
	dw_draw_filled_rectangle(r, C_WHITE, C_BLACK);

	SDL_Rect s = dm_get_text_box_result_text1();
	dw_draw_texture_fits_width(s, g_win_text);
	
	static bool buttons_created = false;
	static button_t *ret;
	static button_t *con;
	bool button_pressed = false;
	int action_selected = NO_BUTTON_PRESSED;

	if (buttons_created == false){
		buttons_created = true;
		texture_t *con_texture = dw_create_text_texture(
								 STR_CONT, C_BLACK);
		check_mem(con_texture);
		SDL_Rect r = dm_get_text_box_result_but2();
		con = bt_create_button(r, true, true, false, C_BLACK, C_BLACK, 
							   con_texture);
		check_mem(con);
					
		texture_t *ret_texture = dw_create_text_texture(
								 STR_BACK, C_BLACK);
		check_mem(ret_texture);
		SDL_Rect b = dm_get_text_box_result_but1();

		ret = bt_create_button(b, true, true, false, C_BLACK, C_BLACK, 
							   ret_texture);
		check_mem(ret);
	} 
	bt_draw_button(ret);
	bt_draw_button(con);

	if (bt_check_mouse_click_button(con) == true){
		button_pressed = true;
		action_selected = CONT_BUTTON_PRESSED;
	} 
	if (bt_check_mouse_click_button(ret) == true){
		button_pressed = true;
		action_selected = BACK_BUTTON_PRESSED;
	} 	
	if (button_pressed == true){
		bt_destroy_button(ret);
		if (win_check == true){
			bt_destroy_button(con);
		}
		buttons_created = false;
	}
	error:

	return action_selected;
}



