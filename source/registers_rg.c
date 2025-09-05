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
#include "arrow_ar.h"
#include "dimensions_dm.h"

#define DEFAULT_OPERAND RAX
#define REG_TEXT "Registers"

static List *register_list = NULL;
static SDL_Rect register_box;

static void set_register_box_member(int value, int member);
List *get_register_list();
reg_t *create_register(int id, button_t *b);
static void destroy_register_list();
static void draw_register_text();
static void draw_register_box();
static void draw_value_boxes();
static void display_arrow_registers();

value_box_t g_ibox;
value_box_t g_obox;

texture_t *reg_text = NULL;
texture_t *g_reg_arrow = NULL;

/* Function: iw_init_reg_texture
 *------------------------------------------------------------------------------
 * Creates the instructions texture of the instruction box
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Void.
 */
void rg_init_reg_texture()
{
	reg_text = dw_create_text_texture(REG_TEXT, C_WHITE);
}

/* Function: display_arrow_registers
 * -----------------------------------------------------------------------------
 * Animate with a moving arrow the registers available for selection
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void display_arrow_registers()
{
	static arrow_t arrow;
	static bool arrow_initialized = false;
	int startx = REG_BOX_X + (REG_BOX_W)*2/5;
	if (arrow_initialized == false){
		arrow.box.x = startx;
		arrow.box.w = ARROW_W;
		arrow.box.h = ARROW_H;
		arrow.texture = g_reg_arrow;
		arrow.startx = startx;
		arrow.starty = arrow.box.y;
		arrow.dir = AR_LEFT;
		arrow.in_place = false;
		arrow_initialized = true;
	}
	SDL_SetTextureColorMod(arrow.texture->texture, 255, 255, 0);
	List *registers = get_register_list();
	assert(registers != NULL && "Invalid pointer");
	int i = 0;
	LIST_FOREACH(registers, first, next, cur){ 
		reg_t *c = cur->value;
		arrow.box.y = c->b->r.y + (CODE_BUTTON_H - arrow.box.h)/2; 
		arrow.travel = startx - (c->b->r.x + c->b->r.w);
		if (i == 0){
			ar_animate_arrow(&arrow);
		} else {
			dw_draw_rotated_texture_fits_h(arrow.box.x, arrow.box.y, 
										arrow.box.h, -180.0, arrow.texture);
		}
		i++;
	}
	
}

/* Function: rg_reset_ibox
 * -----------------------------------------------------------------------------
 * Assigns the value of the ibox to NO_VALUE
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 *
 */
void rg_reset_ibox()
{
	g_ibox.value = NO_VALUE;
}

/* Function: rg_reset_ibox
 * -----------------------------------------------------------------------------
 * Assigns the value of the ibox to NO_VALUE
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 *
 */
void rg_reset_obox()
{
	g_obox.value = NO_VALUE;
}

/* Function: rg_set_ibox_value_box
 * -----------------------------------------------------------------------------
 * Sets the ibox value box
 *
 * Arguments:
 * 	The value box that will be set
 *	
 * Return:
 *	Void.
 *
 */
void rg_set_ibox_value_box(value_box_t value)
{
	g_ibox.value = value.value;
	g_ibox.type = value.type;
	return;
}

/* Function: rg_set_obox_value_box
 * -----------------------------------------------------------------------------
 * Sets the obox value box
 *
 * Arguments:
 * 	The value box that will be set
 *	
 * Return:
 *	Void.
 *
 */
void rg_set_obox_value_box(value_box_t value)
{
	g_obox.value = value.value;
	g_obox.type = value.type;
	return;
}
/* Function: rg_get_ibox_value_box
 * -----------------------------------------------------------------------------
 * Returns the ibox value box
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	ibox value box
 *
 */
value_box_t rg_get_ibox_value_box()
{
	return g_ibox;
}

/* Function: rg_get_ibox_value_box
 * -----------------------------------------------------------------------------
 * Returns the ibox value box
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	ibox value box
 *
 */
value_box_t rg_get_obox_value_box()
{
	return g_obox;
}

/* Function: rg_get_ibox_x
 * -----------------------------------------------------------------------------
 * Returns the x value of the value boxes
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	x_coordinate_of the value box
 *
 */
int rg_get_ibox_x()
{
	return g_ibox.box.x;
}

/* Function: rg_get_ibox_y
 * -----------------------------------------------------------------------------
 * Returns the y value of the ibox
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	y coordinate of the ibox
 *
 */
int rg_get_ibox_y()
{
	return g_ibox.box.y;

}
/* Function: rg_get_obox_y
 * -----------------------------------------------------------------------------
 * Returns the y value of the obox
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	y coordinate of the ibox
 *
 */
int rg_get_obox_y()
{
	return g_obox.box.y;

}
/* Function: rg_initialize_value_boxes
 * -----------------------------------------------------------------------------
 * The value boxes that will serve as an interface for handling values to 
 * the registers are initialized in this function
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 *
 */
