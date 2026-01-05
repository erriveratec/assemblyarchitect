#include <stdlib.h>
#include <stdio.h>
#include "stage_buttons_sb.h"
#include "button_bt.h"
#include "dimensions_dm.h"
#include "code_window_cw.h"
#include "assert.h"
#include "dbg.h"

#define ESC_MENU_TEXT1 "Return to Game"
#define ESC_MENU_TEXT2 "Toggle Full Screen"
#define ESC_MENU_TEXT3 "Exit Game"

#define RST_MENU_TEXT0 "Do you want to reset the current level?"
#define RST_MENU_TEXT1 "NO"
#define RST_MENU_TEXT2 "YES"

#define STAGE_BUTTONS_MOVEMENT_DELTA 10

bool g_escape_menu = false;
bool g_rst_menu = false;
bool g_quit = false;

bool g_step_btns_avail = false;

texture_t *stop_button = NULL;
texture_t *step_back_button = NULL;
texture_t *play_button = NULL;
texture_t *step_forward_button = NULL;
texture_t *return_button = NULL;
texture_t *reset_button = NULL;

iface_btn_t *stop;
iface_btn_t *step_back;
iface_btn_t *play;
iface_btn_t *step_forward;
iface_btn_t *ret_btn;
iface_btn_t *rst_btn;

texture_t *g_escape_b1_texture = NULL;
texture_t *g_escape_b2_texture = NULL;
texture_t *g_escape_b3_texture = NULL;

iface_btn_t *g_escape_b1;
iface_btn_t *g_escape_b2;
iface_btn_t *g_escape_b3;

texture_array_t *g_rst_menu_text = NULL;
texture_t *g_rst_b1_texture = NULL;
texture_t *g_rst_b2_texture = NULL;

iface_btn_t *g_rst_b1;
iface_btn_t *g_rst_b2;

/* Function: sb_set_step_btns_avail
 * ----------------------------------------------------------------------------
 * Sets the global variable that determines if the step buttons will be shown
 * as a part of a level.
 *
 * Arguments:
 * 	state: state that the g_step_btns_avail variable will be set
 *
 * Return:
 *	Void.	
 */
void sb_set_step_btns_avail(bool state)
{
	g_step_btns_avail = true;
}

/* Function: sb_display_rst_menu
 * -----------------------------------------------------------------------------
 * Displays the rst menu that is show when the reset button is pressed.
 * 
 * Arguments:
 *	show_menu: a variable that determines if the menu will be shown.
 *
 * Return:
 *	Void.
 */
void sb_display_rst_menu(bool show_menu)
{
	if (show_menu == true){
		SDL_Rect r = dm_get_center_screen_box();
		dw_draw_iface_box(r);

		bt_draw_iface_btn(g_rst_b1);
		bt_draw_iface_btn(g_rst_b2);

		if (bt_chk_mouse_rel_iface_btn(g_rst_b1) == true){
			sb_set_rst_menu(false);
		} else if (bt_chk_mouse_rel_iface_btn(g_rst_b2) == true){
			sb_set_rst_menu(false);
		} 	
	}
	return;
}

/* Function: sb_init_rst_menu
 * ----------------------------------------------------------------------------
 * Initializes the assets of the reset menu.
 *
 * Arguments:
 * 	Void.
 *
 * Return:
 *	Void.	
 */
void sb_init_rst_menu()
{
	g_rst_b1_texture = dw_create_text_texture(RST_MENU_TEXT1, C_WHITE);
	check_mem(g_rst_b1_texture);

	g_rst_b2_texture = dw_create_text_texture(RST_MENU_TEXT2, C_WHITE);
	check_mem(g_rst_b2_texture);
	
	SDL_Rect r = dm_get_rst_b1_box();
	g_rst_b1 = bt_create_iface_btn(r, g_rst_b1_texture, true);
	check_mem(g_rst_b1);

	r = dm_get_rst_b2_box();
	g_rst_b2 = bt_create_iface_btn(r, g_rst_b2_texture, true);
	check_mem(g_rst_b2);

	int h = dm_get_h_big_text();
	int w = dm_get_w_msg(dm_get_center_screen_box());
	g_rst_menu_text = dw_new_text_texture_by_h(w, h, C_BLACK, RST_MENU_TEXT0);
	
	error:
	return;
}

/* Function: sb_set_rst_menu
 * ----------------------------------------------------------------------------
 * Sets the status of the rst menu to be shown to the player
 *
 * Arguments:
 * 	state: state ot it the g_rst_menu variable will be set.
 *
 * Return:
 *	Void.	
 */
void sb_set_rst_menu(bool state)
{
	g_rst_menu = state;
}

/* Function: sb_get_rst_menu_state
 * ----------------------------------------------------------------------------
 * This function return the boolean state of the rst menu.
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	Boolean with the state of the rst menu variable
 */
