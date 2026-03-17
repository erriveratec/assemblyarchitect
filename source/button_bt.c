#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "button_bt.h"
#include "sdl_config.h"
#include "list.h"
#include "mouse_ms.h"

static const Uint32 BUTTON_LIFT = 4;
static const float HOVER_SCALE = 0.06f;
static const Uint32 BUTTON_SHADOW_OFS = 5; // The shadow of iface buttons

static const Uint32 IFACE_BUTTON_SPACE_W = 7; // Used for the button inner bord
static const Uint32 IFACE_BUTTON_OUTER_W = 3; //Outer width of the iface button

static bool g_btns_blocked = false;

static bool chk_mouse_hover_iface_btn(iface_btn_t *btn);
static void draw_btn_scaled_texture(SDL_Rect box, iface_btn_t *b);
static int get_w_iface_space_border();
static int get_w_iface_outer_border();
static int get_w_button_padding();
static int get_h_button_padding();

/* Function: dm_get_w_button_padding
 * -----------------------------------------------------------------------------
 *	Returns the scale value for the horizontal padding
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
static int get_w_button_padding()
{
	return dm_get_w_padding() + 
		   dm_scale_to_resolution(IFACE_BUTTON_SPACE_W);
}

/* Function: dm_get_h_but_padding
 * -----------------------------------------------------------------------------
 *	Returns the scale value for the vertical padding
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
static int get_h_button_padding()
{
	return dm_get_h_padding() + 
		   dm_scale_to_resolution(IFACE_BUTTON_SPACE_W);
}



/* Function: dm_get_ofs_iface_border
 * -----------------------------------------------------------------------------
 *	Returns the interfacer border space that will be use for interface buttons
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	The offset that will be use of the rectangle padding of the interface
 */
static int get_w_iface_space_border()
{
	return dm_scale_to_resolution(IFACE_BUTTON_SPACE_W);
}

/* Function: dm_get_ofs_outer_border
 * -----------------------------------------------------------------------------
 *	Returns the interfacer border space that will be use for interface buttons
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	The offset that will be use of the rectangle padding of the interface
 */
static int get_w_iface_outer_border()
{
	return dm_scale_to_resolution(IFACE_BUTTON_OUTER_W);
}

/* Function: dm_get_ofs_button_shadow
 * -----------------------------------------------------------------------------
 *	Returns the shadow offset for the buttons
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	The offset that will be used for the shadow of the buttons
 */
int bt_get_ofs_button_shadow()
{
	return dm_scale_to_resolution(BUTTON_SHADOW_OFS);
}

/* Function: get_hover_factor
 * -----------------------------------------------------------------------------
 * Returns the increment of the elements during hover
 * 
 * Arguments:
 *	Void.
 *
 * Return:
 *	The hover factor
 */
int bt_get_btn_lift()
{
	return dm_scale_to_resolution(BUTTON_LIFT);
}

/* Function: get_hover_factor
 * -----------------------------------------------------------------------------
 * Returns the increment of the elements during hover
 * 
 * Arguments:
 *	Void.
 *
 * Return:
 *	The hover factor
 */
float bt_get_hover_factor()
{
	return HOVER_SCALE;
}

/* Function: bt_destroy_iface_btn
 *-----------------------------------------------------------------------------
 * This function frees and destroys an iface button object directed by a 
 * memory pointer
 *
 * Arguments:
 * 	b: the button to be destroyed
 *	
 * Return:
 *	Void
 *
 */
void bt_destroy_iface_btn(iface_btn_t *b)
{
	SDL_DestroyTexture(b->t->texture);
	free(b->t);
	free(b);
}