void rg_init_value_boxes()
{
	g_ibox.box.x = REG_VBOX_X;
	g_ibox.box.y = rg_get_register_box_member(MEMBER_Y) - VALUE_BOX_H - 
																REG_VBOX_OFFSET;
	g_ibox.box.w = VALUE_BOX_W;
	g_ibox.box.h = VALUE_BOX_H;
	g_ibox.value = NO_VALUE;
	g_ibox.visible_box = true;
	g_ibox.t = dw_create_text_texture(char_dash, C_WHITE);
	
	g_obox.box.x = REG_VBOX_X;
	g_obox.box.y = rg_get_register_box_member(MEMBER_Y) + 
					     rg_get_register_box_member(MEMBER_H) + REG_VBOX_OFFSET;
	g_obox.box.w = VALUE_BOX_W;
	g_obox.box.h = VALUE_BOX_H;
	g_obox.value = NO_VALUE;
	g_obox.visible_box = true;
	g_obox.t = dw_create_text_texture(char_dash, C_WHITE);

	return;
}
/* Function: draw_value_boxes
 * -----------------------------------------------------------------------------
 * Draws the register value boxes with white color
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 *
 */
static void draw_value_boxes()
{
	ax_draw_value_box(&g_ibox, C_MAGENTA);
	ax_draw_value_box(&g_obox, C_CYAN);
	return;
}
/* Function: rg_update_register_box_position
 * -----------------------------------------------------------------------------
 * The final position of the register box will be calculated in the middle of
 * the screen when all the level registers are added. This function calculates
 * the final position that the register is going to have.
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 *
 */
void rg_update_register_box_position()
{
	List *registers = get_register_list();
	check_mem(registers);	
	int list_size = List_count(registers);

	int h = register_box.h;	
	int y = SCREEN_HEIGHT/2 - h/2;
	
	set_register_box_member(y, MEMBER_Y);

	int i = 0;

	LIST_FOREACH(registers, first, next, cur){ 
		reg_t *c = cur->value;
		c->b->r.y = register_box.y + REG_BOX_OFFSET + i*2*(CODE_BUTTON_H + 5) + 
				  CODE_BUTTON_H;
		c->value.box.y = c->b->r.y - CODE_BUTTON_H;
		i++;
   	}

error:
	return;
}

/* Function: destroy_register_list
 *------------------------------------------------------------------------------
 * Destroys the register list and assigns a NULL value;
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	void.
 */
static void destroy_register_list()
{
	List_destroy(register_list);
	register_list = NULL;
}


/* Function: rg_destroy_register_list
 *------------------------------------------------------------------------------
 * Destroys the list of registers and assigns the value of the pointer to NULL.
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	void.
 */
void rg_destroy_register_list()
{
	List *registers = get_register_list();
   	
	LIST_FOREACH(registers, first, next, cur){ 
		reg_t *c = cur->value;
		bt_destroy_button(c->b);
		free(c);
   	}
	destroy_register_list();
}

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
	operand_t *r = rg_create_register_operand_by_id(DEFAULT_OPERAND);
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

/* Function: rg_get_register_box_member
 *------------------------------------------------------------------------------
 * This functions returns a specific member of the instruction_box
 *
 * Arguments:
 *	member: the member of the instruction box that will be returned.
 *
 * Return:
 *	box member requested value
 *
 */