bool sb_chk_rst_menu_state()
{
	return g_rst_menu;
}

/* Function: sb_init_escape_menu
 * ----------------------------------------------------------------------------
 * Initializes all the assets of the escape menu.
 *
 * Arguments:
 * 	Void.
 *
 * Return:
 *	Void.	
 */
void sb_init_escape_menu()
{
	g_escape_b1_texture = dw_create_text_texture(ESC_MENU_TEXT1, C_WHITE);
	check_mem(g_escape_b1_texture);

	g_escape_b2_texture = dw_create_text_texture(ESC_MENU_TEXT2, C_WHITE);
	check_mem(g_escape_b2_texture);

	g_escape_b3_texture = dw_create_text_texture(ESC_MENU_TEXT3, C_WHITE);
	check_mem(g_escape_b3_texture);
	
	SDL_Rect r = dm_get_escape_b1_box();
	g_escape_b1 = bt_create_iface_btn(r, g_escape_b1_texture, true);
	check_mem(g_escape_b1);

	r = dm_get_escape_b2_box();
	g_escape_b2 = bt_create_iface_btn(r, g_escape_b2_texture, true);
	check_mem(g_escape_b2);
	
	r = dm_get_escape_b3_box();
	g_escape_b3 = bt_create_iface_btn(r, g_escape_b3_texture, true);
	check_mem(g_escape_b3);
	error:
	return;
}

/* Function: sb_display_escape_menu
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
void sb_display_escape_menu(bool show_menu)
{
	if (show_menu == true){
		
		SDL_Rect r = dm_get_center_screen_box();
		dw_draw_iface_box(r);

		bt_draw_iface_btn(g_escape_b1);
		bt_draw_iface_btn(g_escape_b2);
		bt_draw_iface_btn(g_escape_b3);

		if (bt_chk_mouse_rel_iface_btn(g_escape_b1) == true){
			toggle_escape_menu();
		} else if (bt_chk_mouse_rel_iface_btn(g_escape_b2) == true){
			puts("Full screen must be implemented");	
		} else if (bt_chk_mouse_rel_iface_btn(g_escape_b3) == true){
			set_quit_game();
		}
	}	
	return;
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

/* Function: sb_get_escape_menu_state
 * ----------------------------------------------------------------------------
 * This function return the boolean state of the state.
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	Boolean with the state fo the escape_menu variable
 */