/* Function: bt_chk_mouse_rel_iface_btn
 *------------------------------------------------------------------------------
 * Arguments:
 * 	button: the button to be verified
 *	
 * Return:
 *	true if the mouse clicked inside the button
 *	false if otherwise
*/
bool bt_chk_mouse_rel_iface_btn(iface_btn_t *btn)
{
	assert(btn != NULL && "The button pointer cannot be NULL");

	int mouse_x = ms_get_mouse_x();
	int mouse_y = ms_get_mouse_y();
	int mouse_left_released = ms_left_released();
	
	if (mouse_left_released == false || btn->enabled == false){
		return false;
	}

	if (mouse_x > btn->r.x && mouse_x < (btn->r.x + btn->r.w) &&
		mouse_y > btn->r.y && mouse_y < (btn->r.y + btn->r.h)){
		return true;
	} else {
		return false;
	} 
}


/* Function: bt_chk_mouse_click_iface_button
 *------------------------------------------------------------------------------
 * This function verifies is the mouse has clicked insiden an iface button.
 *
 * Arguments:
 * 	button: the button to be verified
 *	
 * Return:
 *	true if the mouse clicked inside the button
 *	false if otherwise
 *
*/
bool bt_chk_mouse_click_iface_btn(iface_btn_t *btn)
{
	assert(NULL != btn && "The button pointer cannot be NULL");

	int mouse_x = ms_get_mouse_x();
	int mouse_y = ms_get_mouse_y();
	int mouse_left_pressed = ms_left_pressed();

	bool clicked_in_button;
	if (mouse_left_pressed == false || btn->enabled == false){
		clicked_in_button = false;
	} else if (mouse_x > btn->r.x && 
			   mouse_x < (btn->r.x + btn->r.w) && 
			   mouse_y > btn->r.y && 
			   mouse_y < (btn->r.y + btn->r.h)){
		clicked_in_button = true;
	} else {
		clicked_in_button = false;
	} 
	return clicked_in_button;
}

/* Function: chk_mouse_hover_iface_button
 *------------------------------------------------------------------------------
 * This function verifies is the mouse has clicked insiden an iface button.
 *
 * Arguments:
 * 	button: the button to be verified
 *	
 * Return:
 *	true if the mouse clicked inside the button
 *	false if otherwise
 *
*/
static bool chk_mouse_hover_iface_btn(iface_btn_t *btn)
{
	assert(NULL != btn && "The button pointer cannot be NULL");

	int mouse_x = ms_get_mouse_x();
	int mouse_y = ms_get_mouse_y();

	bool hover;
	if (mouse_x > btn->r.x 
		&& mouse_x < (btn->r.x + btn->r.w) 
		&& mouse_y > btn->r.y 
		&& mouse_y < (btn->r.y + btn->r.h)){
		hover = true;
	} else {
		hover = false;
	} 
	return hover;
}
/* Function: bt_create_iface_btn
 * -----------------------------------------------------------------------------
 * Creates an interface button used for menus with letters
 * 	
 * Arguments:
 *	b: box with x, y, w, h
 *	t: texture of the button.
 *  enabled: boolean to determine if the button is enabled at a given time
 *
 * Return:
 *	Pointer to the button object
 */
iface_btn_t *bt_create_iface_btn(SDL_Rect r, texture_t *t, bool enabled)
{
	iface_btn_t *new_button = malloc(sizeof(iface_btn_t));
	new_button->r = r;
	float scale_w = (float)r.w/t->w;
	float scale_h = (float)r.h/t->h;
	new_button->t = t;
	new_button->enabled = enabled;
	return new_button;
}

/* Function: draw_btn_scaled_texture
 * -----------------------------------------------------------------------------
 * Scales the texture of the interface button according to whats needed.
 * If the texture is NULL does not do anything.
 * 	
 * Arguments:
 *	box: The box that will be used to calculate the texture.
 *	b: The button that contains the texture.
 *
 * Return:
 *	Void.
 */
