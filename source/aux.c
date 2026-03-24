#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>
#include "aux.h"
#include "dbg.h"
#include "draw_dw.h"
#include "dimensions_dm.h"
#include "mouse_ms.h"

#define MOVE_DELTA 15
#define FAST_MOVE_DELTA 60
#define ARROW_MDELTA_SLOW 5
#define ARROW_MDELTA_FAST 30
#define CW_MDELTA 15

char *ax_char_space = " ";
char *ax_char_comma = ",";
char *ax_char_newline = "\n";
char *ax_char_colon = ":";
char *ax_char_dash = "-";
char *ax_level_text = "Level";

static int move_delta;
static int arrow_mdelta;



/* Function: ax_chk_mouse_hover_rect
 *------------------------------------------------------------------------------
 * This function verifies is the mouse hovered a rectangle area.
 *
 * Arguments:
 * 	r: The rectangle to be verified
 *	
 * Return:
 *	true if the mouse hovered the rectangle
 *	false if otherwise
 *
*/
bool ax_chk_mouse_hover_rect(SDL_Rect r)
{
	int mouse_x = ms_get_mouse_x();
	int mouse_y = ms_get_mouse_y();

	bool hover;
	if (mouse_x > r.x 
		&& mouse_x < (r.x + r.w) 
		&& mouse_y > r.y 
		&& mouse_y < (r.y + r.h)){
		hover = true;
	} else {
		hover = false;
	} 
	return hover;
}
/* Function: ax_set_arrow_mdelta
 *------------------------------------------------------------------------------
 * Sets the fast and slow values for the arrow delta
 *
 * Arguments:
 *	true if fast, false if slow
 *
 * Return:
 *	void
 */
void ax_set_arrow_mdelta(bool fast)
{
	if (fast == true){
		arrow_mdelta = dm_scale_to_res(ARROW_MDELTA_FAST);
	} else if (fast == false){
		arrow_mdelta = dm_scale_to_res(ARROW_MDELTA_SLOW);
	}
}

/* Function: ax_get_arrow_move_delta
 *------------------------------------------------------------------------------
 * Returns the move delta for the arrow
 *
 * Arguments:
 *	Void
 *
 * Return:
 *	Arrow move delta
 */
int ax_get_arrow_move_delta()
{
	return arrow_mdelta;
}

/* Function: ax_get_cw_move_delta
 *------------------------------------------------------------------------------
 * Returns the move delta for the code window elements
 *
 * Arguments:
 *	Void
 *
 * Return:
 *	Code windows move delta
 */
int ax_get_cw_move_delta()
{
	return dm_scale_to_res(CW_MDELTA);
}

/* Function: ax_set_fast_move_delta
 *------------------------------------------------------------------------------
 * Sets the move delta variable to delta.
 *
 * Arguments:
 *	state: true if fast enabled, false if otherwise.
 *
 * Return:
 *	Void.
 */
void ax_set_fast_move_delta(bool state)
{
	if (state == true){
		move_delta = dm_scale_to_res(FAST_MOVE_DELTA);
	} else if (state == false){
		move_delta = dm_scale_to_res(MOVE_DELTA);
	}
}

/* Function: ax_get_move_delta
 *------------------------------------------------------------------------------
 * Gets the move delta variable
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	delta: The movement ammount.
 */
int ax_get_move_delta()
{
	return move_delta;
}


/* Function: ax_pad_rectangle
 *------------------------------------------------------------------------------
 * This functions takes a rectangle a returns a concentric rectangle offset
 * by the number of pixels specfified in the function.
 *
 * Arguments:
 *	r: The original rectable.
 *	offset: the offset of pixels of the concentric rectanble
 *  inside: the returned rectangle will be inside of the original.
 *			true if inside, false of outside
 *
 * Return:
 *	The new rectangle
 */
SDL_Rect ax_pad_rectangle(SDL_Rect b, int offset, bool inside)
{
	SDL_Rect r;
	if (inside == true){
		r.x = b.x + offset;
		r.y = b.y + offset;
		r.w = b.w - 2*offset;
		r.h = b.h - 2*offset;
	} else if (inside == false){
		r.x = b.x - offset;
		r.y = b.y - offset;
		r.w = b.w + 2*offset;
		r.h = b.h + 2*offset;
	}
	return r;
}

