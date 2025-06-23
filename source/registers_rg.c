#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <assert.h>
#include <stdbool.h>
#include "dbg.h"
#include "registers_rg.h"
#include "draw_dw.h"
#include "aux.h"
#include "code_line_cl.h"
#include "dimensions.h"

enum member {INVALID_MEMBER, X, Y, W, H};

static List *register_list = NULL;

static SDL_Rect register_box;
static char REGISTER_TEXT[] = "Registers";

void set_register_box_member(int value, int member);
List *get_register_list();
reg_t *create_register(int id, button_t *b);

/* Function: reset_register_values
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	void.
 */
void reset_register_values()
{
	List *registers = get_register_list();
	
	assert(registers != NULL && "Invalid pointer");
	
	LIST_FOREACH(registers, first, next, cur){
		reg_t *reg = cur->value;
		reg->value.value = NO_VALUE;
	}

}

/* Function: rg_get_default_operand_register
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Pointer to the object that will be assigned as a default reg operand.
 */
operand_t *rg_get_default_operand_register()
{
	operand_t *r = rg_create_register_operand_by_id(RAX);
	return r;
}

/* Function: get_register_list
 *------------------------------------------------------------------------------
 * This function returns the pointer to the register list.
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 *
 */
List *get_register_list()
{
	return register_list;
}


/* Function: create_register_list
 *------------------------------------------------------------------------------
 * This function creates and initializes the register list.
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 *
 */
void create_register_list()
{
	assert(NULL == register_list && "The register list is not NULL");
	register_list = List_create();
	check_mem(register_list);

error:
	return;
}


/* Function: set_register_box_member
 *------------------------------------------------------------------------------
 * This functions sets a specific member of the instruction_box
 *
 * Arguments:
 *	value: the value that will be set in a given member
 *	member: the member of the instruction box that will be set.
 *
 * Return:
 *	Void.
 *
 */
void set_register_box_member(int value, int member)
{
	assert(member >= X && H >= member && "Invalid member");

	switch(member){
		case X:
			register_box.x = value;
			break;
		case Y:
			register_box.y = value;
			break;
		case W:
			register_box.w = value;
			break;
		case H:
			register_box.h = value;
			break;
	}

	return;
}

/* Function: create_operand
 * -----------------------------------------------------------------------------
 * This function creates a operand object.
 *
 * Arguments:
 * 	id: the id of the instruction that will be created.
 * 	b: the button object of the instruction.
 *
 * Return:
 *	Pointer to the created code node
 */
reg_t *create_register(int id, button_t *b)
{
	assert(NULL != b && "The button pointer is NULL");
	assert(id > REGISTERS_MIN && id < REGISTERS_MAX && "Invalid operand id");
	
	reg_t *op = malloc(sizeof(reg_t));	
	check_mem(op);

	op->b = b;
	op->id = id;
	op->value.box.x = b->x;	
	op->value.box.y = b->y - CODE_BUTTON_H;
	op->value.box.w = VALUE_BOX_W;
	op->value.box.h = VALUE_BOX_H;
	op->value.value = NO_VALUE;

error:
	return op;
}


/* Function: rg_create_reg_op_by_id
 * -----------------------------------------------------------------------------
 * Arguments:
 * 	id: The id of the register operand that will be created.
 *	
 * Return:
 *	The operand of the created register
 *
 */
operand_t *rg_create_register_operand_by_id(int id)
{
	assert(id > REGISTERS_MIN && id < REGISTERS_MAX && "Incorrect id");
	texture_t *reg_text = cl_create_operand_texture(id);
	check_mem(reg_text);
	
	int register_text_w = get_text_width_fits_height(REG_TEXT_H, REGISTER_TEXT);
	int x = 0;
	int y = 0;
	button_t *b = create_button(x, y, CODE_BUTTON_W, CODE_BUTTON_H,
								false, false, reg_text);
	check_mem(b);

	operand_t *op = malloc(sizeof(operand_t));
	
	op->b = b;
	op->id = id;

	error:
	return op;
}



