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
#include "dimensions.h"
#include "sdl_config.h"
#include "stage_buttons_sb.h"
#include "levels_lv.h"
#include "mouse_ms.h"
#include "dbg.h"

#define STUDIO_SCREEN_DELAY 1000 // 1 sec

#define ESC_MENU_TEXT1 "Return to Game"
#define ESC_MENU_TEXT2 "Toggle Full Screen"
#define ESC_MENU_TEXT3 "Exit Game"

#define WIN_MENU_TEXT1 "Back"
#define WIN_MENU_TEXT2 "Continue"

// Escape option menu variables
bool g_quit = false;

int g_player = FL_NO_PLAYER;

bool g_escape_menu = false;

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

static char *win_text = "The challenge is correct";
static char *lose_text = "The challenge is incorrect";
static SDL_Rect result_box;

void display_level_inputs(int x, int y, List *l);
void stage_drawings();
static void pending_operand_handler();
static void flag_handler(level_flags_t *flags, int clicked_button);
static void edit_code(int level_id);
static void reset_level(int level_id, level_flags_t *flags, bool *run_finished);
static int display_run_result(bool win_check);
static bool get_escape_menu_state();
static void display_escape_menu(bool menu_variable_state);
static void destroy_level(level_flags_t *flags);

/* Function: initialize_stage_assets
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
void initialize_stage_assets()
{
	sb_initialize_stage_buttons();
	sb_initialize_return_button();

	bf_set_input_box(BUFFER_BOX_X, INPUT_BUFFER_BOX_Y, BUFFER_BOX_W, 
					 BUFFER_BOX_H);
	bf_set_output_box(BUFFER_BOX_X, OUTPUT_BUFFER_BOX_Y, BUFFER_BOX_W, 
				   	  BUFFER_BOX_H);
	
	bf_set_input_buffer_button(BUFFER_BOX_X, INPUT_BUFFER_TEXT_Y, BUFFER_BOX_W, 
							BUFFER_TEXT_H + BUFFER_BOX_H);
	bf_set_output_buffer_button(BUFFER_BOX_X, OUTPUT_BUFFER_BOX_Y, BUFFER_BOX_W, 
							 BUFFER_TEXT_H + BUFFER_BOX_H);
	bf_initialize_buffer_operands();
	}

/* Function: set_quit_game_value
 * ----------------------------------------------------------------------------
 * Sets the quit game val to tru
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	void.	
 */
void set_quit_game()
{
	g_quit = true;
}

/* Function: get_quit_game_value
 * ----------------------------------------------------------------------------
 * Gets the quit game val to tru
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	Bool with the state of the quit game variable	
 */
bool get_quit_game_value()
{
	return g_quit;
}

/* Function: player_pressed_escape_key
 * ----------------------------------------------------------------------------
 * This function inverts the state of the escape menu flag in case that the 
 * player has pressed the escape key.
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	void.	
 */
void toggle_escape_menu()
{
	g_escape_menu = !g_escape_menu;
}

/* Function: get_escape_menu_state
 * ----------------------------------------------------------------------------
 * This function return the boolean state of the state.
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	Boolean with the state fo the escape_menu variable
 */