/* Function: ax_scale_rect_proportion
 *------------------------------------------------------------------------------
 * This function scales a rectangle and keeps the center. It scales it according
 * to the width and height of the rectangle.
 * Value of 1 is no change. 1.2 is 20% percent
 *
 * Arguments:
 *	r: The original rectable.
 *	scale: The value to which the rectangle will be scaled.
 *
 * Return:
 *	The new rectangle
 */
SDL_Rect ax_scale_rect_proportion(SDL_Rect r, float scale)
{
	SDL_Rect b;
	b.w = (int)r.w * scale;
	b.h = (int)r.h * scale;

	int x_center = r.x + r.w/2;
	int y_center = r.y + r.h/2;
  
	b.x = x_center - b.w/2;
	b.y = y_center - b.h/2;

	return b;
}

/* Function: ax_scale_rect_percentage
 *------------------------------------------------------------------------------
 * This function scales a rectangle and keeps the center. It scales it according
 * to the width and height of the rectangle.
 * Value of 1 is 100% increment, .20 is 20 percent/
 *
 * Arguments:
 *	r: The original rectable.
 *	scale: The value to which the rectangle will be scaled.
 *
 * Return:
 *	The new rectangle
 */
SDL_Rect ax_scale_rect_percentage(SDL_Rect r, float scale)
{
	SDL_Rect b = r;
	b.x -= (int)(scale * (float)b.w);
	b.y -= (int)(scale * (float)b.h);
	b.w += (int)(2*scale * (float)b.w);
	b.h += (int)(2*scale * (float)b.h);

	return b;
}

/* Function: ax_get_w_fit_h
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 */
int ax_get_texture_h_fit_w(int w, texture_t *t)
{
	float h = (float) (t->h * w)/t->w;
	return (int)h;
}

/* Function: ax_get_w_fit_h
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 */
int ax_get_texture_w_fit_h(int h, texture_t *t)
{
	float w = (float) (t->w * h)/t->h;
	return (int)w;
}

/* Function: create_string_append_number
 *------------------------------------------------------------------------------
 * Creates a string with a number attached to the end, there is not prepend 
 * zero value added.
 *
 * Arguments:
 * s: string to be used in the first part
 * n: number that will be used.
 *
 * Return:
 *	char * to the created string.
 *
 */
char *create_string_append_number(char *s,  int n)
{
	assert(s != NULL && "The string pointer is NULL");
	
	char *number = NULL;

	if (n<10){
		number = ax_number_to_string_prepend_zero(n);
	} else {
		number = ax_number_to_string(n);
	}

	check_mem(number);
	char *string = malloc(sizeof(char)*(strlen(s) + strlen(number)));
	check_mem(string);

	strcpy(string, s);
	strcat(string, ax_char_space);
	strcat(string, number);
	free(number);

error:
	return string;
}

/* Function: ax_copy_vbox
 *------------------------------------------------------------------------------
 * Assigns all the values to a box, it frees the old texture for an updated one
 * 
 *
 * Arguments:
 *	*vb: A pointer to the value box that will be modified.
 * 	val: The value that will be assigned
 *	pos: Determines if the position of the box will be copied
 *
 * Return:
 *	Void.
 */
void ax_copy_vbox(value_box_t *dst, value_box_t src, bool pos)
{
	dst->value = src.value;
	dst->type = src.type;
	dst->visible_box = src.visible_box;
	if (pos == true){
		dst->box = src.box;
	}
	dw_free_texture(dst->t);
	char *number = ax_number_to_string(src.value);
	dst->t = dw_create_text_texture(number, C_WHITE);
	free(number);
}