/* Function: rg_add_register_to_list
 * -----------------------------------------------------------------------------
 * Arguments:
 * 	id: The id of the register that will be added to the list.
 *	
 * Return:
 *	void
 *
 */
void rg_add_register_to_list(int id)
{
	List *registers = get_register_list();
	assert(NULL != registers && "Register pointer is NULL");
	assert(id > REGISTERS_MIN && id < REGISTERS_MAX && 
		   "The operand id is invalid");
	
	int list_size = List_count(registers);
	set_register_box_member(2*REG_BOX_OFFSET + (list_size + 1)*2*CODE_BUTTON_H +
							REG_TEXT_H, H);
	
	texture_t *reg_text = cl_create_operand_texture(id);
	check_mem(reg_text);
	
	int register_text_w = get_text_width_fits_height(REG_TEXT_H, REGISTER_TEXT);
	int x = register_box.x + register_box.w/2 - CODE_BUTTON_W/3;
	int y = register_box.y + REG_BOX_OFFSET + list_size*2*(CODE_BUTTON_H + 5) +
		    REG_TEXT_H + CODE_BUTTON_H;
	
	button_t *b = create_button(x, y, CODE_BUTTON_W, CODE_BUTTON_H,
								false, false, reg_text);
	check_mem(b);

	reg_t *new_register = create_register(id, b);
	check_mem(new_register);

	List_push(registers, new_register);

error:
	return;
}

/* Function: display_registers
 * -----------------------------------------------------------------------------
 * This function displays the list of register that will be available in a level
 *
 * Arguments:
 * 	registers: The list of registers that will be displayed
 *	
 * Return:
 *	void
 *
 */
void display_registers()
{
	List *registers = get_register_list();
	
	assert(registers != NULL && "Invalid pointer");

	draw_register_box();
	
	LIST_FOREACH(registers, first, next, cur){
		reg_t *reg = cur->value;
		button_t *button = reg->b;
		bt_draw_button(button);

		char *number = number_to_string(reg->value.value);
		draw_value_box(&reg->value);
		free(number);

	}
}

/* Function: get_registers_text_width
 *------------------------------------------------------------------------------
 * This function returns the width of the registers text drawn with a specficic
 * height.
 *
 * Arguments:
 *	h: height of the drawn registers text
 * 
 * Return:
 *	The width of the text of the registers text
 */
int get_registers_text_width(int h)
{
	assert(h > 0 && "Height value must be greater than zero.");
	int width = get_text_width_fits_height(h, REGISTER_TEXT);
	
	return width;
}

/* Function: set_register_box
 *------------------------------------------------------------------------------
 * This funcion sets the dimensions of the register box
 *
 * Arguments:
 *	x: x coordinate.
 *	y: y coordinate.
 * 	w: width of the box.
 *	h: height of the box.
 * 
 * Return:
 *	Void.
 */
void set_register_box(int x, int y, int w, int h)
{
	assert(w > 0 && h > 0 && "Width and height must be > 0");
	register_box.x = x;
	register_box.y = y;
	register_box.w = w;
	register_box.h = h;
}

/* Function: draw_register_box
 *------------------------------------------------------------------------------
 * This function draws the box where the registers will be contained
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 */
void draw_register_box()
{
	draw_rectangle(register_box.x, register_box.y, register_box.w, 
				   register_box.h, COLOR_WHITE);
}

/* Function: draw_register_text
 *------------------------------------------------------------------------------
 * This functions receives a pair of coordinates where it will draw the register
 * text
 *
 * Arguments:
 *	x: the x coordinate of the text
 *	y: the y coordinate of the text
 *	h: height in pixels of the text to be drawn
 * 
 * Return:
 *	Void.
 *
 */
