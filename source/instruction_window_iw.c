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

texture_t *instructions_text;


static List *instruction_list = NULL;
SDL_Rect g_instruction_box;

static List *get_instruction_list();
static void draw_instruction_text();

/* Function: iw_init_ins_box_texture
 *------------------------------------------------------------------------------
 * Creates the instructions texture of the instruction box
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Void.
 */
void iw_init_ins_box_texture()
{
	instructions_text = dw_create_text_texture(INSTRUCTIONS_TEXT, C_AMBER);
}

/* Function: iw_get_instruction_box_by_pos
 *------------------------------------------------------------------------------
 * Returns the box coordinates of an instruction of a given pos
 *
 * Arguments:
 *	pos: The position of the instruction on the instruction list
 *
 * Return:
 *	Lower y value of the instruction
 */
SDL_Rect iw_get_instruction_rect_by_pos(int pos)
{
	int list_size = iw_get_instruction_list_size();
	assert(pos >= 0 && pos < list_size && "Incorrect position");
	
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
   return i->b->r;
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
	SDL_Rect ib = dm_get_stage_instruction_box();
	int x = ib.x;
	int h = get_text_height_fits_width(ib.w, INSTRUCTIONS_TEXT);
	int y = ib.y - h;
	int text_h = dm_get_h_stage_elements_titles();
	SDL_Rect r = {.x = x, .y = y, .w = ib.w, .h = text_h};
	dw_draw_texture_fits_height(r, instructions_text);
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
	   
	   if (bt_btn_clicked(c->b) == true){
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
		if (true == bt_btn_clicked(c->b)){
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
	
	SDL_Rect cb = dm_get_code_button_wh();
	int list_size = List_count(instructions);
	
	SDL_Rect ib = dm_get_stage_instruction_box();
	
	int x = ib.x + dm_get_w_border_padding()+ dm_get_w_padding();
	int y;
	if (list_size == 0){
		y = ib.y + dm_get_h_border_padding() + dm_get_h_padding() + 
			list_size*cb.h;
	} else {
		y = ib.y + 2*dm_get_h_padding() + list_size*cb.h;
	}
		
	SDL_Rect r = dm_get_code_button_wh();
	
	r.x = x;
	r.y = y;
	btn_t *b = bt_create_btn(r, true, false, false, C_BLACK, 
								   C_WHITE, instruction_text);
	check_mem(b);

	instruction_t *new_ins = cl_create_instruction(id, b);

	List_push(instructions, new_ins);
error:
	return;
}

/* Function: iw_draw_instruction_box
 * -----------------------------------------------------------------------------
 * This function displays the instructions available for a level.
 *
 * Arguments:
 *	Void.
 *	
 * Return:
 *	void
 *
 */
void iw_draw_instruction_box()
{
	List *instructions = get_instruction_list();
	assert(instructions != NULL && "Invalid pointer");

	draw_instruction_text();
	
	SDL_Rect r = dm_get_stage_instruction_box();
	
	dw_draw_thick_rect(r, dm_get_w_borders(), C_GREY);

	LIST_FOREACH(instructions, first, next, cur){
		instruction_t *c = cur->value;
		bt_draw_btn(c->b);
	}
}


