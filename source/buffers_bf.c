#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <time.h>
#include <assert.h>
#include <stdbool.h>
#include "buffers_bf.h"
#include "draw_dw.h"
#include "list.h"
#include "dbg.h"
#include "aux.h"
#include "dimensions_dm.h"
#include "button_bt.h"
#include "code_line_cl.h"

// The posible types of input values
#define WHOLE 0
#define ASCII 1
#define DEFAULT_BUFFER_SIZE 5
#define BUFFER_MOVEMENT_DELTA 5

#define NATURAL_NMAX 9
#define NATURAL_NMIN 1

#define INPUT_BUFFER_TEXT "Input Buffer [IB]"
#define OUTPUT_BUFFER_TEXT "Output Buffer [OB]"
#define OUTPUT_BUFFER_WIN_X 10000

texture_t *input_text; 
texture_t *output_text;

SDL_Rect input_box;
SDL_Rect output_box;

static List *input_list = NULL;
static List *output_list = NULL;

button_t input_buffer_button;
button_t output_buffer_button;

operand_t *input_buffer = NULL;
operand_t *output_buffer = NULL;

static bool g_win_condition;

static int g_input_list_x_pos;
static int g_output_list_x_pos;
int g_input_list_type = NOT_ASSIGNED;  
int g_input_buffer_size = DEFAULT_BUFFER_SIZE;
void add_input_to_list(int value, int type);
void draw_output_buffer();
void draw_input_buffer();
static bool check_win_condition();
static void destroy_input_list();
static void destroy_output_list();


/* Function: iw_init_buf_texture
 *------------------------------------------------------------------------------
 * Creates the instructions texture of the instruction box
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Void.
 */
void bf_init_buffer_assets()
{
	input_text = dw_create_text_texture(INPUT_BUFFER_TEXT, C_WHITE);
	output_text = dw_create_text_texture(OUTPUT_BUFFER_TEXT, C_WHITE);
	SDL_Rect ib = dm_get_stage_input_buffer_box();
	int ofs = dm_get_ofs_buffer_value_box();
	g_output_list_x_pos = ib.x + ofs;
	g_input_list_x_pos = dm_get_screen_width();
}

/* Function: bf_set_win_condition
*------------------------------------------------------------------------------
* Sets the win condition variable for the output buffer
*
* Arguments:
* 	Void.
*	
* Return:
*	Void.
*/
void bf_set_win_condition()
{
	g_win_condition = true;
}

/* Function: bf_reset_win_condition
*------------------------------------------------------------------------------
* Resets the win condition variable for the output buffer
*
* Arguments:
* 	Void.
*	
* Return:
*	Void.
*/
void bf_reset_win_condition()
{
	g_win_condition = false;
}

/* Function: bf_reset_win_condition
*------------------------------------------------------------------------------
* Resets the win condition variable for the output buffer
*
* Arguments:
* 	Void.
*	
* Return:
*	Void.
*/
static bool check_win_condition()
{
	return g_win_condition;
}

/* Function: bf_set_input_buffer_size
*------------------------------------------------------------------------------
* Sets a value to the input buffer
*
* Arguments:
* 	size: The size that will be set
*	
* Return:
*	void.
*/
void bf_set_input_buffer_size(int size)
{
	g_input_buffer_size = size;
}

/* Function: get_input_buffer_list_size
*------------------------------------------------------------------------------
* Arguments:
* 	None.
*	
* Return:
*	int with the size of the input buffer
*/
int get_input_buffer_list_size()
{
	List *list = get_input_list();
	return List_count(list);
}

/* Function: get_input_buffer_list_size
*------------------------------------------------------------------------------
* Returns the list size of the input buffer.
*
* Arguments:
* 	None.
*	
* Return:
*	int with the size of the output buffer
*/
int get_output_buffer_list_size()
{
	List *list = get_output_list();
	return List_count(list);
}
/* Function: bf_check_released_in_buffer
*------------------------------------------------------------------------------
* This functions verifies if the user clicked one of the registers available
* in the screen
*
* Arguments:
* 	None.
*	
* Return:
*	true if the user clicked over an instructions, false if otherwise
*
*/
bool bf_check_released_in_buffer()
{
   	bool selected = false;

	if (bt_check_mouse_released_button(&input_buffer_button) == true){
		selected = true;
	} 
   	
	if (bt_check_mouse_released_button(&output_buffer_button) == true){
		selected = true;
	}
	return selected;
}