int rg_get_register_box_member(int member)
{
	assert(member >= MEMBER_X && MEMBER_H >= member && "Invalid member");

	int box_member;
	switch(member){
		case MEMBER_X:
			box_member = register_box.x;
			break;
		case MEMBER_Y:
			box_member = register_box.y;
			break;
		case MEMBER_W:
			box_member = register_box.w;
			break;
		case MEMBER_H:
			box_member = register_box.h;
			break;
	}
	return box_member;
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
static void set_register_box_member(int value, int member)
{
	assert(member >= MEMBER_X && MEMBER_H >= member && "Invalid member");

	switch(member){
		case MEMBER_X:
			register_box.x = value;
			break;
		case MEMBER_Y:
			register_box.y = value;
			break;
		case MEMBER_W:
			register_box.w = value;
			break;
		case MEMBER_H:
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
	op->value.box.x = b->r.x + (b->r.w - VALUE_BOX_W)/2;	
	op->value.box.y = b->r.y - CODE_BUTTON_H;
	op->value.box.w = VALUE_BOX_W;
	op->value.box.h = VALUE_BOX_H;
	op->value.value = NO_VALUE;
	op->value.t = dw_create_text_texture(char_dash, C_WHITE);

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
	
	int register_text_w = get_text_width_fits_height(REG_TEXT_H, REG_TEXT);
	int x = 0;
	int y = 0;
	SDL_Rect r = {.x = x, .y = y, .w = CODE_BUTTON_W, .h = CODE_BUTTON_H};
	button_t *b = bt_create_button(r, true, false, false, C_BLACK, C_WHITE,  
																	  reg_text);
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
	
	if (list_size == 0){
		set_register_box_member(REG_BOX_OFFSET + (list_size + 1)*2*CODE_BUTTON_H
												  + REG_BOX_OFFSET, MEMBER_H);
	} else {
		set_register_box_member(REG_BOX_OFFSET + (list_size + 1)*2*CODE_BUTTON_H
												  + 2*REG_BOX_OFFSET, MEMBER_H);

	}

	texture_t *reg_text = cl_create_operand_texture(id);
	check_mem(reg_text);
	
	int register_text_w = get_text_width_fits_height(REG_TEXT_H, REG_TEXT);
	int x = register_box.x + REG_BOX_X_OFFSET;

	int y = register_box.y + REG_BOX_OFFSET + list_size*2*(CODE_BUTTON_H + 5) +
		    REG_TEXT_H + CODE_BUTTON_H;

	SDL_Rect r = {.x = x, .y = y, .w = CODE_BUTTON_W, .h = CODE_BUTTON_H};	
	button_t *b = bt_create_button(r,true, false, false, C_BLACK, C_WHITE, 
																	  reg_text);
	check_mem(b);

	reg_t *new_register = create_register(id, b);
	check_mem(new_register);

	List_push(registers, new_register);

error:
	return;
}

/* Function: rg_draw_registers
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
void rg_draw_registers(bool show_arrows)
{
	List *registers = get_register_list();
	
	assert(registers != NULL && "Invalid pointer");

	draw_register_box();
	draw_register_text();
	draw_value_boxes();
	
	LIST_FOREACH(registers, first, next, cur){
		reg_t *reg = cur->value;
		button_t *button = reg->b;
		bt_draw_button(button);

		char *number = ax_number_to_string(reg->value.value);
		ax_draw_value_box(&reg->value, C_WHITE);
		free(number);
	}
	if (show_arrows == true){
		display_arrow_registers();
	} 
}

/* Function: rg_get_registers_text_width
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
int rg_get_registers_text_width(int h)
{
	assert(h > 0 && "Height value must be greater than zero.");
	int width = get_text_width_fits_height(h, REG_TEXT);
	
	return width;
}

/* Function: rg_set_register_box
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
void rg_set_register_box(SDL_Rect r)
{
	assert(r.w > 0 && r.h > 0 && "Width and height must be > 0");
	register_box.x = r.x;
	register_box.y = r.y;
	register_box.w = r.w;
	register_box.h = r.h;
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
static void draw_register_box()
{
	dw_draw_rectangle(register_box, C_WHITE);
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
static void draw_register_text()
{
	int text_w = get_text_width_fits_height(REG_TEXT_H, REG_TEXT);
	int x = register_box.x;// + (register_box.w - text_w)/2;
	int y = register_box.y - REG_TEXT_H;

	SDL_Rect r = {.x = x, .y = y, .h = REG_TEXT_H};
	dw_draw_texture_fits_height(r, reg_text);
}

/* Function: rg_check_mouse_released_in_register
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

bool rg_check_mouse_released_in_register()
{
	List *registers = get_register_list();
	assert(registers != NULL && "The registers pointer cannot be NULL");
   	int x = List_count(registers);
   	bool selected = false;

   	LIST_FOREACH(registers, first, next, cur){ 
	   
	   	reg_t *c = cur->value;
		SDL_Rect r = {.x = c->value.box.x, .y = c->value.box.y, 
									  .w = c->value.box.w, .h = c->value.box.h};
	  	button_t b = {.r = r};
	   	if (bt_check_mouse_released_button(c->b) == true ||
									bt_check_mouse_released_button(&b) == true){
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
		SDL_Rect r = {.x = c->value.box.x, .y = c->value.box.y, 
									  .w = c->value.box.w, .h = c->value.box.h};

		button_t b = {.r = r};
	   	if (bt_check_mouse_released_button(c->b) == true||
									bt_check_mouse_released_button(&b) == true){
			
			texture_t *t = cl_create_operand_texture(c->id);
			SDL_Rect r = {.x = c->b->r.x, .y = c->b->r.y, .w = c->b->r.w, 
																.h = c->b->r.h};
			button_t *b = bt_create_button(r, c->b->act, c->b->rect, c->b->fill, 
													 	c->b->in, c->b->out, t);

			o = malloc(sizeof(operand_t));
			o->b = b;
			o->id = c->id;
			break;
	   	} 
   	}
   
   return o;
}

int rg_get_register_value_box_x_coord_by_id(int id)
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

int rg_get_register_value_box_y_coord_by_id(int id)
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

/* Function: rg_get_register_value_box_by_id
*------------------------------------------------------------------------------
* Arguments:
*	id: The id of the value box that will be retrieved	
*
* Return:
*	The pointer to the clicked register in an operand form 
*/
value_box_t rg_get_register_value_box_by_id(int id)
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

/* Function: rg_set_value_box_value
 *------------------------------------------------------------------------------
 * Puts a value in a register
 *
 * Arguments:
 *	id: id of the operand that will be set
 *	value: the value that wll be set
 *
 * Return:
 *	void.
 */
void rg_set_register_value_box(int id, value_box_t val)
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