/* Function: ax_draw_value_box
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 */
void ax_draw_value_box(value_box_t *box, SDL_Color color)
{
	int text_w = 0;

	SDL_Rect val =  dm_get_value_box_val_wh();
	if (box->t != NULL){
		text_w = ax_get_texture_w_fit_h(val.h, box->t);
	}

	SDL_Rect vb = dm_get_value_box_val_wh();
	int x_offset = (vb.w - text_w)/2;
	int y_offset = ((vb.h - val.h)/2) + (val.h/5)/2;
	
	SDL_Rect r = {.x = box->box.x + x_offset, .y = box->box.y + y_offset,
				  .h = val.h};
	if(box->t != NULL){
		dw_draw_texture_fit_h(r, box->t);
	}	

	dw_draw_rectangle(box->box, color);
}

/* Function: get_text_height_fits_width
 * -----------------------------------------------------------------------------
 * This function receives a text string and a width value for that string and
 * returns the height that the text will need
 *
 * Arguments:
 *	w: width dimension.
 *	text: the text that will be checked
 *
 * Return:
 *	height of the text.
 */
int get_text_height_fits_width(int w, char *text)
{
	texture_t *text_texture = NULL;

	text_texture = dw_create_text_texture(text, C_WHITE);
	assert(NULL != text_texture && 
		   "Failed to load texture from rendered text");
	
	float scale = get_scale_fit_width(w, text_texture);
	int text_height = (int)text_texture->h * scale;

	dw_free_texture(text_texture);

	return text_height;

}

/* Function: get_text_width_fits_height
 * -----------------------------------------------------------------------------
 * This function receives a text string and a height value for that string and
 * returns the 
 *
 * Arguments:
 *	h: height dimension.
 *	text: the text that will be checked
 *
 * Return:
 *	width of the text.
 */
int get_text_width_fits_height(int h, char *text)
{
	texture_t *text_texture = NULL;

	text_texture = dw_create_text_texture(text, C_WHITE);
	assert(NULL != text_texture && 
		   "Failed to load texture from rendered text");
	
	float scale = get_scale_fit_height(h, text_texture);
	int text_width = (int)text_texture->w * scale;

	dw_free_texture(text_texture);

	return text_width;

}


/* Function: get_scale_fit_height
 * -----------------------------------------------------------------------------
 * This function receives a texture and a height value and returns the 
 * corresponding scale factor of the texture to fit the received height value.
 * 
 * Arguments:
 *	h: height dimension.
 *	t: texture that will be used for calculating the dimension.
 *
 * Return:
 *	The calculated scale that fits the height for the texture
 */
float get_scale_fit_height(int h, texture_t *texture)
{
	assert(NULL != texture && "The texture pointer is NULL");

	float scale_h = (float)h/texture->h;
	return scale_h;
}

/* Function: get_scale_fit_width
 * -----------------------------------------------------------------------------
 * This function receives a texture and a width value and sets the 
 * corresponding scale factor of the texture to fit the received width value.
 * 
 * Arguments:
 *	w: width dimension.
 *	t: texture that will be used for calculating the dimension.
 *
 * Return:
 *	The calculated scale that fits the height for the texture
 */
float get_scale_fit_width(int w, texture_t *texture)
{
	assert(NULL != texture && "The texture pointer is NULL");

	float scale_w = (float)w/texture->w;
	return scale_w;
}


/* Function: ax_get_x_delta_div
 * ----------------------------------------------------------------------------
 * Delta will be calculated according to the proportions of the x and y 
 * of the final values
 *
 * Arguments:
 *	x1: origin
 *	x2: destiny
 * 	y1: origin
 *	y2: destiny
 *
 * Return:
 * 	The delta movement according to distance.
 */
int ax_get_x_delta_div(int x1, int x2, int y1, int y2)
{
	int x_dis = abs(x1 - x2);
	int y_dis = abs(y1 - y2);
	float prop;	
	
	if (x_dis > y_dis){
	 	prop = 1;
	} else {
		prop = (float) y_dis/(float) x_dis;
	}
	return prop;
}

/* Function: ax_get_y_delta_div
 * ----------------------------------------------------------------------------
 * Delta will be calculated according to the proportions of the x and y 
 * of the final values
 *
 * Arguments:
 *	x1: origin
 *	x2: destiny
 * 	y1: origin
 *	y2: destiny
 *
 * Return:
 * 	The delta movement according to distance.
 */