bool sb_get_escape_menu_state()
{
	return g_escape_menu;
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


/* Function: cw_get_play_button_member
 * -----------------------------------------------------------------------------
 * This function returns the solicited button value from the play button
 *
 * Arguments:
 * member: The specific member that is being requested.
 *
 * Return:
 *	The accessed member.
 */
int sb_get_play_button_member(int member)
{
	assert(member > MEMBER_MIN && member < MEMBER_MAX && "Invalid member");

	int return_value;
	switch (member){
		case MEMBER_X:
			return_value = play->r.x;
			break;
		case MEMBER_Y:
			return_value = play->r.y;
			break;
		case MEMBER_W:
			return_value = play->r.w;
			break;
		case MEMBER_H:
			return_value = play->r.h;
			break;
	}
	return return_value;
}

static bool sb_check_released_in_stage_button(); // not used

/* Function: adjust_stage_buttons_position
 * -------------------------------------
 * Arguments:
 * 	status:
 *
 * Return:
 *	Void.
 */
void adjust_stage_buttons_position(int code_size)
{
	int hidden_y = dm_get_y_hidden_stage_buttons();
	SDL_Rect sb = dm_get_stage_buttons();
	int y_final;
	if (code_size == CW_EMPTY){
		y_final = hidden_y;
	} else {
		y_final = sb.y;
	}

	int delta = STAGE_BUTTONS_MOVEMENT_DELTA;

	if (stop->r.y < y_final){
		stop->r.y += delta;
		step_back->r.y += delta;
		play->r.y += delta;
		step_forward->r.y += delta;
	} else if (stop->r.y > y_final){
		stop->r.y -= delta;
		step_back->r.y -= delta;
		play->r.y -= delta;
		step_forward->r.y -= delta;
	}

}

/* Function: sb_draw_stage_buttons
 * -------------------------------------
 * Arguments:
 * 	None.
 *
 * Return:
 *	Void.
 */
void sb_draw_stage_buttons(int code_size)
{
	adjust_stage_buttons_position(code_size);
	bt_draw_iface_btn(stop);
	bt_draw_iface_btn(play);
	if (g_step_btns_avail == true){
		bt_draw_iface_btn(step_back);
		bt_draw_iface_btn(step_forward);
	}
}


/* Function: sb_draw_rst_btn
 * ----------------------------------------------------------------------------
 * This function draws the reset button that will be shown to clean the code
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	Void.
 */
void sb_draw_rst_btn()
{
	bt_draw_iface_btn(rst_btn);
}

/* Function: sb_init_rst_btn
 * -----------------------------------------------------------------------------
 * This function initialises the reset button of the stage.
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	Void.
 */
void sb_init_rst_btn()
{
	if (rst_btn == NULL){
		SDL_Rect r = dm_get_rst_btn_box();
		rst_btn = malloc(sizeof(iface_btn_t));
		rst_btn = bt_create_iface_btn(r, reset_button, true);
	}
	return;
}

/* Function: sb_chk_click_rst_btn
 * -----------------------------------------------------------------------------
 * Verifies if the reset button has been clicked
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	true if button clicked, false if otherwise.
 */
bool sb_chk_click_rst_btn()
{
	int rst = false;

	if (bt_chk_mouse_rel_iface_btn(rst_btn) == true) {
		rst = true;
	}
	return rst;
}

/* Function: sb_draw_return_button
 * ----------------------------------------------------------------------------
 * This function draws the return button that will be shown on several displays
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	Void.
 */
void sb_draw_return_button()
{
	bt_draw_iface_btn(ret_btn);
}

/* Function: sb_init_return_buttons
 * -----------------------------------------------------------------------------
 * This function initialises the return button that is used and present in 
 * several stages.
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	Void.
 */
void sb_init_return_button()
{
	if (ret_btn == NULL){
		SDL_Rect r = dm_get_return_button_box();
		ret_btn = malloc(sizeof(iface_btn_t));
		ret_btn = bt_create_iface_btn(r, return_button, true);
	}
	return;
}

/* Function: sb_initialize_stage_buttons
 * -----------------------------------------------------------------------------
 * Arguments:
 * 	None.
 *
 * Return:
 *	Void.
 */
void sb_initialize_stage_buttons()
{
	SDL_Rect sb = dm_get_stage_buttons();
	int hidden_y = dm_get_y_hidden_stage_buttons();
	SDL_Rect r0 = {.x = sb.x, .y = hidden_y, .w = sb.w, .h = sb.h};
	
	stop = malloc(sizeof(iface_btn_t));
	stop = bt_create_iface_btn(r0, stop_button, true);

	int space =  dm_get_ofs_space_stage_buttons();
	SDL_Rect r1 = {.x = sb.x + sb.w + space,.y = hidden_y, .w = sb.w, 
																	 .h = sb.h};
	step_back = malloc(sizeof(iface_btn_t));
	step_back = bt_create_iface_btn(r1, step_back_button, true);

	SDL_Rect r2 = {.x = sb.x + 2*sb.w + 2*space, .y = hidden_y, .w = sb.w, 
																	 .h = sb.h};
	play = malloc(sizeof(iface_btn_t));
	play = bt_create_iface_btn(r2, play_button, true);
	
	SDL_Rect r3 = {.x = sb.x + 3*sb.w + 3*space, .y = hidden_y, .w = sb.w, 
																	 .h = sb.h};
	step_forward = malloc(sizeof(iface_btn_t));
	step_forward = bt_create_iface_btn(r3, step_forward_button, true);
	return;
}

/* Function: sb_check_clicked_ret_button
 * -----------------------------------------------------------------------------
 * This function verifies if the player has clicked on the return button
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	true if button clicked, false if otherwise.
 */
bool sb_check_clicked_ret_button()
{
	int ret = false;

	if (bt_chk_mouse_rel_iface_btn(ret_btn) == true) {
		ret = true;
	}
	return ret;
}

/* Function: sb_check_clicked_stage_button
 * ----------------------------------------------------------------------------
 * Verifies if a stage button was clicked
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	true if button clicked, false if otherwise.
 */
bool sb_check_clicked_stage_button()
{
	int ret = false;

	if (bt_chk_mouse_click_iface_btn(stop) == true ||
		bt_chk_mouse_click_iface_btn(play) == true){
		ret = true;
	}
	else if (g_step_btns_avail == true && 
			 (bt_chk_mouse_click_iface_btn(step_back) == true ||
			  bt_chk_mouse_click_iface_btn(step_forward) == true)){
		ret = true;	
	}
	return ret;
}


/* Function: identify_clicked_stage_button
 * -------------------------------------
 * Arguments:
 * 	None.
 *
 * Return:
 *	true if released in stage button, false if otherwise.
 */
int identify_clicked_stage_button()
{
	int clicked_button = INVALID;
	
	if (bt_chk_mouse_click_iface_btn(stop) == true){
		clicked_button = STOP;
	} else if (bt_chk_mouse_click_iface_btn(step_back) == true && 
			   g_step_btns_avail == true){
		clicked_button = BACKWARD;
	} else if (bt_chk_mouse_click_iface_btn(play) == true){
		clicked_button = PLAY;
	} else if (bt_chk_mouse_click_iface_btn(step_forward) == true &&
			   g_step_btns_avail == true){
		clicked_button = FORWARD;
	}
	return clicked_button;
}