static void draw_btn_scaled_texture(SDL_Rect box, iface_btn_t *b)
{
	if (b->t == NULL) return;
	
	int w_pad = get_w_button_padding();
	int h_pad = get_h_button_padding();
	
	float scale_w = (float)(box.w - 2*w_pad)/b->t->w;
	float scale_h = (float)(box.h - 2*h_pad)/b->t->h;

	if (scale_w < scale_h){
		int y = box.y + (box.h - b->t->h*scale_w)/2;
		SDL_Rect r = {.x = box.x + w_pad, .y = y, .w = box.w - 2*w_pad};
		dw_draw_texture_fits_width(r, b->t);
	} else {
		int x = box.x + (int)(box.w - b->t->w*scale_h)/2;
		SDL_Rect r = {.x = x, .y = box.y + w_pad, .h = box.h - 2*h_pad};
		dw_draw_texture_fit_h(r, b->t);
	}
}

/* Function: bt_draw_iface_btn
 * -----------------------------------------------------------------------------
 * This function draws the interface button 
 * 	
 * Arguments:
 *	b: The button object that is going to be drawn
 *	blk = If true, the buttons just draws without interaction 
 *
 * Return:
 *	Void.
 */
void bt_draw_iface_btn(iface_btn_t *b, bool blk)
{
	assert(b != NULL && "The button pointer is NULL");
	int status = SUCCESS;	
	
	SDL_Rect box = b->r;
 	bool hover = chk_mouse_hover_iface_btn(b);
	bool clicked = bt_chk_mouse_click_iface_btn(b);
	int offset = get_w_iface_space_border();
	int shadow_offset = bt_get_ofs_button_shadow();	
	
	if (hover == true && b->enabled == true && blk == false){
		float h_ofs = bt_get_hover_factor();
		box = ax_scale_rect_percentage(box, h_ofs);
		box.y -= bt_get_btn_lift();
		shadow_offset += 2*h_ofs;
	}

	if (b->enabled == false){
		dw_draw_filled_rectangle(box, C_BLACK, C_GREY);
		SDL_Rect in = ax_pad_rectangle(b->r, offset, true);
		dw_draw_rectangle(in, C_GREY);
		draw_btn_scaled_texture(box, b);
	} else if (clicked == true && blk == false){
		SDL_Rect shadow = {.x = box.x + shadow_offset, 
						   .y = box.y + shadow_offset,
					   	   .w = box.w, 
						   .h = box.h};
		dw_draw_filled_rectangle(shadow, C_DIMGREY, C_DIMGREY);
		dw_draw_filled_rectangle(box, C_WHITE, C_WHITE);
		SDL_Rect in = ax_pad_rectangle(box, offset, true);
		dw_draw_filled_rectangle(in, C_NEARBLACK, C_WHITE);
		draw_btn_scaled_texture(box, b);
	} else {
		SDL_Rect shadow = {.x = box.x + shadow_offset, 
						   .y = box.y + shadow_offset,
					   	   .w = box.w, 
						   .h = box.h};
		dw_draw_filled_rectangle(shadow, C_DIMGREY, C_DIMGREY);
		
		int outer_border = get_w_iface_outer_border();
		dw_draw_thick_rect(box, outer_border, C_WHITE);

		SDL_Rect in = ax_pad_rectangle(box, offset, true);
		if (hover == true){
			dw_draw_filled_rectangle(in, C_SOFTBLACK, C_LIGHTGREY);
		} else {
			dw_draw_filled_rectangle(in, C_NEARBLACK, C_LIGHTGREY);
		}
		
		in = ax_pad_rectangle(in, 1, true);
		dw_draw_inner_shadow_lines(in, C_GREY, C_SILVERGREY);
		in = ax_pad_rectangle(in, 1, true);
		dw_draw_inner_shadow_lines(in, C_GREY, C_SILVERGREY);
		if (hover == true && blk == false){
			in = ax_pad_rectangle(in, 1, true);
			dw_draw_inner_shadow_lines(in, C_GREY, C_SILVERGREY);
		}
		draw_btn_scaled_texture(box, b);
	}
	
	assert(status != FAIL && "The texture could not be drawn");
}