int ax_get_y_delta_div(int x1, int x2, int y1, int y2)
{
	int x_dis = abs(x1 - x2);
	int y_dis = abs(y1 - y2);
	float prop;	
	
	if (y_dis > x_dis){
	 	prop = 1;
	} else {
		prop = (float) x_dis/(float) y_dis;
	}
	return prop;
}
/* Function: ax_get_movement_delta
 * ----------------------------------------------------------------------------
 * This function receives two values and returns a movement delta, according
 * to the distance of the two values, higher the distance, higher de movement
 * delta.
 *
 * Arguments:
 *	v1: first value to be considered.
 * 	v2: second value
 *	max_delta: maximum delta value that the function will use.
 *
 * Return:
 * 	The delta movement according to distance.
 */
int ax_get_movement_delta(int v1, int v2, int max_delta)
{
	int distance = abs(v1 - v2);
	
	float half_delta = (float)max_delta/2;

	int delta = 0;
	
	if(distance < 4){
		delta = 1;
	} else if (distance <= half_delta/2){
		delta = half_delta/4;
	} else if (distance <= half_delta){
		delta = half_delta/2;
	} else if (distance <= max_delta){
		delta = half_delta;
	} else {
		delta = max_delta;
	}
	return delta;
}

/* Function: ax_number_to_hex_string_two_digits
 * ----------------------------------------------------------------------------
 * Receives a number as a parameter and returns a string of the hex value
 * of the number
 *
 * Arguments:
 *	int number
 *
 * Return:
 *	string with the converted number with two digits of the hex value
 */
char *ax_number_to_hex_string_two_digits(int number)
{
	assert(number >= 0 && number < 100 && "Invalid number value");	
	char *number_hex = malloc(sizeof(char) * 3);
	snprintf(number_hex, 3, "%02X", number & 0xFF);
	return number_hex;

}

/* Function: ax_number_to_string_two_digits
 * ----------------------------------------------------------------------------
 * Receives a number as a parameter and returns a string of two digits of the
 * number
 *
 * Arguments:
 *	int number
 *
 * Return:
 *	string with the converted number with two digits nomenclature
 */
char *ax_number_to_string_two_digits(int number)
{
	assert(number >= 0 && number < 100 && "Invalid number value");	
	char *number_text = NULL;
	if (number < 10){
			number_text = ax_number_to_string_prepend_zero(number);
		} else {
			number_text = ax_number_to_string(number);
		}
	return number_text;
}
/* Function: ax_number_to_string_prepend_zero
 * ----------------------------------------------------------------------------
 * This function receives receives an integer and it returns it as a 
 * string form with a prepend zero as the first character
 *
 * Arguments:
 *	int number
 *
 * Return:
 *	string with the converted number with a leading zero
 */
char *ax_number_to_string_prepend_zero(int number)
{
	char *zero = ax_number_to_string(0);
	char *c = NULL;
	char *final_string = NULL;
	if (0 == number){
		c = malloc(sizeof(char)*2);
		sprintf(c, "%d", number);
		final_string = malloc(sizeof(char)*(strlen(c)+1));
		strcpy(final_string, zero);
		strcat(final_string, c);
	} else {
		c = malloc(sizeof(char)*(int)log10(number)+2);
		sprintf(c, "%d", number);
		final_string = malloc(sizeof(char)*(strlen(c)+1));
		strcpy(final_string, zero);
		strcat(final_string, c);
	}
	return final_string;
}

/* Function: number_to_string
 * -------------------------------------
 * This function receives receives an integer and it returns it as a 
 * string form that can be used to generate a texture and display it
 * on screen.
 *
 * Arguments:
 *	int number
 *
 * Return:
 *	char *string_number
 */