static bool get_escape_menu_state()
{
	return g_escape_menu;
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

/* Function: level_initialization
 * -------------------------------------
 * Arguments:
 * stage_id: the id for the specific stage that is going to be played
 *
 * Return:
 *	true: used for the caller of the function.
 */
int level_initialization(int level_id)
{
	assert(level_id > LV_LEVEL_MIN && level_id < LV_LEVEL_MAX && 	
		   "Invalid stage id");

	bf_create_input_list();
	bf_create_output_list();
	lv_create_win_list();
	fl_file_initialize_level(level_id);
	
	cw_set_code_box(CODE_BOX_X, CODE_BOX_Y, CODE_BOX_W, CODE_BOX_H);
	iw_set_instruction_box(INS_BOX_X, INS_BOX_Y, INS_BOX_W, INS_BOX_H);
	
	lv_generate_win_condition_list(level_id);
	cw_create_code_list();	
	fl_load_save_file(g_player, level_id);
	mc_reset_avatar();

	return true;
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
	bf_destroy_buffer_lists(); //destruction of textures not needed
	lv_destroy_win_list(); //destruction of textures not needed
	cw_destroy_code_window_assets(); //destruction of textures done
	iw_destroy_instruction_list(); //destruction of textures done
	rg_destroy_register_list();
	reset_level_flags(flags);
	bf_reset_input_list_x_pos();
	return ;
}
/* Function: stages_drawings
 * -----------------------------------------------------------------------------
 * Arguments:
 *	input_list: the list of inputs that are going to be available to the player
 *	instruction_list: the list of available instructions in the level
 *	code: the developed code of the player
 *
 * Return:
 *	Void.
 */
void stage_drawings()
{
	draw_buffers();
	
	draw_register_text(REG_TEXT_X, REG_TEXT_Y, REG_TEXT_H);

	display_registers();
	iw_display_instructions(INS_BOX_X, INS_BOX_Y);

	cw_draw_code_window();	

	draw_stage_buttons(cw_get_code_list_size());

	mc_draw_avatar();
	mc_draw_execution_arrow();

	draw_return_button();

	return;
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
 *	LV_TITLE_SCREEN if the delay time is already finished
 *	LV_LEVEL_SELECTION if otherwise
 */
int stage_studio(Uint64 start_time, Uint64 cur_time)
{
	int delay = cur_time - start_time;	
	dw_draw_text_fits_width(STUDIO_NAME_X, STUDIO_NAME_Y, STUDIO_NAME_W, 
						 COLOR_WHITE, STUDIO_NAME_TEXT);

	if (delay > STUDIO_SCREEN_DELAY){
		return LV_TITLE_SCREEN;
	} else {
		return LV_STUDIO_SCREEN;
	}
}

/* Function: stage_select_player
 * ----------------------------------------------------------------------------
 * This function displays the screen where the player is selected
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	LV_SELECT_PLAYER_SCREEN if the user have not selected a player. 
 *	LV_LEVEL_SELECTION if a player has been chosen
 */
int stage_select_player()
{
	static bool buttons_created = false;
	static button_t *player_1;
	static button_t *player_2;
	static button_t *player_3;
	int ret_val = LV_SELECT_PLAYER_SCREEN;
	bool player_chosen = false;

	if (buttons_created == false){
		buttons_created = true;
		texture_t *b1_texture = load_texture_from_rendered_text(PLAYER_1_TEXT, 
								COLOR_WHITE);
		check_mem(b1_texture);
		texture_t *b2_texture = load_texture_from_rendered_text(PLAYER_2_TEXT, 
								COLOR_WHITE);
		check_mem(b2_texture);
		texture_t *b3_texture = load_texture_from_rendered_text(PLAYER_3_TEXT, 
								COLOR_WHITE);
		check_mem(b3_texture);

		player_1 = create_button(P1_BUTTON_X, P1_BUTTON_Y, P_BUTTON_H, 
				   P_BUTTON_W, true, true, b1_texture);
		check_mem(player_1);
		player_2 = create_button(P2_BUTTON_X, P2_BUTTON_Y, P_BUTTON_H, 
				   P_BUTTON_W, true, true, b2_texture);
		check_mem(player_2);
		player_3 = create_button(P3_BUTTON_X, P3_BUTTON_Y, P_BUTTON_H, 
			       P_BUTTON_W, true, true, b3_texture);
		check_mem(player_3);
	}
	
	draw_text_fits_height(SELECT_PLAYER_TEXT_X, SELECT_PLAYER_TEXT_Y, 
						 SELECT_PLAYER_TEXT_H, COLOR_WHITE, SELECT_PLAYER_TEXT);
	
	bt_draw_button(player_1);
	bt_draw_button(player_2);
	bt_draw_button(player_3);

	if (true == check_mouse_click_in_button(player_1)){
		player_chosen = true;
		g_player = FL_PLAYER_1;
	} else if (true == check_mouse_click_in_button(player_2)){
		player_chosen = true;
		g_player = FL_PLAYER_2;
	} else if (true == check_mouse_click_in_button(player_3)){
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
 *	LV_TITLE_SCREEN if user has not pressed spacebar, 
 *	LV_LEVEL_SELECTION if otherwise
 */
int stage_title(const Uint8 *keystate)
{
	int ret_val;
	dw_draw_text_fits_width(GAME_TITLE_X, GAME_TITLE_Y, GAME_TITLE_W, COLOR_WHITE, 
						 GAME_TITLE_TEXT);
	dw_draw_text_fits_width(SPACE_TEXT_X, SPACE_TEXT_Y, SPACE_TEXT_W, COLOR_WHITE, 
						 PRESS_SPACE_TEXT);

	if (keystate[SDL_SCANCODE_SPACE]){
		ret_val = LV_SELECT_PLAYER_SCREEN;
	} else {
		ret_val = LV_TITLE_SCREEN;
	}

	display_escape_menu(get_escape_menu_state());
	return ret_val;
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
	
	int x = SEL_LEVEL_BUTTON_X;
	int y = SEL_LEVEL_BUTTON_Y;

	for (int i = 1; i <= LV_LEVEL_QUANTITY; i++){
		char *button_text = create_string_append_number(level_text, i);
		texture_t *button_texture = NULL;
		if (levels[i-1] == true){
			button_texture = load_texture_from_rendered_text(button_text, 
							 COLOR_WHITE);
			buttons[i-1] = create_button(x, y, SEL_LEVEL_BUTTON_W, 
									 SEL_LEVEL_BUTTON_H, true, true, 
									 button_texture);
		} else {
			button_texture = load_texture_from_rendered_text(button_text, 
							 COLOR_GREY);
			buttons[i-1] = create_button(x, y, SEL_LEVEL_BUTTON_W, 
									 SEL_LEVEL_BUTTON_H, true, true, 
									 button_texture);
		}
				
				y += SEL_LEVEL_OFFSET_Y;	
		
		if (i != 0 && i%8 == 0){
			x += SEL_LEVEL_BUTTON_X + SEL_LEVEL_BUTTON_W;
			y = SEL_LEVEL_BUTTON_Y;
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
	static bool level_initialized = false;
	static button_t *level_buttons[40];
	static bool player_levels[LV_LEVEL_QUANTITY];
	int ret_val = LV_LEVEL_SELECTION;
	
	if (level_initialized == false){
		fl_load_player_levels(g_player, player_levels);
		level_initialized = true;
		create_select_level_buttons(level_buttons, player_levels);
	}

	dw_draw_text_fits_width(SEL_LEVEL_TEXT_X, SEL_LEVEL_TEXT_Y, 
							SEL_LEVEL_TEXT_W, COLOR_WHITE, SEL_LEVEL_TEXT);

	for (int i = 0; i < LV_LEVEL_QUANTITY; i++){
		bt_draw_button(level_buttons[i]);
		if (check_mouse_click_in_button(level_buttons[i]) == true){
			ret_val = LV_LEVEL_1 + i;
			level_initialized = false;
			bt_destroy_button(level_buttons[i]);
		}
	}

	draw_return_button();
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
 * Arguments:
 * 	code: developed by the player.
 *
 * Return:
 *	void.
 */
static void flag_handler(level_flags_t *flags, int clicked_button)
{
	assert(clicked_button != INVALID && "Invalid clicked button");
	assert(flags != NULL && "The flags pointer is NULL");
	
	//reset_level_flags(flags);
	
	switch(clicked_button){
		case PLAY: 
			flags->play = true;
			flags->non_stop = true;
			flags->stop = false;
			flags->stop_enabled = true;
			flags->forward = false;
			flags->backward = false;
			flags->backward_enabled = true;
			mc_reset_execution_arrow();
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

/* Function: pending_operand_handler
 * ----------------------------------------------------------------------------
 * Arguments:
 * 	code: developed by the player.
 *
 * Return:
 *	void.
 */
static void pending_operand_handler()
{
	bool left_released = ms_check_mouse_left_released();
	bool register_selected = rg_check_released_in_register();
	bool buffer_selected = bf_check_released_in_buffer();
	bool label_selected = cw_check_released_in_label();
	cw_highlight_code_pending_operand();
	code_line_t *l = cw_get_code_line_pending_operand();

	if (l->ins->id == JMP){
		if (label_selected == true){
			operand_t *a = cw_create_jump_operand();
			cl_assign_operand_to_line(a, l);
		}
	} else if (left_released == true && register_selected == true){
		operand_t *r = rg_create_operand_of_selected_register();
		cl_assign_operand_to_line(r, l);
	} else if (left_released == true && buffer_selected == true){
		operand_t *b = bf_create_operand_of_selected_buffer();
		if (check_operand_compatilibity(b, l) == true){
			cl_assign_operand_to_line(b, l);
		} else {
			cl_destroy_operand(b);
		}
	} else if (left_released == true && register_selected == false && 
			   buffer_selected == false){
		operand_t *r = rg_get_default_operand_register();
		cl_assign_operand_to_line(r, l);
	}
	return;
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
 *	void.
 */
static void edit_code(int level_id)
{
	assert(level_id > 0 && level_id <= LV_LEVEL_QUANTITY && 
		   "Incorrect level_id value");
	static code_line_t *line = NULL;
	bool left_pressed = ms_check_mouse_left_pressed();

	if (cw_check_code_pending_operand() == true && 
		cw_check_all_code_sorted() == true && cw_check_clicked_code() == false){
		pending_operand_handler();	
		if (cw_check_code_pending_operand() == false){
			fl_save_level(g_player, level_id);
		}
	} else if (cw_check_clicked_code_operand() == true && line == NULL){
		ms_disable_mouse_button();
		cw_change_clicked_code_line_state();	
	} else if (iw_check_clicked_instruction() == true && line == NULL){
		line = cl_new_code_line(iw_get_clicked_instruction());
	} else if (cw_check_clicked_code() == true && line == NULL){
		line = cw_get_clicked_code();
	} else if (left_pressed == true && line != NULL){
		cw_player_holding_instruction(line);
	} else if (left_pressed == false && NULL != line){
		if (cw_check_if_in_code_list(line) == false){
			cl_destroy_code_line(line);
		} 
		if (cw_check_code_pending_operand() == false){
			fl_save_level(g_player, level_id);
		}
		
		line = NULL;
	}
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
	reset_register_values();
	bf_reset_input_list();
	bf_reset_output_list();
	reset_win_list();
	lv_generate_win_condition_list(level_id);
	cw_reset_code_execution();
	mc_hide_execution_arrow();
	mc_reset_invalid_operation_flag();
	*run_finished = false;
	ms_reset_mouse_values();
}

int stage_level(int level_id)
{
	int ret_val = level_id;
	static bool level_init = false;
	static bool run_finished = false;
	static level_flags_t flags;
	static bool reset = false;
	bool back_to_level_selection = sb_check_clicked_ret_button(); 

	if (level_init == false){
		level_init = level_initialization(level_id);
	}
	if (check_clicked_stage_button() == true){
		flag_handler(&flags, identify_clicked_stage_button());
	}
	
	stage_drawings();
	cw_sort_code();
	
	if (flags.non_stop == false || cw_check_code_pending_operand() == true){
		edit_code(level_id);
	}
	if (run_finished == true || lv_check_if_win() == true){
		int action_selected = display_run_result(lv_check_if_win());
		if (action_selected == BACK_BUTTON_PRESSED){
			reset_level(level_id, &flags, &run_finished);		
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
	if (mc_get_operation_flag() != NO_INVALID_OPERATION){
		reset = mc_invalid_operation_handler(mc_get_operation_flag());
		flags.play = false;
	}
	if (back_to_level_selection == true){
		ret_val = LV_LEVEL_SELECTION;	
		level_init = false;
		reset_level(level_id, &flags, &run_finished);		
		destroy_level(&flags);
		run_finished = false;
	}
	display_escape_menu(get_escape_menu_state());
	return ret_val;
}

/* Function: display_escape_menu
 * -----------------------------------------------------------------------------
 * This function is called in all the stages, it will show the escape menu 
 * according to the state of the g_escape_menu variable.
 * 
 * Arguments:
 *	menu_variable_state: The state of the show menu variable.
 *
 * Return:
 *	void
 */
static void display_escape_menu(bool menu_variable_state)
{
	static bool buttons_created = false;
	static button_t *player_1;
	static button_t *player_2;
	static button_t *player_3;

	if (menu_variable_state == true){
		
		dw_draw_filled_rectangle(ESC_MENU_BOX_X, ESC_MENU_BOX_Y, ESC_MENU_BOX_W, 
					   ESC_MENU_BOX_H, COLOR_BLACK, COLOR_WHITE);

		bool player_chosen = false;

		if (buttons_created == false){
			buttons_created = true;
			texture_t *b1_texture = load_texture_from_rendered_text(
									ESC_MENU_TEXT1, COLOR_WHITE);
			check_mem(b1_texture);
			texture_t *b2_texture = load_texture_from_rendered_text(
									ESC_MENU_TEXT2, COLOR_WHITE);
			check_mem(b2_texture);
			texture_t *b3_texture = load_texture_from_rendered_text(
									ESC_MENU_TEXT3, COLOR_WHITE);
			check_mem(b3_texture);

			player_1 = create_button(ESC_MENU_BUTTON_X, ESC_MENU_BUTTON1_Y, 
									 ESC_MENU_BUTTON_W, ESC_MENU_BUTTON_H, 
									 true, true, b1_texture);
			check_mem(player_1);
			player_2 = create_button(ESC_MENU_BUTTON_X, ESC_MENU_BUTTON2_Y, 
									 ESC_MENU_BUTTON_W, ESC_MENU_BUTTON_H, 
									 true, true, b2_texture);
			check_mem(player_2);
			player_3 = create_button(ESC_MENU_BUTTON_X, ESC_MENU_BUTTON3_Y, 
									 ESC_MENU_BUTTON_W, ESC_MENU_BUTTON_H, 
									 true, true, b3_texture);
			check_mem(player_3);
		}
		
		
		bt_draw_button(player_1);
		bt_draw_button(player_2);
		bt_draw_button(player_3);

		if (true == check_mouse_click_in_button(player_1)){
			player_chosen = true;
			toggle_escape_menu();
		} else if (true == check_mouse_click_in_button(player_2)){
			player_chosen = true;
		} else if (true == check_mouse_click_in_button(player_3)){
			player_chosen = true;
			set_quit_game();
		}
		if (player_chosen == true){
			bt_destroy_button(player_1);
			bt_destroy_button(player_2);
			bt_destroy_button(player_3);
			buttons_created = false;
		}
	}	
	error:	
	return;
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
	char *text_to_print;
	int result_box_height;
	if (win_check == true){
		result_box_height = get_wrapped_text_height(RES_BOX_W, 
												  	TEXT_BOX_HEIGHT,
												  	win_text);
		text_to_print = win_text;
	} else {
		result_box_height = get_wrapped_text_height(RES_BOX_W, 
												  	TEXT_BOX_HEIGHT,
												  	lose_text);
		text_to_print = lose_text;

	}
	dw_draw_filled_rectangle(RES_BOX_X, RES_BOX_Y, RES_BOX_W, RES_BOX_H, 
	               			 COLOR_BLACK, COLOR_WHITE);
	dw_draw_text_fits_width(RES_BOX_TEXT_X, RES_BOX_TEXT_Y, RES_BOX_TEXT_W, 
							COLOR_WHITE, text_to_print);
	
	static bool buttons_created = false;
	static button_t *ret;
	static button_t *con;
	bool button_pressed = false;
	int action_selected = NO_BUTTON_PRESSED;

	if (buttons_created == false){
		buttons_created = true;
		texture_t *ret_texture = load_texture_from_rendered_text(
								 WIN_MENU_TEXT1, COLOR_WHITE);
		check_mem(ret_texture);
		texture_t *con_texture = load_texture_from_rendered_text(
								 WIN_MENU_TEXT2, COLOR_WHITE);
		check_mem(con_texture);

		ret = create_button(WIN_MENU_BUTTON1_X, WIN_MENU_BUTTON_Y, 
							WIN_MENU_BUTTON1_W, WIN_MENU_BUTTON_H, 
							true, true, ret_texture);
		check_mem(ret);
		con = create_button(WIN_MENU_BUTTON2_X, WIN_MENU_BUTTON_Y, 
							WIN_MENU_BUTTON2_W, WIN_MENU_BUTTON_H, 
								 true, true, con_texture);
	} 
	bt_draw_button(ret);
	bt_draw_button(con);

		if (check_mouse_click_in_button(ret) == true){
			button_pressed = true;
			action_selected = BACK_BUTTON_PRESSED;
		} else if (check_mouse_click_in_button(con) == true){
			button_pressed = true;
			action_selected = CONT_BUTTON_PRESSED;
		} 
		if (button_pressed == true){
			bt_destroy_button(ret);
			bt_destroy_button(con);
			buttons_created = false;
		}
	
	error:

	return action_selected;
}


/* Function: display_level_inputs
 * This function receives the coordinates and the list of inputs from 
 * a level in order to display it.
 * 	
 * Arguments:
 *	x: x coordinate where the list will be located.
 *  y: y coordinate whre the list will be located.
 *	l: list with the inputs
 *
 * Return:
 *	void
 */
void display_level_inputs(int x, int y, List *l)
{
	assert(NULL != l && "Invalid list with value NULL");
	// The rectangle that contains the numbers from the input buffer
	draw_rectangle(x, y, 100, 250, COLOR_WHITE);

	//Display the numbers contained in the list

	int first_input_element_x = x + 40;
	int first_input_element_y = y + 20;

	LIST_FOREACH(l, first, next, cur){

		char *c = number_to_string(*(int *)cur->value);
		draw_text(first_input_element_x, first_input_element_y, 
				  M_SCALE_FACTOR, COLOR_WHITE, c);
		first_input_element_y += 50;
		free(c);
	}
}