/* Function: bt_draw_iface_btn_nopad
 * -----------------------------------------------------------------------------
 * This function draws the interface button 
 * 	
 * Arguments:
 *	b: The button object that is going to be drawn
 *
 * Return:
 *	Void.
 */
void bt_draw_iface_btn_nopad(iface_btn_t *b)
{
	assert(b != NULL && "The button pointer is NULL");
	
	int status = SUCCESS;	

	int shadow_offset = bt_get_ofs_button_shadow();	
	SDL_Rect shadow = {.x = b->r.x + shadow_offset, .y = b->r.y + shadow_offset,
					   .w = b->r.w, .h = b->r.h};
	dw_draw_filled_rectangle(shadow, C_DARKGREY, C_DARKGREY);

	bool clicked = bt_chk_mouse_click_iface_btn(b);
	int offset = get_w_iface_space_border();
	
	if (b->enabled == false){
		dw_draw_filled_rectangle(b->r, C_BLACK, C_GREY);
		SDL_Rect in = ax_pad_rectangle(b->r, offset, true);
		dw_draw_rectangle(in, C_GREY);
	} else if (clicked == true){
		dw_draw_filled_rectangle(b->r, C_WHITE, C_WHITE);
		SDL_Rect in = ax_pad_rectangle(b->r, offset, true);
		dw_draw_filled_rectangle(in, C_BLACK, C_WHITE);
	} else {
		dw_draw_filled_rectangle(b->r, C_BLACK, C_WHITE);
		SDL_Rect in = ax_pad_rectangle(b->r, offset, true);
		dw_draw_rectangle(in, C_WHITE);
	}
	
	SDL_Rect p = ax_pad_rectangle(b->r, offset, true);
	float scale_w = (float)(b->r.w - 2*offset)/b->t->w;
	float scale_h = (float)(b->r.h - 2*offset)/b->t->h;
	
	if (scale_w < scale_h){
		int y = p.y + (p.h - b->t->h*scale_w)/2;
		SDL_Rect r = {.x = p.x, .y = y, .w = p.w};
		dw_draw_texture_fits_width(p, b->t);
	} else {
		int x = p.x + (p.w - b->t->w*scale_h)/2;
		SDL_Rect r = {.x = x, .y = p.y, .h = p.h};
		dw_draw_texture_fit_h(r, b->t);
	}
	assert(status != FAIL && "The texture could not be drawn");
}

/* Function: bt_create_btn
 * -----------------------------------------------------------------------------
 * This function is used to create a button object, it receives as 
 * arguments the parameters of the button. The scale of the texture is 
 * calculated based on the parameters of widht and height
 * 	
 * Arguments:
 *	b: box with x, y, w, h
 *	t: texture of the button.
 *  animate: boolean that determines if the texture will be animated.	
 * 	anim_state: 
 *  rect: boolean that determines if a rectangle is draw around the button.	
 *
 * Return:
 *	Pointer to the button object
 */
btn_t *bt_create_btn(SDL_Rect r, texture_t *t)
{
	btn_t *new_button = malloc(sizeof(btn_t));
	new_button->r = r;
	new_button->t = t;
	new_button->animated = false;
	new_button->anim_dir = false;
	new_button->anim_state = 0;
	new_button->rect = false;
	new_button->enabled = true;

	return new_button;
}

