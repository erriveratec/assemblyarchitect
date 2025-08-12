#include <stdlib.h>
#include <stdio.h>
#include "stage_buttons_sb.h"
#include "button_bt.h"
#include "dimensions.h"
#include "code_window_cw.h"
#include "assert.h"


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
	assert(member >= MEMBER_X && member <= MEMBER_H &&  "Member is incorrect");

	int return_value;
	switch (member){
		
		case MEMBER_X:
			return_value = play->x;
			break;
		case MEMBER_Y:
			return_value = play->y;
			break;
		case MEMBER_W:
			return_value = play->w;
			break;
		case MEMBER_H:
			return_value = play->h;
			break;
		default:
			return_value = INVALID_MEMBER;
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
	int y_final;
	if (code_size == CW_EMPTY){
		y_final = STAGE_BUTTON_HIDDEN_Y;
	} else {
		y_final = STAGE_BUTTON_Y;
	}

	int delta = STAGE_BUTTONS_MOVEMENT_DELTA;

	if (stop->y < y_final){
		stop->y += delta;
		step_back->y += delta;
		play->y += delta;
		step_forward->y += delta;
	} else if (stop->y > y_final){
		stop->y -= delta;
		step_back->y -= delta;
		play->y -= delta;
		step_forward->y -= delta;
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

/* Function: sb_initialize_return_buttons
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
void sb_initialize_return_button()
{
	ret_button = malloc(sizeof(button_t));
	ret_button = create_button(RET_BUTTON_X, RET_BUTTON_Y, RET_BUTTON_W, 
								  RET_BUTTON_H, true, true, return_button);
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
	stop = malloc(sizeof(button_t));
	stop = create_button(STAGE_BUTTON_X, STAGE_BUTTON_HIDDEN_Y,STAGE_BUTTON_W, 
						 STAGE_BUTTON_H, true, false, stop_button);

	step_back = malloc(sizeof(button_t));
	step_back = create_button(STAGE_BUTTON_X + STAGE_BUTTON_W + BUTTONS_SPACE, 
							  STAGE_BUTTON_HIDDEN_Y, STAGE_BUTTON_W, 
							  STAGE_BUTTON_H, true, false, step_back_button);

	play = malloc(sizeof(button_t));
	play = create_button(STAGE_BUTTON_X + 2*STAGE_BUTTON_W + 2*BUTTONS_SPACE, 
						 STAGE_BUTTON_HIDDEN_Y, STAGE_BUTTON_W, STAGE_BUTTON_H, 
						 true, false, play_button);

	step_forward = malloc(sizeof(button_t));
	step_forward = create_button(STAGE_BUTTON_X + 3*STAGE_BUTTON_W + 
								 3*BUTTONS_SPACE, STAGE_BUTTON_HIDDEN_Y, 
								 STAGE_BUTTON_W, STAGE_BUTTON_H, true, false, 
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


