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
#include "dimensions.h"
#include "button_bt.h"
#include "code_line_cl.h"

// The posible types of input values
#define WHOLE 0
#define ASCII 1
#define DEFAULT_BUFFER_SIZE 5

#define NATURAL_NMAX 9
#define NATURAL_NMIN 1

#define ARROW_INPUT_X (BUFFER_BOX_X - 3*ARROW_W)
#define ARROW_INPUT_Y (INPUT_BUFFER_BOX_Y + (BUFFER_BOX_H - ARROW_H)/2)

#define ARROW_OUTPUT_X (BUFFER_BOX_X - 3*ARROW_W)
#define ARROW_OUTPUT_Y (OUTPUT_BUFFER_BOX_Y + (BUFFER_BOX_H - ARROW_H)/2)

SDL_Rect input_box;
SDL_Rect output_box;

static List *input_list = NULL;
static List *output_list = NULL;

button_t input_buffer_button;
button_t output_buffer_button;

operand_t *input_buffer = NULL;
operand_t *output_buffer = NULL;

static char input_text[] = "Input Buffer [IB]";
static char output_text[] = "Output Buffer [OB]";

static bool g_win_condition;

static int g_input_list_x_pos = SCREEN_WIDTH;
static int g_output_list_x_pos = BUFFER_BOX_X + BUFFER_VALUE_OFFSET_X;
int g_input_list_type = NOT_ASSIGNED;  
int g_input_buffer_size = DEFAULT_BUFFER_SIZE;
void add_input_to_list(int value, int type);
void draw_output_buffer();
void draw_input_buffer();
static bool check_win_condition();
static void display_input_arrow();
static void display_output_arrow();

texture_t *g_input_arrow = NULL;
texture_t *g_output_arrow = NULL;


