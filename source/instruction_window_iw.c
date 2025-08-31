#include <assert.h>
#include <stdbool.h>
#include <SDL.h>
#include "instruction_window_iw.h"
#include "list.h"
#include "button_bt.h"
#include "code_line_cl.h"
#include "dbg.h"
#include "dimensions_dm.h"

#define INSTRUCTIONS_TEXT "Instructions"

static List *instruction_list = NULL;
SDL_Rect instruction_box;

void set_instruction_box_member(int value, int member);
static List *get_instruction_list();
static void draw_instruction_text();

/* Function: iw_get_instruction_y_by_pos
 *------------------------------------------------------------------------------
 * This function returns THE LOWER y value of an instruction
 *
 * Arguments:
 *	pos: The position of the instruction on the instruction list
 *
 * Return:
 *	Lower y value of the instruction
 */
int iw_get_instruction_y_by_pos(int pos)
{
	int list_size = iw_get_instruction_list_size();
	assert(pos >= 0  && pos < list_size && "Incorrect position");

	List *instructions = get_instruction_list();
	instruction_t *i;
	int count = 0;
	LIST_FOREACH(instructions, first, next, cur){ 
		i = cur->value;
		if (count == pos){
			break;
	   	}
   }
   assert(i != NULL && "null pointer returned for instruction");
   return i->b->r.y + i->b->r.h;
}


/* Function: iw_get_instruction_y_by_id
 *------------------------------------------------------------------------------
 * This function returns THE LOWER y value of an instruction
 *
 * Arguments:
 *	id: the id of the instruction 
 *
 * Return:
 *	Lower y value of the instruction
 */
int iw_get_instruction_y_by_id(int id)
{
	assert(id > INSTRUCTION_MIN && id < INSTRUCTION_MAX && "Invalid id");

	List *instructions = get_instruction_list();

	instruction_t *i;
	LIST_FOREACH(instructions, first, next, cur){ 
		i = cur->value;
		if (id == i->id){
			break;
	   	}
   }
   assert(i != NULL && "null pointer returned for instruction");
   return i->b->r.y + i->b->r.h;
}

/* Function: draw_instruction_text
 *------------------------------------------------------------------------------
 * Drasw the instruction text that is above the instruction box.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Void.
 */
static void draw_instruction_text()
{
	int x = instruction_box.x;
	int y = instruction_box.y  - get_text_height_fits_width(INS_BOX_TEXT_W, 
															INSTRUCTIONS_TEXT);
	dw_draw_text_fits_width(x, y, INS_BOX_TEXT_W, C_WHITE, 
															INSTRUCTIONS_TEXT);
}

/* Function: iw_get_instruction_list_size
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	int: size of the instruction list
 */
int iw_get_instruction_list_size()
{
	assert(instruction_list != NULL);
	List *instructions = get_instruction_list();
	int instruction_count = List_count(instructions);
	return instruction_count;
}

/* Function: iw_create_instruction_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 *
 */
void iw_create_instruction_list()
{
	assert(instruction_list == NULL && "The register list is not NULL");
	instruction_list = List_create();
	return;
}

/* Function: iw_destroy_instruction_list
 *------------------------------------------------------------------------------
 * Destroys the instruction list when leaving a level.
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 *
 */
void iw_destroy_instruction_list()
{
	List *instructions = get_instruction_list();
	LIST_FOREACH(instructions, first, next, cur){ 
	   
		instruction_t *c = cur->value;
	  	bt_destroy_button(c->b);
	  	free(c);
   	}
	List_destroy(instruction_list);
	instruction_list = NULL;
}
/* Function: get_instruction_list
 *------------------------------------------------------------------------------
 * This function returns the pointer to the instruction list.
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 *
 */
static List *get_instruction_list()
{
	return instruction_list;
}

/* Function: set_instruction_box_member
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
void set_instruction_box_member(int value, int member)
{
	assert(member >= MEMBER_X && MEMBER_H >= member && "Invalid member");

	switch(member){
		case MEMBER_X:
			instruction_box.x = value;
			break;
		case MEMBER_Y:
			instruction_box.y = value;
			break;
		case MEMBER_W:
			instruction_box.w = value;
			break;
		case MEMBER_H:
			instruction_box.h = value;
			break;
	}
	return;
}

/* Function: iw_set_instruction_box
 *------------------------------------------------------------------------------
 * This functions sets the parameters that will be used in the instruction
 * box
 *
 * Arguments:
 *	x: x coordinate of the box;
 *	y: y coordinate of the box;
 *	w: width of the box
 *	h: height of the box
 *
 * Return:
 *	Void.
 *
 */
