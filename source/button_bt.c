#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "button_bt.h"
#include "sdl_config.h"
#include "list.h"
#include "mouse_ms.h"


/* Function: bt_create_button
 * -----------------------------------------------------------------------------
 * This function is used to create a button object, it receives as 
 * arguments the parameters of the button. The scale of the texture is 
 * calculated based on the parameters of widht and height
 * 	
 * Arguments:
 *	b: box with x, y, w, h
 *	act: boolean with info if the button is in a stae.
 *  rect: boolean that determines if a rectangle is draw around the button.	
 *	fill: boolean that determines if drawing the rectangle is filles
 *  in: inside color of the button. 
 *  out: outside color of the button.
 *	t: texture of the button.
 *
 * Return:
 *	Pointer to the button object
 */
button_t *bt_create_button(SDL_Rect r, bool act, bool rect, int fill, 
									  SDL_Color in , SDL_Color out,texture_t *t)
{
	button_t *new_button = malloc(sizeof(button_t));

	new_button->r = r;
	new_button->act = act;
	new_button->rect = rect;
	new_button->fill = fill;
	new_button->in = in;
	new_button->out = out;

	float scale_w = (float)r.w/t->w;
	float scale_h = (float)r.h/t->h;
	new_button->t = t;
	
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
	
	if (b->fill == true){
		dw_draw_filled_rectangle(b->r, b->in, b->in);
	}
	if (b->rect == true){
		dw_draw_rectangle(b->r, b->out);
	}
	
	float scale_w = (float)b->r.w/b->t->w;
	float scale_h = (float)b->r.h/b->t->h;
	if (scale_w < scale_h){
		int y = b->r.y + (b->r.h - b->t->h*scale_w)/2;
		SDL_Rect r = {.x = b->r.x, .y = y, .w = b->r.w};
		dw_draw_texture_fits_width(r, b->t);
	} else {
		int x = b->r.x + (b->r.w - b->t->w*scale_h)/2;
		SDL_Rect r = {.x = x, .y = b->r.y, .h = b->r.h};
		dw_draw_texture_fits_height(r, b->t);
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
void assign_button_parameters(SDL_Rect r, button_t *b)
{
	b->r = r;
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
	if (mouse_left_pressed == false || button->act == false){
		clicked_in_button = false;
	} else if (mouse_x > button->r.x && 
			   mouse_x < (button->r.x + button->r.w) && 
			   mouse_y > button->r.y && 
			   mouse_y < (button->r.y + button->r.h)){
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

	if (mouse_x > button->r.x && mouse_x < (button->r.x + button->r.w) &&
		mouse_y > button->r.y && mouse_y < (button->r.y + button->r.h)){
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
	copied_button->r = b->r;
	copied_button->act = b->act;
	copied_button->rect = b->rect;
	copied_button->fill = b->fill;
	copied_button->in = b->in;
	copied_button->out = b->out;
	copied_button->t = malloc(sizeof(texture_t));
	copied_button->t->w = b->t->w;
	copied_button->t->h = b->t->h;
	copied_button->t->texture = b->t->texture;
	
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
	SDL_DestroyTexture(b->t->texture);
	free(b->t);
	free(b);
}