/* Function: bf_create_buffer_operand_by_id
*------------------------------------------------------------------------------
* This returns the pointer to the buffer where the player clicked
*
* Arguments:
* 	id: The id of the operand that will be created.
*	
* Return:
*	The created buffer operand
*
*/
operand_t *bf_create_buffer_operand_by_id(int id)
{
	assert(id > BUFFERS_MIN && id < BUFFERS_MAX && "Incorrect buffer id");
   	
	operand_t *b = malloc(sizeof(operand_t));
	if (id == IB){
		texture_t *t = cl_create_operand_texture(id);
		b->b = bt_create_button(input_buffer->b->r, input_buffer->b->act, 
							 input_buffer->b->rect, input_buffer->b->fill, 
							 C_BLACK, C_WHITE, t);

		b->id = input_buffer->id;
	} else if (id == OB){
		texture_t *t = cl_create_operand_texture(id);
		b->b = bt_create_button(output_buffer->b->r, output_buffer->b->act, 
							 output_buffer->b->rect, output_buffer->b->fill, 
							 C_BLACK, C_WHITE, t);
		b->id = output_buffer->id;
	}
	check_mem(b);
	error:
	return b;
}

/* Function: bf_create_operand_of_selected_buffer
*------------------------------------------------------------------------------
* This returns the pointer to the buffer where the player clicked
*
* Arguments:
* 	None.
*	
* Return:
*	The pointer of the buffer that user selected.
*
*/
operand_t *bf_create_operand_of_selected_buffer()
{
   	operand_t *b = NULL;

	if (bt_check_mouse_released_button(&input_buffer_button) == true){
		b = malloc(sizeof(operand_t));
		texture_t *t = cl_create_operand_texture(IB);

		b->b = bt_create_button(input_buffer->b->r, input_buffer->b->act, 
							 input_buffer->b->rect, input_buffer->b->fill,
							 C_BLACK, C_WHITE, t);

		b->id = input_buffer->id;
	} 
   	
	if (bt_check_mouse_released_button(&output_buffer_button) == true){
		b = malloc(sizeof(operand_t));
		texture_t *t = cl_create_operand_texture(OB);
		b->b = bt_create_button(output_buffer->b->r, output_buffer->b->act, 
							 output_buffer->b->rect, output_buffer->b->fill, 
							 C_BLACK, C_WHITE, t);
		b->id = output_buffer->id;
	}
	return b;
}

/* Function: bf_initialize_buffer_operands
 * -----------------------------------------------------------------------------
 * Arguments:
 * 	None.
 *	
 * Return:
 *	Void.
 *
 */
void bf_initialize_buffer_operands()
{
	SDL_Rect r = dm_get_code_button_wh();
	texture_t *ib = dw_create_text_texture(ib_text, C_WHITE);
	check_mem(ib);
	button_t *a = bt_create_button(r, true, false, false, C_BLACK, C_WHITE, 
						        ib);
	check_mem(a);

	input_buffer = cl_create_operand(IB, a);
	check_mem(input_buffer);

	
	texture_t *ob = dw_create_text_texture(ob_text, C_WHITE);
	check_mem(ob);
	button_t *b = bt_create_button(r, true, false, false, C_BLACK, C_WHITE,
								ob);
	check_mem(b);
	output_buffer = cl_create_operand(OB, b);
	check_mem(output_buffer);

error:
	return;
}

/* Function: bf_set_input_buffer_button
 *------------------------------------------------------------------------------
 * Arguments:
 *	x: x coordinate.
 *	y: y coordinate.
 * 	w: width of the box.
 *	h: height of the box.
 * 
 * Return:
 *	Void.
 */
void bf_set_input_buffer_button(SDL_Rect r)
{
	input_buffer_button.r = r;
}

/* Function: bf_set_output_buffer_button
 *------------------------------------------------------------------------------
 * Arguments:
 *	x: x coordinate.
 *	y: y coordinate.
 * 	w: width of the box.
 *	h: height of the box.
 * 
 * Return:
 *	Void.
 */
void bf_set_output_buffer_button(SDL_Rect r)
{
	output_buffer_button.r = r;
}

/* Function: get_input_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Pointer to input list.
 */
List *get_input_list()
{
	return input_list;
}

/* Function: get_output_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Pointer to input list.
 */
List *get_output_list()
{
	return output_list;
}


