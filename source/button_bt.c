#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "button_bt.h"
#include "sdl_config.h"
#include "list.h"
#include "mouse_ms.h"


/* Function: create_button
 * -----------------------------------------------------------------------------
 * This function is used to create a button object, it receives as 
 * arguments the parameters of the button. The scale of the texture is 
 * calculated based on the parameters of widht and height
 * 	
 * Arguments:
 *	x: position x
 *	y: position y
 *	w: width
 *	h: height
 *	act: boolean with info if the button is in a stae.
 *  rect: boolean that determines if a rectangle is draw around the button.	
 *  in: inside color of the button. 
 *  out: outside color of the button.
 *
 * Return:
 *	Pointer to the button object
 */
button_t *create_button(int x, int y, int w, int h, bool act, bool rect, 
 						texture_t *t)
{
	button_t *new_button = malloc(sizeof(button_t));

	new_button->x = x;
	new_button->y = y;
	new_button->w = w;
	new_button->h = h;
	new_button->active = active;
	new_button->rectangle = rectangle;

	float scale_w = (float)w/texture->w;
	float scale_h = (float)h/texture->h;
	new_button->texture = texture;
	
	return new_button;
}

/* Function: bt_draw_button
 * -----------------------------------------------------------------------------
 * This function draws the image of a given button using the according
 * scale factor of the image to fit on the button. The texture is centered
 * 	
 * Arguments:
 *	button: The button that is going to be displayed
 *
 * Return:
 *	Void
 */
void bt_draw_button(button_t *b)
{
	assert(b != NULL && "The button pointer is NULL");
	
	int status = SUCCESS;	

	dw_draw_filled_rectangle(b->x, b->y, b->w, b->h, b->in, b->in);
	if (b->rectangle == true){
		dw_draw_rectangle(b->x, b->y, b->w, b->h, b->out);
	}
	
	float scale_w = (float)b->w/b->texture->w;
	float scale_h = (float)b->h/b->texture->h;
	if (scale_w < scale_h){
		int y = b->y + (b->h - b->texture->h*scale_w)/2;
		dw_draw_texture_fits_width(b->x, y, b->w, b->texture);
	} else {
		int x = b->x + (b->w - b->texture->w*scale_h)/2;
		dw_draw_texture_fits_height(x, b->y, b->h, b->texture);
	}
	assert(status != FAIL && "The texture could not be drawn");
}

/* Function: assign_button_parameters
 * This function receive a function and modifies its parameters
 * 
 * Arguments:
 *	x: The x coordinate of the button.
 *  y: The y coordiante of the button.
 *  w: The width of the button.
 *	h: The height of the button.
 * 	b: The button pointer of the object.
 *	
 * Return:
 *	void
 *
 */
void assign_button_parameters(int x, int y, int w, int h, button_t *b)
{
	b->x = x;
	b->y = y;
	b->w = w;
	b->h = h;
}

/* Function: bt_check_mouse_click_in_button
 *------------------------------------------------------------------------------
 * This function verifies is the mouse has clicked insiden a give button.
 *
 * Arguments:
 * 	button: the button to be verified
 *	
 * Return:
 *	true if the mouse clicked inside the button
 *	false if otherwise
 *
*/
bool bt_check_mouse_click_button(button_t *button)
{
	assert(NULL != button && "The button pointer cannot be NULL");

	int mouse_x = ms_get_mouse_x();
	int mouse_y = ms_get_mouse_y();
	int mouse_left_pressed = ms_chk_mouse_left_pressed();

	bool clicked_in_button;
	if (mouse_left_pressed == false || button->active == false){
		clicked_in_button = false;
	} else if (mouse_x > button->x && mouse_x < (button->x + button->w) &&
		mouse_y > button->y && mouse_y < (button->y + button->h)){
		clicked_in_button = true;
	} else {
		clicked_in_button = false;
	} 
	return clicked_in_button;
}

/* Function: check_mouse_released_button
 *------------------------------------------------------------------------------
 * Arguments:
 * 	button: the button to be verified
 *	
 * Return:
 *	true if the mouse clicked inside the button
 *	false if otherwise
*/
bool bt_check_mouse_released_button(button_t *button)
{
	assert(button != NULL && "The button pointer cannot be NULL");

	int mouse_x = ms_get_mouse_x();
	int mouse_y = ms_get_mouse_y();
	int mouse_left_released = ms_chk_mouse_left_released();
	
	if (mouse_left_released == false){
		return false;
	}

	if (mouse_x > button->x && mouse_x < (button->x + button->w) &&
		mouse_y > button->y && mouse_y < (button->y + button->h)) {
		return true;
	} else {
		return false;
	} 
}


/* Function: bt_copy_button
 * This function duplicates a button object of a pointer
 * it creates different pointers that can be used on a list
 *
 * Arguments:
 * 	b: the button to be copied
 *	
 * Return:
 *	The new button object 
 *
 */
button_t *bt_copy_button(button_t *b)
{
	button_t *copied_button = malloc(sizeof(button_t));
	copied_button->x = b->x;
	copied_button->y = b->y;
	copied_button->w = b->w;
	copied_button->h = b->h;
	copied_button->active = b->active;
	copied_button->rectangle = b->rectangle;
	copied_button->texture = malloc(sizeof(texture_t));
	copied_button->texture->w = b->texture->w;
	copied_button->texture->h = b->texture->h;
	copied_button->texture->texture = b->texture->texture;
	
	return copied_button;
}

/* Function: bt_destroy_button
 * This function frees and destroys a button object directed by a 
 * memory pointer
 *
 * Arguments:
 * 	b: the button to be destroyed
 *	
 * Return:
 *	Void
 *
 */
void bt_destroy_button(button_t *b)
{
	SDL_DestroyTexture(b->texture->texture);
	free(b->texture);
	free(b);
}
