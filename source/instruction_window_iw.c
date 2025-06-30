#include <assert.h>
#include <stdbool.h>
#include <SDL.h>
#include "instruction_window_iw.h"
#include "list.h"
#include "button_bt.h"
#include "code_line_cl.h"
#include "dbg.h"
#include "dimensions.h"

enum member {INVALID_MEMBER, X, Y, W, H};

static List *instruction_list = NULL;
SDL_Rect instruction_box;

void set_instruction_box_member(int value, int member);
static List *get_instruction_list();
static int get_instruction_list_size();

/* Function: get_instruction_list_size
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	int: size of the instruction list
 */
static int get_instruction_list_size()
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
	assert(member >= X && H >= member && "Invalid member");

	switch(member){
		case X:
			instruction_box.x = value;
			break;
		case Y:
			instruction_box.y = value;
			break;
		case W:
			instruction_box.w = value;
			break;
		case H:
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
	   
	   if (check_mouse_click_in_button(c->b) == true){
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
		if (true == check_mouse_click_in_button(c->b)){
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
	texture_t *instruction_text = load_texture_from_rendered_text(text, 
								  COLOR_WHITE);
	
	int list_size = List_count(instructions);
	int x = INS_BOX_X + INS_BOX_OFFSET;
	int y = INS_BOX_Y + INS_BOX_OFFSET + list_size*CODE_BUTTON_H;
	button_t *b = create_button(x, y, CODE_BUTTON_W, CODE_BUTTON_H,
								false, false, instruction_text);
	check_mem(b);
	set_instruction_box_member(2*INS_BOX_OFFSET + (list_size + 1)*CODE_BUTTON_H,
							   H);

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

	// The rectangle the contains the list of buttons
	draw_rectangle(instruction_box.x, instruction_box.y, 
				   instruction_box.w, instruction_box.h, COLOR_WHITE);
	
	LIST_FOREACH(instructions, first, next, cur){
		instruction_t *c = cur->value;
		bt_draw_button(c->b);
		}
}