/* Function: bf_add_output_to_list
 * -----------------------------------------------------------------------------
 * Arguments:
 *	None.
 *	
 * Return:
 *	void
 */
void bf_add_output_to_list()
{
	List *outputs = get_output_list();

	assert(outputs != NULL && "Input pointer is NULL");
	
	value_box_t *new_output = malloc(sizeof(value_box_t));
	check_mem(new_output);

	new_output->value = NO_VALUE;
	new_output->type = NOT_ASSIGNED;
	SDL_Rect ib = dm_get_stage_input_buffer_box();
	int ofs = dm_get_ofs_buffer_value_box();
	new_output->box.x = ib.x + ofs;
	SDL_Rect vb = dm_get_vbox_wh();
	int y_offset = (output_box.h - vb.h)/2;
	new_output->box.y = output_box.y + y_offset;
	new_output->box.w = vb.w;
	new_output->box.h = vb.h;
    new_output->t = dw_create_text_texture(char_dash, C_WHITE);
	new_output->visible_box = false;

	List_push(outputs, new_output);

error:
	return;
}
/* Function: add_input_to_list
 * -----------------------------------------------------------------------------
 * Arguments:
 * 	value: The input value to be added.
 *	type: The type of input to be added.
 *	
 * Return:
 *	void
 */
void add_input_to_list(int value, int type)
{
	List *inputs = get_input_list();

	assert(inputs != NULL && "Input pointer is NULL");
	
	value_box_t *new_input = malloc(sizeof(value_box_t));
	check_mem(new_input);

	int screen_width = dm_get_screen_width();

	new_input->value = value;
	new_input->type = type;
	new_input->box.x = screen_width;
	SDL_Rect vb = dm_get_vbox_wh();
	int y_offset = (input_box.h - vb.h)/2;
	new_input->box.y = input_box.y + y_offset;
	new_input->box.w = vb.w;
	new_input->box.h = vb.h;
	char *number = ax_number_to_string(value);
	new_input->t = dw_create_text_texture(number, C_WHITE);
	free(number);

	List_push(inputs, new_input);

error:
	return;
}

/* Function: bf_create_natural_input_list
 * -----------------------------------------------------------------------------
 * Arguments:
 *	Size: the size of the input list.
 *	
 * Return:
 *	void
 */
void bf_create_natural_numbers_input_list(int size)
{
	for (int i = 0; i < size; i++){
		int value = (rand() % NATURAL_NMAX) + NATURAL_NMIN;
		add_input_to_list(value, WHOLE);
	}
}

/* Function: bf_create_input_list
 *------------------------------------------------------------------------------
 * This function creates and initializes the input list.
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 *
 */
void bf_create_input_list()
{
	assert(input_list == NULL && "The register list is not NULL");
	input_list = List_create();
	check_mem(input_list);

error:
	return;
}

/* Function: bf_create_output_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 *
 */
void bf_create_output_list()
{
	assert(output_list == NULL && "The register list is not NULL");
	output_list = List_create();
	check_mem(output_list);

error:
	return;
}


/* Function: bf_set_input_box
 *------------------------------------------------------------------------------
 * Arguments:
 *	x: x coordinate.
 *	y: y coordinate.
 * 	w: width of the box.
 *	h: height of the box.
 * 
 * Return:
 *	Void.
 */
void bf_set_input_box(SDL_Rect r)
{
	assert(r.w > 0 && r.h > 0 && "Width and height must be > 0");
	input_box.x = r.x;
	input_box.y = r.y;
	input_box.w = r.w;
	input_box.h = r.h;
}

/* Function: bf_set_output_box
 *------------------------------------------------------------------------------
 * Arguments:
 *	x: x coordinate.
 *	y: y coordinate.
 * 	w: width of the box.
 *	h: height of the box.
 * 
 * Return:
 *	Void.
 */
void bf_set_output_box(SDL_Rect r)
{
	assert(r.w > 0 && r.h > 0 && "Width and height must be > 0");
	output_box.x = r.x;
	output_box.y = r.y;
	output_box.w = r.w;
	output_box.h = r.h;
}



/* Function: draw_buffers
 *------------------------------------------------------------------------------
 * Arguments:
 *	Void.
 * 
 * Return:
 *	Void.
 */
void bf_draw_buffers()
{
	draw_input_buffer();
	draw_output_buffer();
}