/* Function: bt_draw_btn
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
void bt_draw_btn(btn_t *b, bool blk)
{
	assert(b != NULL && "The button pointer is NULL");
	
	
	if (b->rect == true){
		dw_draw_rectangle(b->r, C_WHITE);
	}
	
	bool hover = ax_chk_mouse_hover_rect(b->r);
	SDL_Rect box = b->r;	
	if (hover == true && b->enabled == true && blk == false){
		float h_ofs = bt_get_hover_factor();
		box = ax_scale_rect_percentage(box, h_ofs);
		box.y -= bt_get_btn_lift();
	}

	float scale_w = (float)b->r.w/b->t->w;
	float scale_h = (float)b->r.h/b->t->h;

	if (b->animated == false){
		if (scale_w < scale_h){
			int y = b->r.y + (b->r.h - b->t->h*scale_w)/2;
			SDL_Rect r = {.x = b->r.x, .y = y, .w = b->r.w};
			dw_draw_texture_fits_width(r, b->t);
		} else {
			int x = b->r.x + (b->r.w - b->t->w*scale_h)/2;
			SDL_Rect r = {.x = x, .y = b->r.y, .h = b->r.h};
			dw_draw_texture_fit_h(r, b->t);
		}
	} else if (b->animated == true){
		
		if (scale_w < scale_h){
			int y = b->r.y + (b->r.h - b->t->h*scale_w)/2;
			SDL_Rect r = {.x = b->r.x - b->anim_state/2, 
						  .y = y - b->anim_state/2, 
						  .w = b->r.w + b->anim_state};
			dw_draw_texture_fits_width(r, b->t);
		} else {
			int x = b->r.x + (b->r.w - b->t->w*scale_h)/2;
			SDL_Rect r = {.x = x - b->anim_state/2, 
						  .y = b->r.y - b->anim_state/2, 
						  .h = b->r.h + b->anim_state};
			dw_draw_texture_fit_h(r, b->t);
		}
		int anim_max = dm_get_btn_anim_max();
		int anim_delta = dm_get_btn_anim_delta();
		if (b->anim_dir == false && b->anim_state == anim_max){
			b->anim_dir = true;
		} else if (b->anim_dir == true && b->anim_state == 0){
			b->anim_dir = false;
		}
		if (b->anim_dir == false){
			b->anim_state += anim_delta;
		} else if (b->anim_dir == true){
			b->anim_state -= anim_delta;
		}
	}


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
void assign_button_parameters(SDL_Rect r, btn_t *b)
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
bool bt_btn_clicked(btn_t *button)
{
	assert(NULL != button && "The button pointer cannot be NULL");

	int mouse_x = ms_get_mouse_x();
	int mouse_y = ms_get_mouse_y();
	int mouse_left_pressed = ms_left_pressed();

	bool clicked_in_button;
	if (mouse_left_pressed == false){
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

/* Function: bt_btn_rclicked
 *------------------------------------------------------------------------------
 * This function verifies is the mouse has right clicked insiden a give button.
 *
 * Arguments:
 * 	button: the button to be verified
 *	
 * Return:
 *	true if the mouse right clicked inside the button
 *	false if otherwise
 *
*/
bool bt_btn_rclicked(btn_t *button)
{
	assert(NULL != button && "The button pointer cannot be NULL");

	int mouse_x = ms_get_mouse_x();
	int mouse_y = ms_get_mouse_y();
	int mouse_right_pressed = ms_right_pressed();

	bool clicked_in_button;
	if (mouse_right_pressed == false){
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
bool bt_ms_rel_btn(btn_t *button)
{
	assert(button != NULL && "The button pointer cannot be NULL");

	int mouse_x = ms_get_mouse_x();
	int mouse_y = ms_get_mouse_y();
	int mouse_left_released = ms_left_released();
	
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
btn_t *bt_copy_button(btn_t *b)
{
	btn_t *copied_button = malloc(sizeof(btn_t));
	copied_button->r = b->r;
	copied_button->t = malloc(sizeof(texture_t));
	copied_button->t->w = b->t->w;
	copied_button->t->h = b->t->h;
	copied_button->t->texture = b->t->texture;
	copied_button->animated = b->animated;
	copied_button->anim_state = b->anim_state;
	copied_button->rect = b->rect;

	
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
void bt_destroy_button(btn_t *b)
{
	SDL_DestroyTexture(b->t->texture);
	free(b->t);
	free(b);
}