void draw_register_text(int x, int y, int h)
{
	draw_text_fit_height(x, y, h, COLOR_WHITE, REGISTER_TEXT);

}

/* Function: rg_check_released_in_register
*------------------------------------------------------------------------------
* This functions verifies if the user clicked one of the registers available
* in the screen
*
* Arguments:
* 	registers: the list of registers to be verified
*	
* Return:
*	true if the user clicked over an instructions, false if otherwise
*
*/

bool rg_check_released_in_register()
{
	List *registers = get_register_list();
	assert(registers != NULL && "The registers pointer cannot be NULL");
   	int x = List_count(registers);
   	bool selected = false;

   	LIST_FOREACH(registers, first, next, cur){ 
	   
	   	reg_t *c = cur->value;
	   
	   	if (check_mouse_released_in_button(c->b) == true){
		   	selected = true;
		   	break;
	   	} 
   	}
   	return selected;
}

/* Function: rg_greate_operand_of_selected_register
*------------------------------------------------------------------------------
* Arguments:
*	
* Return:
*	The pointer to the clicked register in an operand form 
*
*/
operand_t *rg_create_operand_of_selected_register()
{
	List *registers = get_register_list();
   	int x = List_count(registers);
	operand_t *o = NULL;

   	LIST_FOREACH(registers, first, next, cur){ 

		reg_t *c = cur->value;
	   
	   	if (true == check_mouse_released_in_button(c->b)){
			
			texture_t *t = cl_create_operand_texture(c->id);
			button_t *b = create_button(c->b->x, c->b->y, c->b->w, c->b->h, 
							  	c->b->active, c->b->rectangle, t);

			o = malloc(sizeof(operand_t));
			o->b = b;
			o->id = c->id;
			break;
	   	} 
   	}
   
   return o;
}

int get_register_value_box_x_coord_by_id(int id)
{
	assert(id > REGISTERS_MIN && id < REGISTERS_MAX && 
		   "Invalid register id");

	List *registers = get_register_list();
	reg_t *c;
   	
	LIST_FOREACH(registers, first, next, cur){ 
		c = cur->value;
	   
	   	if (id == c->id){
			break;
	   	} 
   	}
   return c->value.box.x;
}

int get_register_value_box_y_coord_by_id(int id)
{
	assert(id > REGISTERS_MIN && id < REGISTERS_MAX && 
		   "Invalid register id");

	List *registers = get_register_list();
	reg_t *c;
   	
	LIST_FOREACH(registers, first, next, cur){ 
		c = cur->value;
	   
	   	if (id == c->id){
			break;
	   	} 
   	}
   return c->value.box.y;
}

/* Function: get_register_value_box_by_id
*------------------------------------------------------------------------------
* Arguments:
*	id: The id of the value box that will be retrieved	
*
* Return:
*	The pointer to the clicked register in an operand form 
*/
value_box_t get_register_value_box_by_id(int id)
{
	assert(id > REGISTERS_MIN && id < REGISTERS_MAX && 
		   "Invalid register id");

	List *registers = get_register_list();
	reg_t *c;
   	
	LIST_FOREACH(registers, first, next, cur){ 
		c = cur->value;
	   
	   	if (id == c->id){
			break;
	   	} 
   	}
   return c->value;
}

/* Function: set_value_box_value
 *------------------------------------------------------------------------------
 * Arguments:
 *	id: id of the operand that will be set
 *	value: the value that wll be set
 *
 * Return:
 *	Pointer to the object that will be assigned as a default reg operand.
 */
void set_register_value_box(int id, value_box_t val)
{
	assert(id > REGISTERS_MIN && id < REGISTERS_MAX && 
		   "Invalid register id");

	List *registers = get_register_list();
	reg_t *c;
   	
	LIST_FOREACH(registers, first, next, cur){ 
		c = cur->value;
	   
	   	if (id == c->id){
			break;
	   	} 
   	}
   c->value.value = val.value;
   c->value.type = val.type;
}


