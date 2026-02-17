#include <stdio.h>
#include <stdlib.h>
#include "mouse_ms.h"
#include <assert.h>
#include <stdbool.h>
#include <SDL.h>

typedef struct mouse_t{
	
	int x;
	int y;
	int scroll_x;
	int scroll_y;
	bool left_pressed;
	bool left_released;
	bool right_pressed;
	bool right_released;
	bool chord_pressed;
	bool chord_released;
} mouse_t;

static mouse_t mouse;

static void set_mouse_x(int x);
static void set_mouse_y(int y);
static void set_mouse_scroll_x(int x);
static void set_mouse_scroll_y(int y);
static void set_mouse_left_pressed(bool s);
static void set_mouse_left_released(bool s);
static void set_mouse_right_pressed(bool s);
static void set_mouse_right_released(bool s);
static void set_mouse_chord_pressed(bool s);
static void set_mouse_chord_released(bool s);


/* Function: ms_click_inside_rect
 * -----------------------------------------------------------------------------
 * Verifies if the mouse click whas done inside an area defined by a rect
 *
 * Arguments:
 *	r: The rectangle that will be set
 *
 * Return:
 *	true if the left button of the mouse is pressed, false if otherwise
 */
bool ms_click_inside_rect(SDL_Rect r)
{
	bool in = false;

	if (ms_left_pressed() == true){
		int x = ms_get_mouse_x();
		int y = ms_get_mouse_y();
		
		if (x > r.x && x < (r.x + r.w) && y > r.y && y < (r.y + r.h)){
			in = true;
		} 

	}

	return in;
}

/* Function: ms_left_pressed
 * -----------------------------------------------------------------------------
 * This functions verifies if the left button of the mouse is pressed.
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	true if the left button of the mouse is pressed, false if otherwise
 */
bool ms_left_pressed()
{
	return mouse.left_pressed;
}

/* Function: get_mouse_left_released
 * -----------------------------------------------------------------------------
 * This function is used for obtaining the status of mouse left released
 *
 * Arguments:
 *	none.
 *
 * Return:
 *	bool with the state of the button enable attribute.
 */
bool ms_left_released()
{
	return mouse.left_released;
}

/* Function: ms_right_pressed
 * -----------------------------------------------------------------------------
 * This function is used for obtaining the status of mouse right pressed
 *
 * Arguments:
 *	none.
 *
 * Return:
 *	bool with the state of the button enable attribute.
 */
bool ms_right_pressed()
{
	return mouse.right_pressed;
}

/* Function: ms_right_released
 * -----------------------------------------------------------------------------
 * This function is used for obtaining the status of mouse right released
 *
 * Arguments:
 *	none.
 *
 * Return:
 *	bool with the state of the button enable attribute.
 */
bool ms_right_released()
{
	return mouse.right_released;
}

/* Function: ms_chord_pressed
 * -----------------------------------------------------------------------------
 * This function is used for obtaining the status of mouse chord pressed
 *
 * Arguments:
 *	none.
 *
 * Return:
 *	bool with the state of the button enable attribute.
 */
bool ms_chord_pressed()
{
	return mouse.chord_pressed;
}

/* Function: ms_chord_released
 * -----------------------------------------------------------------------------
 * This function is used for obtaining the status of mouse chord released
 *
 * Arguments:
 *	none.
 *
 * Return:
 *	bool with the state of the button enable attribute.
 */
bool ms_chord_released()
{
	return mouse.chord_released;
}

/* Function: ms_init_mouse
 * -----------------------------------------------------------------------------
 * This function is initializes 
 *
 * Arguments:
 *	x: x coordinate of mouse;
 *
 * Return:
 *	void.
 */

void ms_init_mouse()
{
	set_mouse_x(0);
	set_mouse_y(0);
	set_mouse_left_pressed(false);
	set_mouse_left_released(false);

	return;
}

/* Function: set_mouse_scroll_x
 * -----------------------------------------------------------------------------
 * This function is used for setting the scroll of the x coordinate of the 
 * mouse.
 *
 * Arguments:
 *	x: x scroll of mouse;
 *
 * Return:
 *	void.
 */
static void set_mouse_scroll_x(int x)
{
	mouse.scroll_x = x;
	return;
}

/* Function: set_mouse_scroll_y
 * -----------------------------------------------------------------------------
 * This function is used for setting the scroll of the y coordinate of the 
 * mouse.
 *
 * Arguments:
 *	y: y scroll of mouse;
 *
 * Return:
 *	void.
 */
static void set_mouse_scroll_y(int y)
{
	mouse.scroll_y = y;
	return;
}

/* Function: set_mouse_x
 * -----------------------------------------------------------------------------
 * This function is used for setting the x coordinate of the mouse.
 *
 * Arguments:
 *	x: x coordinate of mouse;
 *
 * Return:
 *	void.
 */
static void set_mouse_x(int x)
{
	mouse.x = x;
	return;
}

/* Function: set_mouse_y
 * -----------------------------------------------------------------------------
 * This function is used for setting the y coordinate of the mouse.
 *
 * Arguments:
 *	y: y coordinate of mouse;
 *
 * Return:
 *	void.
 */
static void set_mouse_y(int y)
{
	mouse.y = y;
	return;
}

/* Function: set_mouse_left_pressed
 * -----------------------------------------------------------------------------
 * This function is used for setting the pressed attribute of the left button
 * of the mouse;
 *
 * Arguments:
 *	s: state that will be set for the left button;
 *
 * Return:
 *	void.
 */
static void set_mouse_left_pressed(bool s)
{
	mouse.left_pressed = s;
	return;
}

/* Function: set_mouse_left_released
 * -----------------------------------------------------------------------------
 * This function is used for setting the pressed attribute of the left button
 * of the mouse;
 *
 * Arguments:
 *	s: state that will be set for the left button.
 *
 * Return:
 *	void.
 */