/* Function: draw_output_buffer
 *------------------------------------------------------------------------------
 * This function draws the corresponding to the output buffer
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 */
void draw_output_buffer()
{
	List *outputs = get_output_list();
	check_mem(outputs);
	int	list_size = List_count(outputs);

	int x;
	int ofs = dm_get_ofs_buffer_value_box();
	int ofsval = dm_get_ofs_between_value_box();
	SDL_Rect val =  dm_get_vbox_val_wh();
	if (check_win_condition() == true){
		x = OUTPUT_BUFFER_WIN_X;	
	} else {
		x = output_box.x + ofs + (list_size-1)*(val.w + ofsval);
	}
	int y = output_box.y + ofs;
	if (outputs != NULL && list_size > 0){
		int draw_x = g_output_list_x_pos;
		LIST_FOREACH(outputs, first, next, cur){
			value_box_t *cur_output = cur->value;
			cur_output->box.x = draw_x;

			if (cur_output->visible_box != false){
				ax_draw_value_box(cur_output, C_WHITE);
			}
			draw_x -= val.w + ofsval;
		}
		if (g_output_list_x_pos < x){
			g_output_list_x_pos += get_movement_delta(x, g_output_list_x_pos, 
								   						BUFFER_MOVEMENT_DELTA);
		}
	}
	dw_draw_rectangle(output_box, C_WHITE);
	SDL_Rect ob = dm_get_stage_ob_text_box();
	dw_draw_texture_fits_height(ob, output_text);
error:
	return;
}

/* Function: check_if_output_buffer_is_in_pos
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	bool if buffer is in position
 */
bool check_if_output_buffer_position_set()
{
	bool in_pos = false;
	int list_size = 0;
	List *outputs = get_output_list();
	if (outputs != NULL){
		list_size = List_count(outputs);
	}

	int ofs = dm_get_ofs_buffer_value_box();
	int ofsval = dm_get_ofs_between_value_box();
	SDL_Rect val =  dm_get_vbox_val_wh();
	int x = output_box.x + ofs + (list_size-1)*(val.w + ofsval);

	value_box_t *first = outputs->first->value;	

	if (first->box.x !=x){
		in_pos = false;
	} else {
		in_pos = true;
	}
	return in_pos;
}

/* Function: draw_input_buffer
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 */
void draw_input_buffer()
{
	int list_size = 0;
	List *inputs = get_input_list();
	int ofs = dm_get_ofs_buffer_value_box();
	int ofsval = dm_get_ofs_between_value_box();
	SDL_Rect val =  dm_get_vbox_val_wh();
	int x = input_box.x + ofs;
	int y = input_box.y + ofs;


	if (inputs != NULL){
		list_size = List_count(inputs);
	}
	if (inputs != NULL && list_size > 0){
		int draw_x = g_input_list_x_pos;
		LIST_FOREACH(inputs, first, next, cur){
			value_box_t *cur_input = cur->value;
			cur_input->box.x = draw_x;
			ax_draw_value_box(cur_input, C_WHITE);
			draw_x += val.w + ofsval;
		}
		if (g_input_list_x_pos > x){
			g_input_list_x_pos -= get_movement_delta(x, g_input_list_x_pos, 
								  BUFFER_MOVEMENT_DELTA);
		}
	}

	dw_draw_rectangle(input_box, C_WHITE);
	SDL_Rect ib = dm_get_stage_ib_text_box();
	dw_draw_texture_fits_height(ib, input_text);
}

/* Function: bf_get_buffer_value_box_x_coord_by_id
 *------------------------------------------------------------------------------
 * Arguments:
 *	op_id: The id of the operand that the x value will get calculated
 *
 * Return:
 *	x coordinate of the first value box
 */
int bf_get_buffer_value_box_x_coord_by_id(int id)
{
	assert(id > BUFFERS_MIN && id < BUFFERS_MAX && "Invalid buffer id");
	
	List *input_list = get_input_list();
	int x;
	int ofs = dm_get_ofs_buffer_value_box();
	if (id == IB){
		x = input_box.x + ofs;
	} else if (id == OB){
		x = output_box.x + ofs;
	}

	return x;
}

/* Function: bf_get_buffer_value_box_y_coord_by_id
 *------------------------------------------------------------------------------
 * Arguments:
 *	op_id: The id of the operand that the x value will get calculated
 *
 * Return:
 *	x coordinate of the first value box
 */