/* Function: display_output_arrow
 * -----------------------------------------------------------------------------
 * Animate with a moving arrow of the output buffer when available for selection
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void display_output_arrow()
{
	static arrow_t arrow;
	static bool arrow_initialized = false;
	int startx = ARROW_INPUT_X;
	if (arrow_initialized == false){
		arrow.box.x = startx;
		arrow.box.y = ARROW_OUTPUT_Y;
		arrow.box.w = ARROW_W;
		arrow.box.h = ARROW_H;
		arrow.texture = g_input_arrow;
		arrow.in_place = false;
		arrow_initialized = true;
	}
	int travel = BUFFER_BOX_X - startx - ARROW_W;
	SDL_SetTextureColorMod(arrow.texture->texture, 0, 255, 255);
	dw_animate_arrow(startx, arrow.box.y, &arrow, DW_RIGHT, travel);
}

/* Function: display_input_arrow_
 * -----------------------------------------------------------------------------
 * Animate with a moving arrow of the input buffer when available for selection
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void display_input_arrow()
{
	static arrow_t arrow;
	static bool arrow_initialized = false;
	int startx = ARROW_INPUT_X;
	if (arrow_initialized == false){
		arrow.box.x = startx;
		arrow.box.y = ARROW_INPUT_Y;
		arrow.box.w = ARROW_W;
		arrow.box.h = ARROW_H;
		arrow.texture = g_input_arrow;
		arrow.in_place = false;
		arrow_initialized = true;
	}
	int travel = BUFFER_BOX_X - startx - ARROW_W;
	SDL_SetTextureColorMod(arrow.texture->texture, 255, 0, 0);
	dw_animate_arrow(startx, arrow.box.y, &arrow, DW_RIGHT, travel);
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
		b->b = create_button(input_buffer->b->x, input_buffer->b->y, 
							 input_buffer->b->w, input_buffer->b->h, 
							 input_buffer->b->active, 
							 input_buffer->b->rectangle, t);
		b->id = input_buffer->id;
	} else if (id == OB){
		texture_t *t = cl_create_operand_texture(id);
		b->b = create_button(output_buffer->b->x, output_buffer->b->y, 
							 output_buffer->b->w, output_buffer->b->h, 
							 output_buffer->b->active, 
							 output_buffer->b->rectangle, t);
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
		b->b = create_button(input_buffer->b->x, input_buffer->b->y, 
							 input_buffer->b->w, input_buffer->b->h, 
							 input_buffer->b->active, 
							 input_buffer->b->rectangle, t);

		b->id = input_buffer->id;
	} 
   	
	if (bt_check_mouse_released_button(&output_buffer_button) == true){
		b = malloc(sizeof(operand_t));
		texture_t *t = cl_create_operand_texture(OB);
		b->b = create_button(output_buffer->b->x, output_buffer->b->y, 
							 output_buffer->b->w, output_buffer->b->h, 
							 output_buffer->b->active, 
							 output_buffer->b->rectangle, t);
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
	int x = 0;
	int y = 0;

	texture_t *ib = load_texture_from_rendered_text(ib_text, COLOR_WHITE);
	check_mem(ib);
	button_t *a = create_button(x, y, CODE_BUTTON_W, CODE_BUTTON_H,
								true, false, ib);
	check_mem(a);

	input_buffer = cl_create_operand(IB, a);
	check_mem(input_buffer);

	
	texture_t *ob = load_texture_from_rendered_text(ob_text, COLOR_WHITE);
	check_mem(ob);
	button_t *b = create_button(x, y, CODE_BUTTON_W, CODE_BUTTON_H,
								true, false, ob);
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
void bf_set_input_buffer_button(int x, int y, int w, int h)
{
	assert(w > 0 && h > 0 && "Width and height must be > 0");
	input_buffer_button.x = x;
	input_buffer_button.y = y;
	input_buffer_button.w = w;
	input_buffer_button.h = h;
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
void bf_set_output_buffer_button(int x, int y, int w, int h)
{
	assert(w > 0 && h > 0 && "Width and height must be > 0");
	output_buffer_button.x = x;
	output_buffer_button.y = y;
	output_buffer_button.w = w;
	output_buffer_button.h = h;
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
	new_output->box.x = BUFFER_BOX_X + BUFFER_VALUE_OFFSET_X;
	int y_offset = (output_box.h - VALUE_BOX_H)/2;
	new_output->box.y = output_box.y + y_offset;
	new_output->box.w = VALUE_BOX_W;
	new_output->box.h = VALUE_BOX_H;
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

	new_input->value = value;
	new_input->type = type;
	new_input->box.x = SCREEN_WIDTH;
	int y_offset = (input_box.h - VALUE_BOX_H)/2;
	new_input->box.y = input_box.y + y_offset;
	new_input->box.w = VALUE_BOX_W;
	new_input->box.h = VALUE_BOX_H;

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
		int value = rand() % NATURAL_NMAX - NATURAL_NMIN + NATURAL_NMIN;
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
void bf_set_input_box(int x, int y, int w, int h)
{
	assert(w > 0 && h > 0 && "Width and height must be > 0");
	input_box.x = x;
	input_box.y = y;
	input_box.w = w;
	input_box.h = h;
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
void bf_set_output_box(int x, int y, int w, int h)
{
	assert(w > 0 && h > 0 && "Width and height must be > 0");
	output_box.x = x;
	output_box.y = y;
	output_box.w = w;
	output_box.h = h;
}

/* Function: draw_buffers
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 * 
 * Return:
 *	Void.
 */