void iw_set_instruction_box(int x, int y, int w, int h)
{
	assert(w > 0 && h > 0 && "Invalid width and height values");

	instruction_box.x = x;
	instruction_box.y = y;
	instruction_box.w = w;
	instruction_box.h = h;

	return;
}

/* Function: iw_check_clicked_instruction
*------------------------------------------------------------------------------
* This functions verifies if the user clicked one of the instructions available
* in the screen
*
* Arguments:
* 	instructions: the list of instructions.
*	
* Return:
*	true if the user clicked over an instructions, false if otherwise
*
*/

bool iw_check_clicked_instruction()
{
	List *instructions = get_instruction_list();
   	int x = List_count(instructions);
   	bool clicked = false;

   	LIST_FOREACH(instructions, first, next, cur){ 
	   
	   instruction_t *c = cur->value;
	   
	   if (bt_check_mouse_click_button(c->b) == true){
		   clicked = true;
		   break;
	   } 
   }
   
   return clicked;
}

/* Function: iw_get_clicked_instruction
 *------------------------------------------------------------------------------
 * This function returns the pointer of the selected instruction
 *
 * Arguments:
 * 	None.
 *	
 * Return:
 *	the pointer to the specific instruction that has been clicked.
 *	NULL if not button was clicked
 *
 */
instruction_t *iw_get_clicked_instruction()
{
	List *instructions = get_instruction_list();
	assert(NULL != instructions && "The instruction pointer cannot be NULL");
	int x = List_count(instructions);
	LIST_FOREACH(instructions, first, next, cur){ 
		
		instruction_t *c = cur->value;
		if (true == bt_check_mouse_click_button(c->b)){
			return c;
		} 
	}
	return NULL;
}

/* Function: iw_add_instruction_to_list
 *-----------------------------------------------------------------------------
 * This function simplifies the construction of the list of instructions that
 * will be available to the player in a given stage. This function assigns the
 * adecuate and corresponding coordinates based on the position of the 
 * instruction box.
 *
 * Arguments:
 * 	id: The id of the instruction that will be added to the list.
 *	
 * Return:
 *	void
 *
 */
void iw_add_instruction_to_list(int id)
{
	List *instructions = get_instruction_list();

	assert(NULL != instructions && "Instruction pointer is NULL");
	
	char *text = cl_get_instruction_text(id);
	texture_t *instruction_text = dw_create_text_texture(text, 
								  C_WHITE);
	
	int list_size = List_count(instructions);
	int x = INS_BOX_X + INS_BOX_OFFSET;
	int y = INS_BOX_Y + INS_BOX_OFFSET + list_size*CODE_BUTTON_H;
	SDL_Rect r = dm_get_code_button_box();
	r.x = x;
	r.y = y;
	button_t *b = bt_create_button(r, true, false, false, C_BLACK, 
								   C_WHITE, instruction_text);
	check_mem(b);
	set_instruction_box_member(2*INS_BOX_OFFSET + (list_size + 1)*CODE_BUTTON_H,
							   MEMBER_H);

	instruction_t *new_ins = cl_create_instruction(id, b);

	List_push(instructions, new_ins);

error:
	return;
}

/* Function: iw_display_instructions
 * -----------------------------------------------------------------------------
 * This function displays the instructions available for a level.
 *
 * Arguments:
 *	x: The x position of the instruction list
 *	y: The y position of the instruction list
 *	
 * Return:
 *	void
 *
 */
void iw_display_instructions(int x, int y)
{
	List *instructions = get_instruction_list();
	assert(instructions != NULL && "Invalid pointer");
	assert(x >= 0 && y >= 0 && "Invalid position coordinate"); 

	draw_instruction_text();
	// The rectangle the contains the list of buttons
	dw_draw_rectangle(instruction_box.x, instruction_box.y, 
				   instruction_box.w, instruction_box.h, C_WHITE);
	
	LIST_FOREACH(instructions, first, next, cur){
		instruction_t *c = cur->value;
		bt_draw_button(c->b);
		}
}