int bf_get_buffer_value_box_y_coord_by_id(int id)
{
	assert(id > BUFFERS_MIN && id < BUFFERS_MAX && "Invalid buffer id");
	
	List *input_list = get_input_list();
	int y;
	int ofs = dm_get_ofs_buffer_value_box();
	if (id == IB){
		y = input_box.y + ofs;	
	} else if (id == OB){
		y = output_box.y - ofs;
	}

	return y;
}

/* Function: bf_set_output_buffer_value_box
 *------------------------------------------------------------------------------
 * Arguments:
 *	op_id: The id of the operand that the x value will get calculated
 *
 * Return:
 *	value_box
 */
void bf_set_output_buffer_value_box(value_box_t val)
{
	List *outputs = get_output_list();
	value_box_t *last = outputs->last->value;
   	
   last->value = val.value;
   last->type = val.type;
   last->visible_box = true;
   dw_free_texture(last->t);
   char *number = ax_number_to_string(val.value);	
   last->t = dw_create_text_texture(number, C_WHITE);
   free(number);
}

/* Function: bf_get_output_buffer_value_box
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	value_box
 */
value_box_t bf_get_output_buffer_value_box()
{
	List *output_list = get_output_list();
	value_box_t *last = output_list->last->value;

	return *last;	
}

/* Function: bf_get_input_buffer_value_box
 *------------------------------------------------------------------------------
 * Arguments:
 *	op_id: The id of the operand that the x value will get calculated
 *
 * Return:
 *	value_box
 */
value_box_t bf_get_input_buffer_value_box()
{
	List *input_list = get_input_list();
	int list_size = List_count(input_list);
	assert(list_size > 0 && "No elements in the list");

	int ofs = dm_get_ofs_buffer_value_box();
	int ofsval = dm_get_ofs_between_value_box();
	SDL_Rect val =  dm_get_vbox_val_wh();
	value_box_t *first = List_shift(input_list);
	g_input_list_x_pos += val.w + ofsval;

	dw_free_texture(first->t);
	first->t = NULL;
	value_box_t ret = *first;
	free(first);
	return ret;	

}
/* Function: bf_reset_input_list_x_pos
 *------------------------------------------------------------------------------
 * Arguments:
 *	void.
 *
 * Return:
 *	void.
 */
void bf_reset_input_list_x_pos()
{
	int screen_width = dm_get_screen_width();
	g_input_list_x_pos = screen_width;
}

/* Function: destroy_input_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	void.
 *
 * Return:
 *	void.
 */
static void destroy_input_list()
{
	LIST_FOREACH(input_list, first, next, cur){
		value_box_t *v = cur->value;
		dw_free_texture(v->t);
    }
	List_clear_destroy(input_list);
	input_list = NULL;
}

/* Function: destroy_output_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	void.
 *
 * Return:
 *	void.
 */
static void destroy_output_list()
{
	LIST_FOREACH(output_list, first, next, cur){
		value_box_t *v = cur->value;
		dw_free_texture(v->t);
    }
	List_clear_destroy(output_list);
	output_list = NULL;
}

/* Function: bf_reset_input_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	void.
 *
 * Return:
 *	void.
 */
void bf_reset_input_list()
{
	destroy_input_list();
	bf_create_input_list();
	bf_create_natural_numbers_input_list(g_input_buffer_size);
	bf_reset_input_list_x_pos();
}

/* Function: bf_reset_output_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	void.
 *
 * Return:
 *	void.
 */
void bf_reset_output_list()
{
	destroy_output_list();
	bf_create_output_list();
	SDL_Rect ib = dm_get_stage_input_buffer_box();
	int ofs = dm_get_ofs_buffer_value_box();
	g_output_list_x_pos = ib.x + ofs;
}

/* Function: bf_destroy_buffer_lists
 *-----------------------------------------------------------------------------
 * This buffer destroy buffer lists when leaving a level.
 *
 * Arguments:
 *	void.
 *
 * Return:
 *	void.
 */
void bf_destroy_buffer_lists()
{
	destroy_input_list();
	destroy_output_list();
}
/* Function: print_output_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	
 */
void print_output_list()
{
	List *output_list = get_output_list();

	assert(output_list != NULL && "Output list pointer is NULL");

	int output_list_size = List_count(output_list);

	printf("The size of the list is %d\n", output_list_size);	

	LIST_FOREACH(output_list, first, next, cur){
		value_box_t *cur_input = cur->value;
		printf("List value: %d\n", cur_input->value);
	}
}

