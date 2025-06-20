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


typedef struct stage_buttons_t {
	button_t *stop;
	button_t *step_back;
	button_t *play;
	button_t *step_forward;
} stage_buttons_t;

stage_buttons_t *stage_buttons = NULL;


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

	int delta = get_movement_delta(stage_buttons->stop->y, 
	 			y_final, MOVEMENT_DELTA);	

	if (stage_buttons->stop->y < y_final){
		stage_buttons->stop->y += delta;
		stage_buttons->step_back->y += delta;
		stage_buttons->play->y += delta;
		stage_buttons->step_forward->y += delta;
	} else if (stage_buttons->stop->y > y_final){
		stage_buttons->stop->y -= delta;
		stage_buttons->step_back->y -= delta;
		stage_buttons->play->y -= delta;
		stage_buttons->step_forward->y -= delta;
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
	bt_draw_button(stage_buttons->stop);
	bt_draw_button(stage_buttons->step_back);
	bt_draw_button(stage_buttons->play);
	bt_draw_button(stage_buttons->step_forward);
}

/* Function: initialize_stage_levels
 * -------------------------------------
 * Arguments:
 * 	None.
 *
 * Return:
 *	Void.
 */
void initialize_stage_buttons()
{
	stage_buttons = malloc(sizeof(stage_buttons_t));

	stage_buttons->stop = create_button(STAGE_BUTTON_X, STAGE_BUTTON_HIDDEN_Y,

						  STAGE_BUTTON_W, STAGE_BUTTON_H, false, false, 
						  stop_button);

	stage_buttons->step_back = create_button(STAGE_BUTTON_X + STAGE_BUTTON_W + 
							   BUTTONS_SPACE, STAGE_BUTTON_HIDDEN_Y, 
							   STAGE_BUTTON_W, STAGE_BUTTON_H, false, false, 
							   step_back_button);

	stage_buttons->play = create_button(STAGE_BUTTON_X + 2*STAGE_BUTTON_W + 
					      2*BUTTONS_SPACE, STAGE_BUTTON_HIDDEN_Y, 
						  STAGE_BUTTON_W, STAGE_BUTTON_H, false, false, 
						  play_button);

	stage_buttons->step_forward = create_button(STAGE_BUTTON_X + 
								  3*STAGE_BUTTON_W + 3*BUTTONS_SPACE, 
								  STAGE_BUTTON_HIDDEN_Y, STAGE_BUTTON_W, 
								  STAGE_BUTTON_H, false, false, 
								  step_forward_button);
	return;
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

	if (check_mouse_click_in_button(stage_buttons->stop) == true  ||
		check_mouse_click_in_button(stage_buttons->step_back) == true ||
		check_mouse_click_in_button(stage_buttons->play) == true ||
		check_mouse_click_in_button(stage_buttons->step_forward) == true){
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

	if (check_mouse_click_in_button(stage_buttons->stop) == true  ||
		check_mouse_click_in_button(stage_buttons->step_back) == true ||
		check_mouse_click_in_button(stage_buttons->play) == true ||
		check_mouse_click_in_button(stage_buttons->step_forward) == true){
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
	
	if (check_mouse_click_in_button(stage_buttons->stop) == true){
		clicked_button = STOP;
	} else if (check_mouse_click_in_button(stage_buttons->step_back) == true){
		clicked_button = BACKWARD;
	} else if (check_mouse_click_in_button(stage_buttons->play) == true){
		clicked_button = PLAY;
	} else if (check_mouse_click_in_button(stage_buttons->step_forward) 
			   == true){
		clicked_button = FORWARD;
	}
	return clicked_button;
}


