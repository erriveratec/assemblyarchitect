#include <stdlib.h>
#include <stdio.h>
#include "stage_buttons.h"
#include "button_bt.h"
#include "dimensions.h"

#define EMPTY 0

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
	if (code_size == EMPTY){
		y_final = STAGE_BUTTON_HIDDEN_Y;
	} else {
		y_final = STAGE_BUTTON_Y;
	}

	int delta = get_movement_delta(stop->y, 
	 			y_final, MOVEMENT_DELTA);	

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

/* Function: draw_stage_buttons
 * -------------------------------------
 * Arguments:
 * 	None.
 *
 * Return:
 *	Void.
 */
void draw_stage_buttons(int code_size)
{
	adjust_stage_buttons_position(code_size);
	bt_draw_button(stop);
	bt_draw_button(step_back);
	bt_draw_button(play);
	bt_draw_button(step_forward);
}

/* Function: draw_return_button
 * ----------------------------------------------------------------------------
 * This function draws the return button that will be shown on several displays
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	Void.
 */
void draw_return_button()
{
	bt_draw_button(ret_button);
}

/* Function: initialize_stage_buttons
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
void initialize_return_button()
{
	ret_button = malloc(sizeof(button_t));
	ret_button = create_button(RET_BUTTON_X, RET_BUTTON_Y, RET_BUTTON_W, 
								  RET_BUTTON_H, false, true, return_button);
	return;
}
/* Function: initialize_stage_buttons
 * -----------------------------------------------------------------------------
 * Arguments:
 * 	None.
 *
 * Return:
 *	Void.
 */
void initialize_stage_buttons()
{
	stop = malloc(sizeof(button_t));
	stop = create_button(STAGE_BUTTON_X, STAGE_BUTTON_HIDDEN_Y,STAGE_BUTTON_W, 
						 STAGE_BUTTON_H, false, false, stop_button);

	step_back = malloc(sizeof(button_t));
	step_back = create_button(STAGE_BUTTON_X + STAGE_BUTTON_W + BUTTONS_SPACE, 
							  STAGE_BUTTON_HIDDEN_Y, STAGE_BUTTON_W, 
							  STAGE_BUTTON_H, false, false, step_back_button);

	play = malloc(sizeof(button_t));
	play = create_button(STAGE_BUTTON_X + 2*STAGE_BUTTON_W + 2*BUTTONS_SPACE, 
						 STAGE_BUTTON_HIDDEN_Y, STAGE_BUTTON_W, STAGE_BUTTON_H, 
						 false, false, play_button);

	step_forward = malloc(sizeof(button_t));
	step_forward = create_button(STAGE_BUTTON_X + 3*STAGE_BUTTON_W + 
								 3*BUTTONS_SPACE, STAGE_BUTTON_HIDDEN_Y, 
								 STAGE_BUTTON_W, STAGE_BUTTON_H, false, false, 
								 step_forward_button);
	return;
}

/* Function: check_clicked_ret_button
 * -----------------------------------------------------------------------------
 * This function verifies if the player has clicked on the return button
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	true if button clicked, false if otherwise.
 */
bool check_clicked_ret_button()
{
	int ret = false;

	if (check_mouse_click_in_button(ret_button) == true && 
		ret_button->active == false) {
		ret_button->active = true;
		ret = true;
	}
	if (ms_check_mouse_left_released() == true){
		ret_button->active = false;	
	}

	return ret;
}

/* Function: check_clicked_stage_button
 * -------------------------------------
 * Arguments:
 * 	None.
 *
 * Return:
 *	true if button clicked, false if otherwise.
 */
bool check_clicked_stage_button()
{
	int ret = false;

	if (check_mouse_click_in_button(stop) == true  ||
		check_mouse_click_in_button(step_back) == true ||
		check_mouse_click_in_button(play) == true ||
		check_mouse_click_in_button(step_forward) == true){
		ret = true;
	}

	return ret;
}

/* Function: check_released_in_stage_button
 * -------------------------------------
 * Arguments:
 * 	None.
 *
 * Return:
 *	true if released in stage button, false if otherwise.
 */
bool check_released_in_stage_button()
{
	int ret = false;

	if (check_mouse_click_in_button(stop) == true  ||
		check_mouse_click_in_button(step_back) == true ||
		check_mouse_click_in_button(play) == true ||
		check_mouse_click_in_button(step_forward) == true){
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
	
	if (check_mouse_click_in_button(stop) == true){
		clicked_button = STOP;
	} else if (check_mouse_click_in_button(step_back) == true){
		clicked_button = BACKWARD;
	} else if (check_mouse_click_in_button(play) == true){
		clicked_button = PLAY;
	} else if (check_mouse_click_in_button(step_forward) 
			   == true){
		clicked_button = FORWARD;
	}
	return clicked_button;
}