char *ax_number_to_string(int number)
{
	char *c = NULL;

	if (number == 0){
		c = malloc(sizeof(char)*2);
		sprintf(c, "%d", number);
	} else if (number == NO_VALUE){
		c = malloc(sizeof(char)*2);
		char negative[] = "-";
		strcpy(c, negative);
	} else if (number < 0){
		int absolute = abs(number);
		char *abs_number = malloc(sizeof(char)*(int)log10(absolute)+2);
		sprintf(abs_number, "%d", absolute);
		c = malloc(sizeof(char)*(int)log10(absolute)+3);
		char negative[] = "-";
		strcpy(c, negative);
		strcat(c, abs_number);
		free(abs_number);
	} else {
		c = malloc(sizeof(char)*(int)log10(number)+2);
		sprintf(c, "%d", number);
	}
	return c;
}


/* Function: check_text_fits_width_by_height
 * -------------------------------------
 * This function receives receives a string and verifies if it fits in a
 * given width using a given height
 *
 * Arguments:
 *	t: text that the comparison will be done
 *	h: height to be used
 *	w: width that is going to be verified if it fits
 *
 * Return:
 *	true if it fits.
 * 	false if it not fits
 */
bool check_text_fits_width_by_height(char *t, int h, int w)
{
	bool check = false;
	texture_t *text_texture = NULL;

	text_texture = dw_create_text_texture(t, C_WHITE);
	assert(NULL != text_texture && 
		   "Failed to load texture from rendered text");

	float scale_by_height = (float)h/(float)text_texture->h;
	float scaled_text_width = (float)text_texture->w * scale_by_height;
	if (scaled_text_width < w){
		check = true;
	}

	dw_free_texture(text_texture);
	return check;
}


/* Function: get_text_width
 * -------------------------------------
 * This functions receives a string parameter and returns the width in pixels
 * of that string using an specified scale factor
 *
 * Arguments:
 *	text: the text that is the width will be calculated
 *	scale: the scale factor of that specific text
 *
 * Return:
 *	An integer with the width of string
 */

int get_text_width(char *text, float scale)
{
	texture_t *text_texture = NULL;

	text_texture = dw_create_text_texture(text, C_WHITE);
	assert(NULL != text_texture && 
		   "Failed to load texture from rendered text");

	int scaled_text_width = (float)text_texture->w * scale;

	dw_free_texture(text_texture);

	return scaled_text_width;

}

/* Function: ax_get_wrapped_text_height
 * -----------------------------------------------------------------------------
 * This function calculates the height required for a warped text in a given 
 * scale with a give width.
 *
 * Arguments:
 *	w: width to the column of the text.
 *	h: height of the text.
 *	t: text to be measured.
 *
 * Return:
 *	Returns the height value of the box
 */
int ax_get_wrapped_text_height(int w, int h, char *t)
{
	assert(0 < w && "The width of the text is negative");
	assert(0 < h && "The height of the text is negative");
	assert( NULL != t && "Text pointer is NULL");

	int x_pos = WRAPPED_TEXT_X_OFFSET;
	int y_pos = 0;
	int y_offset = h;
	int string_size = strlen(t);
	
	char *text = malloc(sizeof(char)*string_size);
	check_mem(text);
	int last_fit = 0;
	int already_drawn = 0;
	for (int i = 0; i <= string_size; i++){
		if (t[i] == CHAR_SPACE || t[i] == CHAR_NULL || t[i] == CHAR_NEWLINE){
			
			strncpy(text, t + already_drawn, i - already_drawn);
			
			if (check_text_fits_width_by_height(text, h, w) == true){
				last_fit = i;
			}
			if (check_text_fits_width_by_height(text, h, w) == false || 
									 CHAR_NULL == t[i] || t[i] == CHAR_NEWLINE){
				if (last_fit == already_drawn){
					puts("The size of the font does not fits width");
					last_fit = string_size;
				}
				memset(text, 0, string_size);
				if (already_drawn == 0){
					strncpy(text, t + already_drawn, last_fit - already_drawn);
				} else {
					strncpy(text, t + already_drawn + 1, 
						   						  last_fit - already_drawn - 1);
				}
				y_pos += y_offset;
				already_drawn = last_fit;
				i = last_fit;
				memset(text, 0, string_size);
			}
		}
	} 
	free(text);
error:
	return y_pos;
}