static void set_mouse_left_released(bool s)
{
	mouse.left_released = s;
	return;
}

/* Function: set_mouse_right_pressed
 * -----------------------------------------------------------------------------
 * This function is used for setting the pressed attribute of the right button
 * of the mouse;
 *
 * Arguments:
 *	s: state that will be set for the right button;
 *
 * Return:
 *	void.
 */
static void set_mouse_right_pressed(bool s)
{
	mouse.right_pressed = s;
	return;
}

/* Function: set_mouse_chord_released
 * -----------------------------------------------------------------------------
 * This function is used for setting the pressed attribute of the chord button
 *
 * Arguments:
 *	s: state that will be set for the right button.
 *
 * Return:
 *	void.
 */
static void set_mouse_chord_released(bool s)
{
	mouse.chord_released = s;
	return;
}

/* Function: set_mouse_chord_pressed
 * -----------------------------------------------------------------------------
 * This function is used for setting the pressed attribute of the chord button
 *
 * Arguments:
 *	s: state that will be set for the right button;
 *
 * Return:
 *	void.
 */
static void set_mouse_chord_pressed(bool s)
{
	mouse.chord_pressed = s;
	return;
}

/* Function: set_mouse_right_released
 * -----------------------------------------------------------------------------
 * This function is used for setting the pressed attribute of the right button
 * of the mouse;
 *
 * Arguments:
 *	s: state that will be set for the right button.
 *
 * Return:
 *	void.
 */
static void set_mouse_right_released(bool s)
{
	mouse.right_released = s;
	return;
}
/* Function: ms_clear_mouse_values
 * -----------------------------------------------------------------------------
 * This function is used for clearing the values not used in the mouse for next
 * game cycles
 *
 * Arguments:
 *
 * Return:
 *	void.
 */
void ms_clear_mouse_values()
{
	set_mouse_scroll_x(0);
	set_mouse_scroll_y(0);
	set_mouse_left_released(false);
	return;
}

/* Function: ms_reset_mouse_values
 * -----------------------------------------------------------------------------
 * This function is used for clearing the values not used in the mouse for next
 * game cycles
 *
 * Arguments:
 *
 * Return:
 *	void.
 */
void ms_reset_mouse_values()
{
	set_mouse_x(0);
	set_mouse_y(0);
	set_mouse_scroll_x(0);
	set_mouse_scroll_y(0);
	set_mouse_left_pressed(false);
	set_mouse_left_released(false);
	set_mouse_right_pressed(false);
	set_mouse_right_released(false);
	set_mouse_chord_pressed(false);
	set_mouse_chord_released(false);
	return;
}

/* Function: ms_get_mouse_x
 * -----------------------------------------------------------------------------
 * This functions returns the x coordinate of the mouse object
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	The x coordinate of the mouse.
 */
int ms_get_mouse_x()
{
	return mouse.x;
}

/* Function: ms_get_mouse_y
 * -----------------------------------------------------------------------------
 * This functions returns the y coordinate of the mouse object
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	The y coordinate of the mouse.
 */
int ms_get_mouse_y()
{
	return mouse.y;
}

/* Function: ms_get_mouse_scroll_y
 * -----------------------------------------------------------------------------
 * This functions returns the y coordinate of the mouse object
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	The y coordinate of the mouse.
 */
int ms_get_mouse_scroll_y()
{
	int cur_y = mouse.scroll_y;
	set_mouse_scroll_y(0);
	return cur_y;
}




/* Function: ms_mouse_button_handler
 * -----------------------------------------------------------------------------
 * This functions handles the mouse action from the main
 *
 * Arguments:
 *	e: The event union struct used for the handling of the mouse.
 *
 * Return:
 *	Void
 */
void ms_mouse_button_handler(SDL_Event e)
{
	set_mouse_x(e.button.x);
	set_mouse_y(e.button.y);

	switch (e.button.button){
		case SDL_BUTTON_LEFT:
			if (SDL_PRESSED == e.button.state){
				set_mouse_left_pressed(true);
				set_mouse_left_released(false);
			} else if (SDL_RELEASED == e.button.state){
				set_mouse_left_pressed(false);
				set_mouse_left_released(true);
			}
			break;
		case SDL_BUTTON_RIGHT:
			if (SDL_PRESSED == e.button.state){
				set_mouse_right_pressed(true);
				set_mouse_right_released(false);
			} else if (SDL_RELEASED == e.button.state){
				set_mouse_right_pressed(false);
				set_mouse_right_released(true);
			}
			break;
		default:
			mouse.left_pressed = false;
			mouse.left_released = false;
			mouse.right_pressed = false;
			mouse.right_released = false;
			break;
	}	
	if (ms_left_pressed() == true && ms_right_pressed()){
		set_mouse_chord_pressed(true);
	}
	if (ms_left_released() == true && ms_right_released() == true){
		set_mouse_chord_released(true);
	}
	return;
}

/* Function: ms_mouse_motion_handler
 * -----------------------------------------------------------------------------
 * This functions handles the mouse motion for movement
 *
 * Arguments:
 *
 * Return:
 *	Void
 */
void ms_mouse_motion_handler(SDL_Event e)
{
	
	set_mouse_x(e.motion.x);
	set_mouse_y(e.motion.y);

	return;
}

/* Function: ms_mouse_wheel_handler
 * -----------------------------------------------------------------------------
 * This functions handles the mouse motion for movement
 *
 * Arguments:
 *
 * Return:
 *	Void
 */
void ms_mouse_wheel_handler(SDL_Event e)
{
	
	set_mouse_scroll_x(e.wheel.x);
	set_mouse_scroll_y(e.wheel.y);

	return;
}

