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

#define STAGE_BUTTONS_MOVEMENT_DELTA 10

bool g_escape_menu = false;
bool g_quit = false;

texture_t *stop_button = NULL;
texture_t *step_back_button = NULL;
texture_t *play_button = NULL;
texture_t *step_forward_button = NULL;
texture_t *return_button = NULL;

button_t *stop;
button_t *step_back;
button_t *play;
button_t *step_forward;
button_t *ret_button;

texture_t *g_escape_b1_texture = NULL;
texture_t *g_escape_b2_texture = NULL;
texture_t *g_escape_b3_texture = NULL;

button_t *g_escape_b1;
button_t *g_escape_b2;
button_t *g_escape_b3;

/* Function: init_escape_menu
 * ----------------------------------------------------------------------------
 * Initializes all the assets of the escape menu.
 *
 * Arguments:
 * 	Void.
 *
 * Return:
 *	Void.	
 */
void init_escape_menu()
{
	g_escape_b1_texture = dw_create_text_texture(ESC_MENU_TEXT1, C_BLACK);
	check_mem(g_escape_b1_texture);

	g_escape_b2_texture = dw_create_text_texture(ESC_MENU_TEXT2, C_BLACK);
	check_mem(g_escape_b2_texture);

	g_escape_b3_texture = dw_create_text_texture(ESC_MENU_TEXT3, C_BLACK);
	check_mem(g_escape_b3_texture);
	
	SDL_Rect r = dm_get_escape_b1_box();
	g_escape_b1 = bt_create_button(r, true, true, true, C_WHITE, C_BLACK,
														   g_escape_b1_texture);
	check_mem(g_escape_b1);

	r = dm_get_escape_b2_box();
	g_escape_b2 = bt_create_button(r, true, true, true, C_WHITE, C_BLACK, 
														   g_escape_b2_texture);
	check_mem(g_escape_b2);
	
	r = dm_get_escape_b3_box();
	g_escape_b3 = bt_create_button(r, true, true, true, C_WHITE, C_BLACK, 
														   g_escape_b3_texture);
	check_mem(g_escape_b3);
	error:
	return;
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
void display_escape_menu(bool show_menu)
{

	if (show_menu == true){
		
		SDL_Rect r = dm_get_escape_menu_box();
		dw_draw_filled_rectangle(r, C_WHITE, C_BLACK);

		
		bt_draw_button(g_escape_b1);
		bt_draw_button(g_escape_b2);
		bt_draw_button(g_escape_b3);

		if (bt_check_mouse_click_button(g_escape_b1) == true){
			toggle_escape_menu();
		} else if (bt_check_mouse_click_button(g_escape_b2) == true){
			puts("Full screen must be implemented");	
		} else if (bt_check_mouse_click_button(g_escape_b3) == true){
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
bool get_escape_menu_state()
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
	bt_draw_button(stop);
	bt_draw_button(step_back);
	bt_draw_button(play);
	bt_draw_button(step_forward);
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
	bt_draw_button(ret_button);
}

/* Function: sb_init_return_buttons
 * -----------------------------------------------------------------------------
 * This function initialixses the return button that is used and present in 
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
	if (ret_button == NULL){
		SDL_SetTextureColorMod(return_button->texture, 0, 0, 0);
		SDL_Rect r = dm_get_return_button_box();
		ret_button = malloc(sizeof(button_t));
		ret_button = bt_create_button(r, true, true, true, C_WHITE, C_WHITE, 	
																 return_button);
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
	stop = malloc(sizeof(button_t));
	stop = bt_create_button(r0, true, false, false, C_BLACK, C_WHITE, 
																   stop_button);

	int space =  dm_get_ofs_space_stage_buttons();
	SDL_Rect r1 = {.x = sb.x + sb.w + space,.y = hidden_y, .w = sb.w, 
																	 .h = sb.h};
	step_back = malloc(sizeof(button_t));
	step_back = bt_create_button(r1, true, false, false, C_BLACK, C_WHITE, 
															  step_back_button);

	SDL_Rect r2 = {.x = sb.x + 2*sb.w + 2*space, .y = hidden_y, .w = sb.w, 
																	 .h = sb.h};
	play = malloc(sizeof(button_t));
	play = bt_create_button(r2, true, false, false, C_BLACK,C_WHITE, 
																   play_button);
	
	SDL_Rect r3 = {.x = sb.x + 3*sb.w + 3*space, .y = hidden_y, .w = sb.w, 
																	 .h = sb.h};
	step_forward = malloc(sizeof(button_t));
	step_forward = bt_create_button(r3, true, false, false, C_BLACK, C_WHITE,
								 						   step_forward_button);
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

	if (bt_check_mouse_click_button(ret_button) == true) {
		ret = true;
	}
	return ret;
}

/* Function: sb_check_clicked_stage_button
 * -------------------------------------
 * Arguments:
 * 	None.
 *
 * Return:
 *	true if button clicked, false if otherwise.
 */
bool sb_check_clicked_stage_button()
{
	int ret = false;

	if (bt_check_mouse_click_button(stop) == true  ||
		bt_check_mouse_click_button(step_back) == true ||
		bt_check_mouse_click_button(play) == true ||
		bt_check_mouse_click_button(step_forward) == true){
		ret = true;
	}

	return ret;
}

/* Function: sb_check_released_in_stage_button
 * -------------------------------------
 * Arguments:
 * 	None.
 *
 * Return:
 *	true if released in stage button, false if otherwise.
 */
static bool sb_check_released_in_stage_button()
{
	int ret = false;

	if (bt_check_mouse_click_button(stop) == true  ||
		bt_check_mouse_click_button(step_back) == true ||
		bt_check_mouse_click_button(play) == true ||
		bt_check_mouse_click_button(step_forward) == true){
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
	
	if (bt_check_mouse_click_button(stop) == true){
		clicked_button = STOP;
	} else if (bt_check_mouse_click_button(step_back) == true){
		clicked_button = BACKWARD;
	} else if (bt_check_mouse_click_button(play) == true){
		clicked_button = PLAY;
	} else if (bt_check_mouse_click_button(step_forward) 
			   == true){
		clicked_button = FORWARD;
	}
	return clicked_button;
}