void bf_draw_buffers(int operand_id)
{
	if (operand_id == IB){
		display_input_arrow();
	}
	if (operand_id == OB){
		display_output_arrow();
	}
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
	if (check_win_condition() == true){
		x = OUTPUT_BUFFER_WIN_X;	
	} else {
		x = output_box.x + BUFFER_VALUE_OFFSET_X + (list_size-1)*(VALUE_W + 
														BETWEEN_NUMBERS_OFFSET);
	}
	int y = output_box.y + BUFFER_VALUE_OFFSET_Y;
	if (outputs != NULL && list_size > 0){
		int draw_x = g_output_list_x_pos;
		LIST_FOREACH(outputs, first, next, cur){
			value_box_t *cur_output = cur->value;
			cur_output->box.x = draw_x;

			if (cur_output->visible_box != false){
				ax_draw_value_box(cur_output, COLOR_WHITE);
			}
			draw_x -= VALUE_W + BETWEEN_NUMBERS_OFFSET;
		}
		if (g_output_list_x_pos < x){
			g_output_list_x_pos += get_movement_delta(x, g_output_list_x_pos, 
								   						BUFFER_MOVEMENT_DELTA);
		}
	}
	dw_draw_rectangle(output_box.x, output_box.y, output_box.w, output_box.h,
				   COLOR_WHITE);
	dw_draw_text_fits_height(OUTPUT_BUFFER_TEXT_X, OUTPUT_BUFFER_TEXT_Y, 
						 BUFFER_TEXT_H, COLOR_WHITE, output_text);
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

	int x = output_box.x + BUFFER_VALUE_OFFSET_X + 
			(list_size-1)*(VALUE_W + BETWEEN_NUMBERS_OFFSET);

	value_box_t *first = outputs->first->value;	

	//if (g_output_list_x_pos < x || g_output_list_x_pos > x){
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
	int x = input_box.x + BUFFER_VALUE_OFFSET_X;
	int y = input_box.y + BUFFER_VALUE_OFFSET_Y;


	if (inputs != NULL){
		list_size = List_count(inputs);
	}
	if (inputs != NULL && list_size > 0){
		int draw_x = g_input_list_x_pos;
		LIST_FOREACH(inputs, first, next, cur){
			value_box_t *cur_input = cur->value;
			cur_input->box.x = draw_x;
			ax_draw_value_box(cur_input, COLOR_WHITE);
			draw_x += VALUE_W + BETWEEN_NUMBERS_OFFSET;
		}
		if (g_input_list_x_pos > x){
			g_input_list_x_pos -= get_movement_delta(x, g_input_list_x_pos, 
								  BUFFER_MOVEMENT_DELTA);
		}
	}

	dw_draw_text_fits_height(INPUT_BUFFER_TEXT_X, INPUT_BUFFER_TEXT_Y, 
						 BUFFER_TEXT_H, COLOR_WHITE, input_text);
	dw_draw_rectangle(input_box.x, input_box.y, input_box.w, input_box.h, 
				   COLOR_WHITE);
	
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
	if (id == IB){
		x = input_box.x + BUFFER_VALUE_OFFSET_X;
	} else if (id == OB){
		x = output_box.x + BUFFER_VALUE_OFFSET_X;
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
	if (id == IB){
		y = input_box.y + BUFFER_VALUE_OFFSET_Y;	
	} else if (id == OB){
		y = output_box.y - BUFFER_VALUE_OFFSET_Y;
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
	//int list_size = List_count(input_list);
	//assert(list_size > 0 && "No elements in the list");

	value_box_t *last = output_list->last->value;
//	g_output_list_x_pos += VALUE_W + BETWEEN_NUMBERS_OFFSET;

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

	value_box_t *first = List_shift(input_list);
	g_input_list_x_pos += VALUE_W + BETWEEN_NUMBERS_OFFSET;

	return *first;	

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
	g_input_list_x_pos = SCREEN_WIDTH;
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
	List_clear_destroy(input_list);
	input_list = NULL;
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
	List_clear_destroy(output_list);
	output_list = NULL;
	bf_create_output_list();
	g_output_list_x_pos = BUFFER_BOX_X + BUFFER_VALUE_OFFSET_X;
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
	List_clear_destroy(input_list);
	List_clear_destroy(output_list);
	input_list = NULL;
	output_list = NULL;
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

