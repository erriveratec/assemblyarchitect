#include <SDL.h>
#include <assert.h>
#include <string.h>
#include "code_window_cw.h"
#include "list.h"
#include "mouse_ms.h"
#include "dbg.h"
#include "dimensions_dm.h"
#include "buffers_bf.h"
#include "registers_rg.h"
#include "draw_dw.h"

#define NOT_FOUND -1
#define CODE_LINES_SIZE 5

static List *code_list = NULL;

static SDL_Rect code_box;
static SDL_Rect text_box;

texture_array_t *g_challenge_text;
texture_t *g_stage_name;
texture_t *g_comma_tex;
texture_t **g_numbers;

void display_line_number();
int get_code_line_position(int y);
ListNode *get_list_node_by_value(code_line_t *line);
static bool check_if_inside_code_window();
static bool check_selected_line_in_position(code_line_t *line);
static void set_text_box(int x, int y, int w, int h);
static void display_player_code();
static void add_code_line(code_line_t *line);
static void set_code_box_member(int value, int member);
static void set_text_box_member(int value, int member);
static void code_box_height_adjust();
static void adjust_code_box_position();
static int get_label_operand_value(code_line_t *line);
List *get_code_list();
static operand_t *create_label_operand(code_line_t *line);
static operand_t *create_saved_label_operand(int op1_id);
static int label_counter_up_to_index(int index);
static void update_label_instructions();
static void update_jump_instructions();
static operand_t *create_updated_jump_operand(code_line_t *jmp_addr);
static code_line_t *get_clicked_label_code_line();
static operand_t *create_saved_jump_operand(int op1_id);
static SDL_Rect get_code_box();

/* Function: cw_init_code_window_texture
 * -----------------------------------------------------------------------------
 * Inits the textures for the code windows
 * 
 * Arguments:
 *  Void.
 *
 * Return:
 * 	Void.
 */
void cw_init_code_window_texture()
{
	g_comma_tex = dw_create_text_texture(",", C_WHITE);
	g_numbers = malloc(sizeof(texture_t*)*MAX_CODE_LINES);
	for (int i = 0; i <= MAX_CODE_LINES; i++){
		char *number = ax_number_to_string_two_digits(i);
		g_numbers[i] = dw_create_text_texture(number, C_GREY);
		free(number);
	}
}

/* Function: cw_operate_jump_instruction
 * -----------------------------------------------------------------------------
 * Changes the status of the code lines between the jump and the destiny label,
 * if the destiny lable is previous to the jump instruction, the status is 
 * change to COMPLETE, if the destiny label is after the jump instruction.
 * all the instructions until that label are changed to EXECUTED
 * 
 * Arguments:
 *  void.
 *
 * Return:
 * 	void.
 */
void cw_operate_jump_instruction(code_line_t *line)
{
	assert(line->ins->id == JMP && "Invalid instruction for jump");

	List *code = get_code_list();
	check_mem(code);
	
	int jmp_pos = cw_get_code_line_pos_by_ptr(line);
	int dst_pos = cw_get_code_line_pos_by_ptr(line->op1->jptr);

	int i = 0;
	code_line_t *c;
	if (dst_pos < jmp_pos){
		LIST_FOREACH(code, first, next, cur){ 
			c = cur->value;
			if (i >= dst_pos && i <= jmp_pos){
				c->state = COMPLETE;
			}
			i++;
		}
	} else if (dst_pos > jmp_pos){
		LIST_FOREACH(code, first, next, cur){ 
			c = cur->value;
			if (i < dst_pos && i >= jmp_pos){
				c->state = EXECUTED;
			}
			i++;
		}	
	}
error:
	return;
}

/* Function: cw_update_saved_jump_instructions
 * -----------------------------------------------------------------------------
 * Traverses the whole code list and updates all the jump instructions
 * when all the saved instructions are completely loaded in the code.
 * 
 * Arguments:
 *  void.
 *
 * Return:
 * 	void.
 */
void cw_update_saved_jump_instructions()
{
	List *code = get_code_list();
	check_mem(code);

	int op1_ofs = dm_get_ofs_code_op1();
	code_line_t *c;
	LIST_FOREACH(code, first, next, cur){ 
		c = cur->value;
		if (c->ins->id == JMP){
			assert(c->op1 != NULL && "op1 should't be NULL");
			code_line_t  *jmp_addr = cw_get_code_line_at_pos(c->op1->id);
			cl_destroy_operand(c->op1);	
			assert(cw_check_if_in_code_list(jmp_addr) == true && "Instruction"
				   "not present in code list");
			c->op1 = create_updated_jump_operand(jmp_addr);
			c->op1->b->r.x = c->ins->b->r.x + op1_ofs;
			c->op1->b->r.y = c->ins->b->r.y;
		}
	}
error:
	return;
}

/* Function: create_saved_jump_operand
*------------------------------------------------------------------------------
* Creates a placeholder jump operand in before the whole code loads
*
* Arguments:
* 	op1_id: the id of the operand what will be created
*	
* Return:
*	Pointer to the newly created operand
*
*/
static operand_t *create_saved_jump_operand(int op1_id)
{
   	operand_t *b = NULL;
	b = malloc(sizeof(operand_t));

	SDL_Rect cb = dm_get_code_button_wh();

	char *line_text = ax_number_to_string_two_digits(op1_id);
	char *op_text = malloc(sizeof(char)*(strlen(line_text)+1));
	strcpy(op_text, line_text);
	texture_t *t = dw_create_text_texture(op_text, C_WHITE);

	SDL_Rect r = {.x = 0, .y = 0, .w = 2*cb.w, .h = cb.h};
	b->b = bt_create_button(r, false, false, false, C_BLACK, 
							C_WHITE, t);

	b->id = op1_id;
	free(line_text);
	free(op_text);
	return b;
}

/* Function: create_updated_jump_operand
*------------------------------------------------------------------------------
* Creates the label operand with the corresponding number that will be shown
* in the code
*
* Arguments:
* 	None.
*	
* Return:
*	Pointer to the newly created operand
*
*/
static operand_t *create_updated_jump_operand(code_line_t *jmp_addr)
{
	assert(cw_check_if_in_code_list(jmp_addr) == true && "The code line is not"
	"in the list and is probably destroyed");

   	operand_t *op = NULL;
	op = malloc(sizeof(operand_t));

	int addr_label_id = jmp_addr->op1->id;
	char *line_text = ax_number_to_string_two_digits(addr_label_id);
	char *op_text = malloc(sizeof(char)*(strlen(label_text) + 
						   strlen(char_space)+ strlen(line_text)));
	
	strcpy(op_text, label_text);
	strcat(op_text, char_space);
	strcat(op_text, line_text);
	texture_t *t = dw_create_text_texture(op_text, C_WHITE);

	SDL_Rect cb = dm_get_code_button_wh();
	SDL_Rect r = {.x = 0, .y = 0, .w = 2*cb.w, .h = cb.h};
	op->b = bt_create_button(r, false, false, false, C_BLACK, C_WHITE, t);
	op->id = cw_get_code_line_pos_by_ptr(jmp_addr);
	op->jptr = jmp_addr;

	free(line_text);
	free(op_text);

error:
	return op;
}

/* Function: update_jump_instructions
 * -----------------------------------------------------------------------------
 * Traverses the whole code list and updates all the jump instructions
 * to match the labels correctly
 * 
 * Arguments:
 *  void.
 *
 * Return:
 * 	void.
 */
static void update_jump_instructions()
{
	List *code = get_code_list();
	check_mem(code);

	int op1_ofs = dm_get_ofs_code_op1();
	code_line_t *c;
	LIST_FOREACH(code, first, next, cur){ 
		c = cur->value;
		if (c->ins->id == JMP){
			if (c->op1 != NULL){
				code_line_t *jmp_addr = c->op1->jptr;
				cl_destroy_operand(c->op1);	
				if (cw_check_if_in_code_list(jmp_addr) == true){
					c->op1 = create_updated_jump_operand(jmp_addr);
					c->op1->b->r.x = c->ins->b->r.x + op1_ofs;
					c->op1->b->r.y = c->ins->b->r.y;
				} else {
					c->op1 = NULL;
					c->state = MISSING_OP1;
				}
			}
		}
	}
error:
	return;
}

/* Function: cw_create_jump_operand
*------------------------------------------------------------------------------
* Creates the label operand with the corresponding number that will be shown
* in the code
*
* Arguments:
* 	None.
*	
* Return:
*	Pointer to the newly created operand
*
*/
operand_t *cw_create_jump_operand(code_line_t *addr)
{
   	operand_t *op = NULL;

	op = malloc(sizeof(operand_t));

	//code_line_t *addr = get_clicked_label_code_line();
	check_mem(addr);

	int addr_label_id = addr->op1->id;
	char *line_text = ax_number_to_string_two_digits(addr_label_id);
	char *op_text = malloc(sizeof(char)*(strlen(label_text) + 
						   strlen(char_space)+ strlen(line_text)));
	
	strcpy(op_text, label_text);
	strcat(op_text, char_space);
	strcat(op_text, line_text);
	texture_t *t = dw_create_text_texture(op_text, C_WHITE);

	SDL_Rect cb = dm_get_code_button_wh();
	SDL_Rect r = {.x = 0, .y = 0, .w = 2*cb.w, .h = cb.h};
	op->b = bt_create_button(r, false, false, false, C_BLACK, 
							 C_WHITE, t);
	op->id = cw_get_code_line_pos_by_ptr(addr);
	op->jptr = addr;

	free(line_text);
	free(op_text);

error:
	return op;
} 

/* Function: get_clicked_label_code_line
*------------------------------------------------------------------------------
* Traverses the whole code list and returns the pointer of the clicked 
* LABEL code line.
*
* Arguments:
* 	Void.
*	
* Return:
*	Clicked Label code line
*
*/
static code_line_t *get_clicked_label_code_line()
{
	List *code = get_code_list();
	check_mem(code);
	code_line_t *clicked_label = NULL;

	code_line_t *c;
	LIST_FOREACH(code, first, next, cur){ 
		c = cur->value;
		if (c->ins->id == LABEL){
			if (bt_check_mouse_released_button(c->ins->b) == true ||
				bt_check_mouse_released_button(c->op1->b) == true){
				clicked_label = c;
				break;
			}
		}
	}
error:
	return clicked_label;
}



/* Function: cw_check_released_in_label
*------------------------------------------------------------------------------
* This function verifies if a mouse click was released on a label instruction
* present in the code
*
* Arguments:
* 	Void.
*	
* Return:
*	boolean with the result if a mouse click was performed
*
*/
bool cw_check_released_in_label()
{
	List *code = get_code_list();
	check_mem(code);
	bool selected = false;

	code_line_t *c;
	LIST_FOREACH(code, first, next, cur){ 
		c = cur->value;
		if (c->ins->id == LABEL){
			if (bt_check_mouse_released_button(c->ins->b) == true ||
				bt_check_mouse_released_button(c->op1->b) == true){
				selected = true;
				break;
			}
		}
	}
error:
	return selected;
}

/* Function: update_label_instructions
 * -----------------------------------------------------------------------------
 * Traverses the whole code list and updates all the label instructions
 * 
 * Arguments:
 *  void.
 *
 * Return:
 * 	void.
 */
static void update_label_instructions()
{
	List *code = get_code_list();
	check_mem(code);
	
	int op1_ofs = dm_get_ofs_code_op1();
	code_line_t *c;
	LIST_FOREACH(code, first, next, cur){ 
		c = cur->value;
		if (c->ins->id == LABEL){
			if (c->op1 != NULL){
				cl_destroy_operand(c->op1);	
				c->op1 = create_label_operand(c);
				c->op1->b->r.x = c->ins->b->r.x + op1_ofs;
				c->op1->b->r.y = c->ins->b->r.y;
			}

		}
	}
error:
	return;
}

/* Function: create_saved_label_operand
*------------------------------------------------------------------------------
* Creates the label operand with the corresponding number that will be shown
* in the code
*
* Arguments:
* 	None.
*	
* Return:
*	Pointer to the newly created operand
*
*/
static operand_t *create_saved_label_operand(int op1_id)
{
   	operand_t *b = NULL;

	b = malloc(sizeof(operand_t));

	char *line_text = ax_number_to_string_two_digits(op1_id);
	char *op_text = malloc(sizeof(char)*(strlen(line_text)+1));
	strcpy(op_text, line_text);
	strcat(op_text, char_colon);
	texture_t *t = dw_create_text_texture(op_text, C_WHITE);

	SDL_Rect r = dm_get_code_button_wh();
	b->b = bt_create_button(r, false, false, false, C_BLACK, 
							C_WHITE, t);

	b->id = op1_id;

	free(line_text);
	free(op_text);

	return b;
}

/* Function: create_label_operand
*------------------------------------------------------------------------------
* Creates the label operand with the corresponding number that will be shown
* in the code
*
* Arguments:
* 	None.
*	
* Return:
*	Pointer to the newly created operand
*
*/
static operand_t *create_label_operand(code_line_t *line)
{
   	operand_t *b = NULL;

	b = malloc(sizeof(operand_t));

	int label = get_label_operand_value(line);

	char *line_text = ax_number_to_string_two_digits(label);
	char *op_text = malloc(sizeof(char)*(strlen(line_text)+1));
	strcpy(op_text, line_text);
	strcat(op_text, ":");
	texture_t *t = dw_create_text_texture(op_text, C_WHITE);

	SDL_Rect r = dm_get_code_button_wh();
	b->b = bt_create_button(r, false, false, false, C_BLACK, 
							C_WHITE, t);

	b->id = label;

	free(line_text);
	free(op_text);

	return b;
}

/* Function: cw_get_code_line_pos_by_ptr 
 * -----------------------------------------------------------------------------
 * This function traverses the list and determines the position in the list
 * of a given instruction
 *
 * Arguments:
 * 	line: Instruction that will be checked its position on the list
 *
 * Return:
 *	int with the position of the line in the code list
 */
int cw_get_code_line_pos_by_ptr(code_line_t *line)
{
	List *code = get_code_list();
	assert(NULL != code && "The code pointer is NULL");
	assert(NULL != line && "The line pointer is NULL");

	bool in_code_list = cw_check_if_in_code_list(line);
	int line_position = 0;

	if (in_code_list == false){
		line_position = NO_VALUE;
	} else {
		LIST_FOREACH(code, first, next, cur){
			if (cur->value == line){
				break;
			}
			line_position++;
		} 
	}
	return line_position;
}

/* Function: label_counter_up_to_index
 * -----------------------------------------------------------------------------
 * This function traverses the code up to an index and returns the value
 * of label instructions up to that point
 * 
 * Arguments:
 *  index: The index that the code list will be traversed up to
 *
 * Return:
 *	int with the number of label instructions
 */
static int label_counter_up_to_index(int index)
{
	int list_size = cw_get_code_list_size();
	assert(index < list_size && "index values is out of reach");

	List *code = get_code_list();
	check_mem(code);

	int label_counter = 0;
	int i = 0;
	code_line_t *c;
	
	LIST_FOREACH(code, first, next, cur){ 
		c = cur->value;
			if (c->ins->id == LABEL){
				label_counter++;	
			}
		if (i == index){
			break;
		}
		i++;
	}
error:
	return label_counter;
}

/* Function: get_label_operand_value
 * -----------------------------------------------------------------------------
 * Traverses the code list to return the correct label position value. The
 * instruction must be already added to the code list.
 * The 2 is added up to the value of label is because instruction 
 * position stars from 0, and the number of counted instructions from 1. The
 * extra one is because the i value of the instruction is counted.
 * 
 * Arguments:
 *  line: The label instruction that the operand will calculated.
 *
 * Return:
 *	int with the label value that will be assigned to the instruction
 */
static int get_label_operand_value(code_line_t *line)
{
	List *code = get_code_list();
	check_mem(code);
	check_mem(line);

	int pos = cw_get_code_line_pos_by_ptr(line);
	int label = pos;	

	bool in_code_list = cw_check_if_in_code_list(line);
	int list_size = cw_get_code_list_size();

	if (in_code_list == false){
		label = NO_VALUE;
	} else if (pos == 0){
		label = 1;
	 } else {
		int i = 0;
		code_line_t *c;
		LIST_FOREACH(code, first, next, cur){ 
			c = cur->value;
			if (i == pos){
				label = i - label_counter_up_to_index(i) + 2;
				break;
			}
			i++;
		}
	}
error:
	return label;
}

/* Function: cw_add_saved_line
 *------------------------------------------------------------------------------
 * Arguments:
 *	line_string: contains a string of the instruction that will be added
 *
 * Return:
 *	void.
 */
void cw_add_saved_line(char *line)
{
	assert(line != NULL && "The line value cannot be NULL");
	char *saveptr1;
	char *ins_text;
	char *op1_text;
	char *op2_text;
	char *delim = char_space;
	int ins_id;
	int op1_id;
	int op2_id;

	ins_text =  strtok_r(line, delim, &saveptr1);
	ins_id = cl_text_to_instruction_id(ins_text);
	
	int operand_quantity = cl_get_instruction_operand_quantity(ins_id);

	if (operand_quantity == ONE_OPERAND || operand_quantity == TWO_OPERANDS){
		if (ins_id == LABEL || ins_id == JMP){
			op1_text =  strtok_r(NULL, delim, &saveptr1);
			op1_id = atoi(op1_text);
		} else {
			op1_text =  strtok_r(NULL, delim, &saveptr1);
			op1_id = cl_text_to_operand_id(op1_text);
		}
	}
	
	if (operand_quantity == TWO_OPERANDS){
		op2_text =  strtok_r(NULL, delim, &saveptr1);
		op2_id = cl_text_to_operand_id(op2_text);
	}
	
	int list_size = cw_get_code_list_size();
	ins_text = cl_get_instruction_text(ins_id);
	texture_t *instruction_tex = dw_create_text_texture(
						  		  ins_text, C_WHITE);
	
	int x = cw_get_code_line_x(ins_id);
	int y = cw_get_code_line_y(0);

	SDL_Rect cb = dm_get_code_button_wh();
	for (int i = 0; i <= list_size; i++){
		y += cb.h;
	}
	
	SDL_Rect r = dm_get_code_button_wh();
	r.x = x;
	r.y = y;
	button_t *b = bt_create_button(r, true, false, false, C_BLACK, 
								   C_WHITE, instruction_tex);
	check_mem(b);
	instruction_t *new_ins = cl_create_instruction(ins_id, b);
	code_line_t *new_line = cl_create_code_line(new_ins);
	
	if (operand_quantity == ONE_OPERAND || operand_quantity == TWO_OPERANDS){
		operand_t *op1;
		if (ins_id == JMP){
			op1 = create_saved_jump_operand(op1_id);
		} else if (ins_id == LABEL){
			op1 = create_saved_label_operand(op1_id);
		} else if (op1_id > REGISTERS_MIN && op1_id < REGISTERS_MAX){
			op1 = rg_create_register_operand_by_id(op1_id);
		} else if (op1_id > BUFFERS_MIN && op1_id < BUFFERS_MAX){
			op1 = bf_create_buffer_operand_by_id(op1_id);
		}
		cl_assign_operand_to_line(op1, new_line);
	}
	if (operand_quantity == TWO_OPERANDS){
		operand_t *op2;
		if (op2_id > REGISTERS_MIN && op2_id < REGISTERS_MAX){
			op2 = rg_create_register_operand_by_id(op2_id);
		} else if (op2_id > BUFFERS_MIN && op2_id < BUFFERS_MAX){
			op2 = bf_create_buffer_operand_by_id(op2_id);
		}
		cl_assign_operand_to_line(op2, new_line);
	}

	new_line->state = COMPLETE;

	List *code = get_code_list();
	List_push(code, new_line);
	
	error:
	return;
}


/* Function: cw_get_code_line_at_pos
 *------------------------------------------------------------------------------
 * Arguments:
 *	int: the position of the code_line that will be recovered.
 *
 * Return:
 *	The code line at a given position 
 */
code_line_t *cw_get_code_line_at_pos(int pos)
{
	int code_size = cw_get_code_list_size();
	assert(pos >= 0 && pos < code_size && "Incorrect position value");

	List *code = get_code_list();
	code_line_t *c;
	
	int i = 0;
	LIST_FOREACH(code, first, next, cur){ 
		c = cur->value;
		if (i == pos){
			break;
		}
		i++;
	}
	return c;
}

/* Function: get_instruction_operand
 *------------------------------------------------------------------------------
 * Arguments:
 * instruction_pos: position of the instruction in the list.
 * operand_pos: the operand position.
 *
 * Return:
 *	id of the operand at given position for an instruction.
 */
int cw_get_instruction_operand(int position, int operand_pos)
{
	assert(position >= 0 && "Invalid instruction position");
	assert(position < cw_get_code_list_size() && "The instruction position is"
		   "invalid");
	assert(operand_pos >= ZERO_OPERANDS && 
		   operand_pos <= TWO_OPERANDS && "The operand position is invalid");

	int operand;
	List *code = get_code_list();
	code_line_t *c;
	
	int i = 0;
	LIST_FOREACH(code, first, next, cur){ 
		c = cur->value;
		
		if (i == position){
			break;
		}
		i++;
	}
	
	int operand_quantity = cl_get_instruction_operand_quantity(c->ins->id);
	
	if (operand_quantity == ZERO_OPERANDS){
		operand = NO_OPERAND;
	} else if (operand_quantity == ONE_OPERAND && operand_pos == FIRST_OP ||
			   operand_quantity == TWO_OPERANDS && operand_pos == FIRST_OP){
		operand = c->op1->id;
	} else if (operand_quantity == TWO_OPERANDS && operand_pos == SECOND_OP){
		operand = c->op2->id;
	}
	return operand;
}

/* Function: cw_get_instruction_at_code_pos
 *------------------------------------------------------------------------------
 * Arguments:
 *	int: the position of the instruction that wants to be recovered.
 *
 * Return:
 *	id of the instruction at a given position.
 */
int cw_get_instruction_at_code_pos(int position)
{
	assert(position >= 0 && "Invalid instruction position");
	assert(position < cw_get_code_list_size() && "The instruction position is"
		   "invalid");

	int i = 0;

	List *code = get_code_list();
	code_line_t *c;
	
	LIST_FOREACH(code, first, next, cur){ 
		c = cur->value;
		
		if (i == position){
			break;
		}
		i++;
	}
	return c->ins->id;
}

/* Function: cw_get_code_list_size
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 */
int cw_get_code_list_size()
{
	assert(code_list != NULL && "The code list is NULL");
	return List_count(code_list);
}

/* Function: cw_create_code_list
 *------------------------------------------------------------------------------
 * This function creates and initializes the code list.
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 *
 */
void cw_create_code_list()
{
	assert(code_list == NULL && "The register list is not NULL");
	code_list = List_create();
	return;
}

/* Function: get_code_list
 *------------------------------------------------------------------------------
 * This function returns the pointer to the code list.
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 *
 */
List *get_code_list()
{
	return code_list;
}

/* Function: cw_get_clicked_code
 *------------------------------------------------------------------------------
 * This function returns a pointer to the selected code line
 *
 * Arguments:
 * 	none.
 *	
 * Return:
 *	the pointer to the specific button that has been clicked.
 *	NULL if not button was clicked
 *
 */
code_line_t *cw_get_clicked_code()
{
	List *code = get_code_list();
	assert(NULL != code && "The code pointer is NULL");
	int x = List_count(code);
	
	code_line_t *clicked = NULL;
	LIST_FOREACH(code, first, next, cur){ 
		
		code_line_t *c = cur->value;

		if (true == bt_check_mouse_click_button(c->ins->b)){
			clicked = c;
		} 
	}
	
	return clicked;
}

/* Function: cw_check_clicked_code
 *------------------------------------------------------------------------------
 * This functions verifies if the user clicked one of the code developed by
 * the player
 *
 * Arguments:
 * 	none.
 *	
 * Return:
 *	true if the user clicked over an instructions, false if otherwise
 *
 */
bool cw_check_clicked_code()
{
	List *code = get_code_list();
	bool clicked = false;

	LIST_FOREACH(code, first, next, cur){ 
		
		code_line_t *c = cur->value;
		
		if (true == bt_check_mouse_click_button(c->ins->b)){
			clicked = true;
			break;
		} 
	}
	return clicked;
}

/* Function: cw_set_stage_name
 * -----------------------------------------------------------------------------
 * This function is called to set the string that will be displayed as the 
 * the stage name
 *
 * Arguments:
 * text: the text that will be set on the challenge text
 *
 * Return:
 *	void
 */
void cw_set_stage_name(char *text)
{
	assert(NULL != text && "The text pointer is NULL");

	g_stage_name = dw_create_text_texture(text, C_WHITE);
	check_mem(g_stage_name);
error:
	return;
}

/* Function: cw_clear_code_window_assets
 * -----------------------------------------------------------------------------
 * Clears the whole assets related to the code window when leaving a level
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	Void.
 */
void cw_destroy_code_window_assets()
{
	dw_free_texture(g_stage_name);
	dw_free_texture_array(g_challenge_text);
	List *code = get_code_list();
	LIST_FOREACH(code, first, next, cur){
		code_line_t *line = cur->value;	
		cl_destroy_code_line(line);
	}
	List_destroy(code_list);
	code_list = NULL;
}


/* Function: cw_get_code_box_member
 * -----------------------------------------------------------------------------
 * This function access and returns the members of the code box object.
 *
 * Arguments:
 * member: The specific member that is being requested.
 *
 * Return:
 *	The accessed member.
 */
static SDL_Rect get_code_box()
{
	return code_box;
}

/* Function: set_code_box_member
 * -----------------------------------------------------------------------------
 * This function modifies an specific member of the code box.
 *
 * Arguments:
 * value: the value that is going to be set.
 * member: The specific member that is gonna be modified.
 *
 * Return:
 *	void.
 */
static void set_code_box_member(int value, int member)
{
	assert(member > MEMBER_MIN && member < MEMBER_MAX && "Invalid member");

	switch (member){
		
		case MEMBER_X:
			code_box.x = value;
			break;
		case MEMBER_Y:
			code_box.y = value;
			break;
		case MEMBER_W:
			code_box.w = value;
			break;
		case MEMBER_H:
			code_box.h = value;
			break;
		default:
			perror("Error: defaulting on invalid case");
			break;
	}
	return;
}

/* Function: set_text_box_member
 * -----------------------------------------------------------------------------
 * This function modifies an specific member of the text box.
 *
 * Arguments:
 * value: the value that is going to be set.
 * member: The specific member that is gonna be modified.
 *
 * Return:
 *	void.
 */
static void set_text_box_member(int value, int member)
{
	assert(member >= MEMBER_X && member <= MEMBER_H &&  "Member is incorrect");

	switch (member){
		
		case MEMBER_X:
			text_box.x = value;
			break;
		case MEMBER_Y:
			text_box.y = value;
			break;
		case MEMBER_W:
			text_box.w = value;
			break;
		case MEMBER_H:
			text_box.h = value;
			break;
		default:
			perror("Error: defaulting on invalid case");
			break;
	}
	return;
}



/* Function: cw_get_text_box_rect
 * -----------------------------------------------------------------------------
 * Returns the rectangle of the text boox
 *
 * Arguments:
 * 	Void.
 *
 * Return:
 *	The rectangle of the text box
 */
SDL_Rect cw_get_text_box_rect()
{
	return text_box;
}

/* Function: check_if_inside_code_window
 * -----------------------------------------------------------------------------
 * This function verifies if the mouse coordinates are inside of one of the 
 * spaces where a instruction can be assigned.
 *
 * Arguments:
 * mouse: the mouse object that will be evaluated.	
 *
 * Return:
 *	Void.
 */
static bool check_if_inside_code_window(){
	
	int mouse_x = ms_get_mouse_x();
	int mouse_y = ms_get_mouse_y();

	if (mouse_x > code_box.x && mouse_x < (code_box.x + code_box.w) &&
		mouse_y > code_box.y && mouse_y < (code_box.y + code_box.h)) {
		return true;
	} else {
		return false;
	}
}

/* Function: cw_set_challenge_text
 * -----------------------------------------------------------------------------
 * This function is called to set the string that will be displayed as the 
 * challenge text.
 *
 * Arguments:
 * text: the text that will be set on the challenge text
 *
 * Return:
 *	void
 */
void cw_set_challenge_text(char *text)
{
	int w = dm_get_w_code_box_text();
	int h = dm_get_h_msg();
	assert(NULL != text && "The text pointer is NULL");
	g_challenge_text = dw_new_text_texture_by_h(w, h, C_WHITE, text);
}

/* Function: cw_set_code_box
 * -----------------------------------------------------------------------------
 * This function is called to set the value on the code box object
 *
 * Arguments:
 * x: x coordinate of the box
 * y: y coordinate of the box
 * w: width of the box
 * h: height of the box
 *
 * Return:
 *	void
 */
void cw_set_code_box(SDL_Rect r)
{
	code_box.x = r.x;
	code_box.y = r.y;
	code_box.w = r.w;
	code_box.h = r.h;

	int w = dm_get_w_code_box_text();
	int h = dm_get_h_msg();

	int text_h = dm_get_h_big_text();
	int border_ofs = dm_get_ofs_code_box_border();
	int text_box_height = g_challenge_text->size * h;
	set_text_box(r.x + border_ofs, r.y + border_ofs + text_h + border_ofs, w, 
															   text_box_height);
}

/* Function: set_text_box
 * -----------------------------------------------------------------------------
 * This function is called to set the value on the text box object
 *
 * Arguments:
 * x: x coordinate of the box
 * y: y coordinate of the box
 * w: width of the box
 * h: height of the box
 *
 * Return:
 *	void
 */
static void set_text_box(int x, int y, int w, int h)
{
	text_box.x = x;
	text_box.y = y;
	text_box.w = w;
	text_box.h = h;
}


/* Function: cw_get_code_line_x
 * -----------------------------------------------------------------------------
 * This function calculates and returns the x position of the lines of code
 * that will be incorporated in the code box. IF it is a label instruction
 * it will provide the correct x value.
 *
 * Arguments:
 * 	none.
 *
 * Return:
 *	int with the coordinate of the x coordinates for the lines of code
 */
int cw_get_code_line_x(int instruction_id)
{
	int x;  
	int number_ofs = dm_get_ofs_code_number();
	SDL_Rect sb = dm_get_stage_code_box();
	int number_h = dm_get_h_line_number();
	int number_w = ax_get_texture_w_fit_h(number_h, g_numbers[0]);
	if (instruction_id == LABEL){
		x = sb.x;
	} else {
		x = sb.x + number_ofs + number_w*3/2;	
	}
	return x;
}

/* Function: code_box_height_adjust
 * -----------------------------------------------------------------------------
 * This function handles the length of the code box handler according to the 
 * number of code lines done by the player.
 *
 * Arguments:
 * 	none.
 *
 * Return:
 *	void.
 */
static void code_box_height_adjust()
{
	List *code = get_code_list();
	assert(NULL != code && "The code list cannot be NULL");

	int list_size = List_count(code);

	SDL_Rect cbut = dm_get_code_button_wh();
	SDL_Rect codbox = get_code_box();
	SDL_Rect ogcodbox = dm_get_stage_code_box();
	if (CODE_LINES_SIZE < list_size){
		
		int increase_size = (list_size - CODE_LINES_SIZE)*cbut.h;
		
		int new_bottom_border = codbox.y + ogcodbox.h + increase_size;

		int cur_bottom_border = codbox.h + codbox.y;

		int delta = get_movement_delta(cur_bottom_border, new_bottom_border, 
									   MOVEMENT_DELTA/2);
		
		if (new_bottom_border > cur_bottom_border){
			cur_bottom_border += delta;
			set_code_box_member(cur_bottom_border - codbox.y, MEMBER_H);
		} else if (new_bottom_border < cur_bottom_border){
			cur_bottom_border -= delta;
			set_code_box_member(cur_bottom_border - codbox.y, MEMBER_H);
		} 
		
	}
}

/* Function: adjust_code_box_position
 * -----------------------------------------------------------------------------
 * This function handles the length position of the code box according to mouse
 * scroll.
 *
 * Arguments:
 * 	none.
 *
 * Return:
 *	void.
 */
static void adjust_code_box_position()
{
	static int y_pending_scroll = 0;
	
	y_pending_scroll += 3*ms_get_mouse_scroll_y();
	SDL_Rect cb = dm_get_stage_code_box();
	SDL_Rect codbox = get_code_box();

	int cur_y = codbox.y;
	int max_upper_border_y = cb.y;
	int min_lower_border_y = cb.y + cb.h;

	if (cur_y >= max_upper_border_y && y_pending_scroll > 0){
		y_pending_scroll = 0;
	} 
	if ((cur_y + codbox.h) <= min_lower_border_y && y_pending_scroll < 0){
		y_pending_scroll = 0;
	}

	int delta = get_movement_delta(0, y_pending_scroll, MOVEMENT_DELTA);	

	if (0 != y_pending_scroll){
		SDL_Rect tb = cw_get_text_box_rect();
		int text_box_y = tb.y;
		
		if (y_pending_scroll < 0){
			cur_y -= delta;
			set_code_box_member(cur_y, MEMBER_Y);
			text_box_y -= delta;
			set_text_box_member(text_box_y, MEMBER_Y);
			y_pending_scroll += delta;
		} else if (y_pending_scroll > 0){
			cur_y += delta;
			set_code_box_member(cur_y, MEMBER_Y);
			text_box_y += delta;
			set_text_box_member(text_box_y, MEMBER_Y);
			y_pending_scroll -= delta;
		}
	}
}

/* Function: cw_draw_code_window
 * -----------------------------------------------------------------------------
 * This function is called to draw all the elements related to the code window
 *
 * Arguments:
 * 	none.
 *
 * Return:
 *	void.
 */
void cw_draw_code_window()
{
	List *code = get_code_list();

	display_player_code();
	display_line_number();

	int h = dm_get_h_msg();
	dw_draw_wrapped_texture_by_h(text_box, h, g_challenge_text);

	// Text rectangle
	dw_draw_rectangle(text_box, C_WHITE);
	
	// Adjust the height of the code box
	code_box_height_adjust();

	if (check_if_inside_code_window() == true){
		adjust_code_box_position();
	}
	// Draw the rectangle of the code and instructions
	dw_draw_rectangle(code_box, C_WHITE);

	// Text of the level

	int border_ofs = dm_get_ofs_code_box_border();
	int text_h = dm_get_h_big_text();
	SDL_Rect b = {.x = code_box.x + border_ofs, 
				  .y = code_box.y + border_ofs,
				  .h = text_h};
	dw_draw_texture_fits_height(b, g_stage_name);
}

/* Function: display_player_code
 * -----------------------------------------------------------------------------
 * This function displays the code that is contained in the list of code
 *
 * Arguments:
 * 	code_list: a list with the code that will be executed
 *
 * Return:
 *	void
 */
static void display_player_code()
{
	List *code = get_code_list();
	assert(code != NULL && "The code list can't be NULL");
	
	int list_size = List_count(code);
	
	if (list_size == 0){
		return;
	}
	SDL_Rect cb = dm_get_code_button_wh();
	int number_ofs = dm_get_ofs_code_number();
	LIST_FOREACH(code, first, next, cur){
		code_line_t *line = cur->value;	
		bt_draw_button(line->ins->b);
		
		if (line->op1 != NULL){
			bt_draw_button(line->op1->b);
		}
		if (line->op2 != NULL){
			bt_draw_button(line->op2->b);
		}
	
		int comma = cl_get_instruction_operand_quantity(line->ins->id);

		if (comma == TWO_OPERANDS && (MISSING_BOTH != line->state && 
							  					  MISSING_OP1 != line->state)){
		
			int comma_ofs = dm_get_ofs_code_comma();
			SDL_Rect r = {.x = line->ins->b->r.x + comma_ofs, 
						  .y = line->ins->b->r.y, .h = cb.h};

			dw_draw_texture_fits_height(r, g_comma_tex);
		}
	}
	return;
}


/* Function: cw_get_instruction_y_coord
 * -----------------------------------------------------------------------------
 * Returns the y coordinaten of the instruction by its position
 *
 * Arguments:
 * instruction_position: Instruction position in the code list
 *
 * Return:
 *	y_coordinate: The y coordinate of the instruction in a given position.
 */
int cw_get_instruction_y_coord(int instruction_position)
{
	int code_size = cw_get_code_list_size();
	assert(instruction_position >= 0 && instruction_position < code_size &&
		   "Incorrect instruction position value");

	int y;
	List *code = get_code_list();
	code_line_t *c;
	
	int i = 0;
	LIST_FOREACH(code, first, next, cur){ 
		c = cur->value;
		if (i == instruction_position){
			y = c->ins->b->r.y;
			break;
		}
		i++;
	}
	return y;
}

/* Function: display_line_number
 * -------------------------------------
 * This function displays the number of each of the line of code according
 * to the number of instructions in the code list
 *
 * Arguments:
 * code: a list with the code developed by the player
 *
 * Return:
 *	void
 */
void display_line_number()
{
	List *code = get_code_list();
	assert(code != NULL && "The code list can't be NULL");
	
	int list_size = List_count(code);
	if (list_size == 0){
		return;
	}

	SDL_Rect cb = dm_get_code_button_wh();
	int number_ofs = dm_get_ofs_code_number();
	SDL_Rect sb = dm_get_stage_code_box();
	int x = sb.x + number_ofs;
	int y = cw_get_code_line_y(0);
	int h = dm_get_h_line_number();
	char *number = NULL;
	
	int line_number = 1;
	for(int i = 0; i < list_size; i++){
	int instruction = cw_get_instruction_at_code_pos(i);
		if (instruction != LABEL){
			SDL_Rect r = {.x = x, .y = y, .h = h};
			dw_draw_texture_fits_height(r, g_numbers[line_number]);
			line_number++;
		}
		y += cb.h;
	}
}

/* Function: cw_get_code_line_y
 * -------------------------------------
 * Returns the y coodinate a instruction must be assigned
 *
 * Arguments:
 * 	position: position of the line
 *
 * Return:
 * 	The y coordinate the instruction must have 
 */
int cw_get_code_line_y(int pos)
{
	List *code = get_code_list();
	assert(code != NULL && "The code list can't be NULL");
	
	int list_size = List_count(code);
	int border_ofs = dm_get_ofs_code_box_border();
	SDL_Rect tb = cw_get_text_box_rect();
	int y = tb.y + tb.h + border_ofs;

	if (list_size == 0 || pos == 0){
		return y;
	}

	SDL_Rect cb = dm_get_code_button_wh();
	
	for(int i = 0; i < list_size; i++){
		y += cb.h;
		if (pos == i){
			break;
		}
	}
	return y;
}

/* Function: cw_get_code_line_coord_at_pos
 * -----------------------------------------------------------------------------
 * This function return a given element of the code line for a position
 * with the properties of the position that should be assigned, for example
 * the instruction, operand1, or operand 2 for a given position
 *
 * Arguments:
 *	element: 
 *
 * Return:
 *	Void.
 */
SDL_Rect cw_get_code_line_coord_at_pos(int code_line_element, int pos)
{
	SDL_Rect cb = dm_get_code_button_wh();
	int op1_ofs = dm_get_ofs_code_op1();
	int op2_ofs = dm_get_ofs_code_op2();
	
	int x = cw_get_code_line_x(MOV);
	int y = cw_get_code_line_y(0);
	
	SDL_Rect bwh = dm_get_code_button_wh();
	SDL_Rect cl = {.x = x, .y = y, .w = bwh.w, .h = bwh.h};

	int list_size = cw_get_code_list_size();

	switch(code_line_element){
		case CW_OP1:
			cl.x += op1_ofs;
			break;
		case CW_OP2:
			cl.x += op2_ofs;	
			break;
		default:
			break;
	}

	if (list_size == 0 || pos == 0){
		return cl;
	}
	

	for(int i = 0; i < list_size; i++){
		cl.y += cb.h;
		if (pos == i){
			break;
		}
	}
	return cl;
}

/* Function: cw_sort_code
 * -----------------------------------------------------------------------------
 * This function iterates throught the list of code done by the player to 
 * verify which instructions are out of position. If a instruction is out
 * of position, it will accomodate it's coordinates.
 *
 * Arguments:
 *	none.
 *
 * Return:
 *	Void.
 */
void cw_sort_code()
{
	List *code = get_code_list();
	int list_size = List_count(code);
	
	if (0 == list_size){
		return;
	}
	
	SDL_Rect cb = dm_get_code_button_wh();
	int op1_ofs = dm_get_ofs_code_op1();
	int op2_ofs = dm_get_ofs_code_op2();
	
	int h = cb.h; 
	int y = cw_get_code_line_y(0);

	LIST_FOREACH(code, first, next, cur){

		code_line_t *line = cur->value;
		int x = cw_get_code_line_x(line->ins->id);
		int delta = get_movement_delta(line->ins->b->r.x, x, MOVEMENT_DELTA);	
		
		if (line->ins->b->r.x < x){
			line->ins->b->r.x += delta;
		} else if (line->ins->b->r.x > x){
			line->ins->b->r.x -= delta;
		}
		delta = get_movement_delta(line->ins->b->r.y, y, MOVEMENT_DELTA);	
		
		if (line->ins->b->r.y < y){
			line->ins->b->r.y += delta;
		} else if (line->ins->b->r.y > y){
			line->ins->b->r.y -= delta;
		}
		if (line->op1 != NULL){
				line->op1->b->r.x = line->ins->b->r.x + op1_ofs;
				line->op1->b->r.y = line->ins->b->r.y;
		}
		if (line->op2 != NULL){
				line->op2->b->r.x = line->ins->b->r.x + op2_ofs;
				line->op2->b->r.y = line->ins->b->r.y;
		}
		y += h;
	}
	return ;
}

/* Function: get_list_node_by_value
 * -----------------------------------------------------------------------------
 * This function returns the corresponding node of a give value for that node 
 * it assumes that each node has a unique value pointer
 *
 * Arguments:
 * 	line: Line that is going to be verified if already in list.
 *
 * Return:
 *	pointer to the node, NULL if node is not found
 */

ListNode *get_list_node_by_value(code_line_t *line)
{
	List *code = get_code_list();
	assert(NULL != code && "The list pointer is NULL");
	assert(NULL != line && "The line pointer is NULL");

	LIST_FOREACH(code, first, next, cur){
		
		if (cur->value == line){
			return cur;
		}
	}
	return NULL;
}

/* Function: cw_check_if_in_code_list
 * -----------------------------------------------------------------------------
 * This function verifies if a selected element of the screen is already in
 * a list, to avoide adding the same element multiple times.
 *
 *
 * Arguments:
 * 	lin: Node that is going to be verified if already in list.
 *
 * Return:
 *	true if already present, false if otherwise.
 */

bool cw_check_if_in_code_list(code_line_t *line)
{
	List *code = get_code_list();
	assert(NULL != code && "The code pointer is NULL");
	assert(NULL != line && "The line pointer is NULL");

	LIST_FOREACH(code, first, next, cur){
		
		if (cur->value == line){
			return true;
		}
	}
	return false;
}


/* Function: get_code_line_position
 * ----------------------------------------------------------------------------
 * This function receives a y coordinate and determines the position in the 
 * linked list of the instruction corresponding to that coordinate.
 *
 * Arguments:
 * y: the y coordinate used to determine the code line position.
 *
 * Return:
 *	The position of the code line according, -1 if the coordinate do not 
 * 	corresponds to any code line.
 */
int get_code_line_position(int y)
{
	List *code = get_code_list();
	int first_y = cw_get_code_line_y(0);

	SDL_Rect cb = dm_get_code_button_wh();
	int h = cb.h;
	int list_size = List_count(code);

	for(int i = 0; i < list_size; i++){
		if(y >= first_y && y < first_y + h){
			return i;
		}
		first_y += h;
	}	
	return NOT_FOUND;
}


/* Function: add_code_line
 * ----------------------------------------------------------------------------
 * This function adds a code line to the respective code list, in the correct
 * position according to mouse coordinates.
 *
 * Arguments:
 * instruction: the instruction to be added to the list
 *
 * Return:
 *	Void.
 */
static void add_code_line(code_line_t *line)
{
	List *code = get_code_list();
	assert(NULL != code &&  "Code list is NULL");
	assert(NULL != line &&  "The code line object is NULL");

	int list_count = List_count(code);
		
	if (0 == list_count){
		List_push(code, line);
		return;
	} 
		
	code_line_t *first = code->first->value;
	int mouse_y = ms_get_mouse_y();
	int y = first->ins->b->r.y;
	
	if (mouse_y < y){
		List_unshift(code, line);
		return;
	} 

	code_line_t *last = code->last->value;
	y = last->ins->b->r.y;
	int h = last->ins->b->r.h;
	if (mouse_y >= y + h){
		List_push(code, line);
		return;
	}

	int position = get_code_line_position(mouse_y);
	if (NOT_FOUND != position){
		List_insert_at_position(code, line, position);
		return;
	} else {
		// If list comes to this place without adding code line, it's added 
		// to the last element
		List_push(code, line);
	}

}

/* Function: check_selected_line_in_position
 * -----------------------------------------------------------------------------
 * This function verifies if the code line is in it's correct line coordinate
 * according to it's position in the list. If the line is not in it's y
 * coordinate range, it is removed. The position is measured using mouse
 * coordinates, so this function works if the line is being selected with the
 * mouse.
 *
 * Arguments:
 * line: the instruction that the mouse object is carrying
 *
 * Return:
 *	Void.
 */
static bool check_selected_line_in_position(code_line_t *line)
{
	List *code = get_code_list();
	assert(NULL != code && "The list of code is NULL");
	assert(NULL != line && "The evaluated line is NULL");
	
	int list_size = List_count(code);
	
	if (0 == list_size){
		return false;
	}
	
	int y = cw_get_code_line_y(0);

	int mouse_y = ms_get_mouse_y();
	bool ret = false;
	SDL_Rect cb = dm_get_code_button_wh();
	int h = cb.h;
	if (mouse_y < y || mouse_y >= y + list_size*h){
		ret = false;
	} else {
		LIST_FOREACH(code, first, next, cur){
			if (cur->value == line){
				if (mouse_y >= y && mouse_y < y + h){
					ret =  true;	
				}
			}
			y += h;
		}
	}
	return ret;
}

/* Function: cw_player_holding_instruction
 * -----------------------------------------------------------------------------
 * This function contains the window were the programmed code is going to be
 * it will contain the list of instructions to be executed and it will high
 * where the instruction can be located.
 *
 * Arguments:
 * 	instruction: the instruction that the mouse object is carrying
 *
 * Return:
 *	Void.
 */
void cw_player_holding_instruction(code_line_t *line)
{
	List *code = get_code_list();
	assert(code != NULL &&  "Code list is NULL");
	assert(line != NULL &&  "The code line object is NULL");
	
	if (check_if_inside_code_window() == true || line->ins->id == LABEL){
		if (cw_check_if_in_code_list(line) == true){
			if (check_selected_line_in_position(line) == false){
				ListNode *node = get_list_node_by_value(line);
				List_remove(code, node);
			}
		} 
		if (cw_check_if_in_code_list(line) == false){
			add_code_line(line);
			update_label_instructions();
			update_jump_instructions();
		}
		if (line->ins->id == LABEL){
			if (line->op1 == NULL){
				line->op1 = create_label_operand(line);
				line->state = COMPLETE;
			} else {
				cl_destroy_operand(line->op1);	
				line->op1 = create_label_operand(line);
				line->state = COMPLETE;
			}
		}
	} else {
		if (cw_check_if_in_code_list(line) == true){
			if (line->ins->id == JMP && line->op1 != NULL){
				code_line_t *addr = line->op1->jptr;
				ListNode *node = get_list_node_by_value(addr);
				cl_destroy_code_line(addr);
				List_remove(code, node);
			}
			ListNode *node = get_list_node_by_value(line);
			List_remove(code, node);
			update_label_instructions();
			update_jump_instructions();
		}
	}

	line->ins->b->r.x = ms_get_mouse_x() - line->ins->b->r.w/2;
	line->ins->b->r.y = ms_get_mouse_y() - line->ins->b->r.h/2;
	int op1_ofs = dm_get_ofs_code_op1();
	int op2_ofs = dm_get_ofs_code_op2();

	if (line->op1 != NULL){
		line->op1->b->r.x = line->ins->b->r.x + op1_ofs;
		line->op1->b->r.y = line->ins->b->r.y;
		bt_draw_button(line->op1->b);
	}
	if (line->op2 != NULL){
		line->op2->b->r.x = line->ins->b->r.x + op2_ofs;
		line->op2->b->r.y = line->ins->b->r.y;
		bt_draw_button(line->op2->b);
	}
	bt_draw_button(line->ins->b);
}

/* Function: cw_check_code_pending_op1
 * -----------------------------------------------------------------------------
 * This function verifies if any of the programmed lines is pending op2
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 * 	true if a line is pending operand, false if otherwise
 *
 */
bool cw_check_code_pending_op2()
{
	List *code = get_code_list();

	bool pending = false;
	LIST_FOREACH(code, first, next, cur){
		code_line_t *c = cur->value;
		int s = c->state;
		if (s == MISSING_BOTH || s == MISSING_OP2 || s == CHANGING_OP2){
			pending = true;
			break;
		}
	}
	return pending;
}

/* Function: cw_check_code_pending_op1
 * -----------------------------------------------------------------------------
 * This function verifies if any of the programmed lines is pending op1
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 * 	true if a line is pending operand, false if otherwise
 *
 */
bool cw_check_code_pending_op1()
{
	List *code = get_code_list();

	bool pending = false;
	LIST_FOREACH(code, first, next, cur){
		code_line_t *c = cur->value;
		int s = c->state;
		if (s == MISSING_BOTH || s == MISSING_OP1 || s == CHANGING_OP1){
			pending = true;
			break;
		}
	}
	return pending;
}

/* Function: cw_check_code_pending_operand
 * -----------------------------------------------------------------------------
 * This function verifies if any of the programmed lines is pending an operand
 *
 * Arguments:
 * 	code: the list of code programmed by the player
 *	
 * Return:
 * 	true if a line is pending operand, false if otherwise
 *
 */
bool cw_check_code_pending_operand()
{
	List *code = get_code_list();

	bool pending = false;
	LIST_FOREACH(code, first, next, cur){
		code_line_t *c = cur->value;
		int s = c->state;
		if (s == MISSING_BOTH || s == MISSING_OP1 || s == MISSING_OP2 ||
			s == CHANGING_OP1 || s == CHANGING_OP2){
			pending = true;
			break;
		}
	}
	return pending;
}

/* Function: get_line_pending_operand
 * -----------------------------------------------------------------------------
 * This returns a pointer to the first code line that has a missing operand
 *
 * Arguments:
 * 	none.
 *	
 * Return:
 * 	pointer to the code line that has the missing operand, or NULL if line is
 *	not found (should not be the case).
 *
 */
code_line_t *cw_get_code_line_pending_operand()
{
	List *code = get_code_list();
	assert(NULL != code && "The code list cannot be NULL");
	code_line_t *ret = NULL;

	LIST_FOREACH(code, first, next, cur){
		code_line_t *c = cur->value;
		int s = c->state;
		if (MISSING_BOTH == s || MISSING_OP1 == s || MISSING_OP2 == s ||
			CHANGING_OP1 == s || CHANGING_OP2 == s){
			ret = c;
			break;
		}
	}
	return ret;
}


/* Function: cw_highlight_code_pending_operand
 * -----------------------------------------------------------------------------
 * This function is the handler that is called when a line is missing an 
 * operand.
 *
 * Arguments:
 * 	code: the list of code programmed by the player
 *	
 * Return:
 * 	Void.
 *
 */
void cw_highlight_code_pending_operand()
{
	code_line_t *line = cw_get_code_line_pending_operand();
	check_mem(line);
	
	int op1_ofs = dm_get_ofs_code_op1();
	int op2_ofs = dm_get_ofs_code_op2();

	if (MISSING_BOTH == line->state || MISSING_OP1 == line->state ||
		CHANGING_OP1 == line->state){
		SDL_Rect r = {.x = line->ins->b->r.x + op1_ofs, 
					  .y = line->ins->b->r.y, .w = line->ins->b->r.w, 
					  .h = line->ins->b->r.h};
		dw_draw_rectangle(r, C_WHITE);

	} else if (MISSING_OP2 == line->state || CHANGING_OP2 == line->state){
		SDL_Rect r = {.x = line->ins->b->r.x + op2_ofs, 
					  .y = line->ins->b->r.y, .w = line->ins->b->r.w, 
					  .h = line->ins->b->r.h};
		dw_draw_rectangle(r, C_WHITE);

	}
error:
	return;
}

/* Function: cw_check_code_sorted
 * -----------------------------------------------------------------------------
 * This function verifies if all the lines of code are in their position
 *
 * Arguments:
 * 	code: the list of code programmed by the player
 *	
 * Return:
 * 	true if all code is in position, false if otherwise.
 *
 */
bool cw_check_code_sorted()
{
	List *code = get_code_list();
	int list_size = List_count(code);
	if (0 == list_size){
		return true;
	}
	
	SDL_Rect cb = dm_get_code_button_wh();
	int h = cb.h;
	int y = cw_get_code_line_y(0);

	int retval = true;
	LIST_FOREACH(code, first, next, cur){

		code_line_t *value = cur->value;
		button_t *b = value->ins->b;
		int x = cw_get_code_line_x(value->ins->id);

		if (b->r.x < x || b->r.x > x || b->r.y < y || b->r.y > y){
			retval = false;
			break;
		} 
		y += h;
	}	
	return retval;
}

/* Function: cw_check_clicked_code_operand
 * -----------------------------------------------------------------------------
 * This function checks if the player clicked on an operando of the instructions
 * available in the code window.
 *
 * Arguments:
 * 	code: the list of code programmed by the player
 *	
 * Return:
 * 	true if player clicked on operand. 
 *	false if otherwise.
 *
 */
bool cw_check_clicked_code_operand()
{
	List *code = get_code_list();
	assert(NULL != code && "The code pointer cannot be NULL");
	bool clicked = false;
	
	LIST_FOREACH(code, first, next, cur){ 
		
		code_line_t *c = cur->value;
		if (NULL != c->op1){
			if (true == bt_check_mouse_released_button(c->op1->b)){
				clicked = true;
				break;
			}
		} 
		if (NULL != c->op2){
			if (true == bt_check_mouse_released_button(c->op2->b)){
				clicked = true;
				break;
			}
		}
	}
	return clicked;
}

/* Function: cw_change_clicked_line_status
 * -----------------------------------------------------------------------------
 * This function changes the status of a line whose operand has been clicked
 * in order to make the game selecte the clicked operand.
 *
 * Arguments:
 * 	none.
 *	
 * Return:
 *	void.
 *
 */
void cw_change_clicked_code_line_state()
{
	List *code = get_code_list();
	assert(NULL != code && "The code pointer cannot be NULL");
	LIST_FOREACH(code, first, next, cur){ 
		
		code_line_t *c = cur->value;
		if (c->ins->id != LABEL && c->ins->id != JMP){
			if (c->op1 != NULL){
				if (bt_check_mouse_released_button(c->op1->b) == true){
					c->state = CHANGING_OP1;
					break;
				}
			} 
			if (c->op2 != NULL){
				if (bt_check_mouse_released_button(c->op2->b) == true){
					c->state = CHANGING_OP2;
					break;
				}
			}
		}
	}
}

/* Function: cw_reset_code_exectution
*------------------------------------------------------------------------------
* Arguments:
* 	None.
*	
* Return:
*	Void.
*/
void cw_reset_code_execution()
{
	List *code = get_code_list();
   	int x = List_count(code);

   	LIST_FOREACH(code, first, next, cur){ 
	   	code_line_t *c = cur->value;
		c->state = COMPLETE;   
   }
}

/* function: cw_create_label_code_line
 * -----------------------------------------------------------------------------
 * This function is called when a jump instruction in in the code, it generates
 * the label codeline that is required as the jump destitu
 *
 * Arguments:
 * 	Void.
 *
 * Return:
 *	Void.
 */
code_line_t *cw_create_label_code_line()
{
	char *text = cl_get_instruction_text(LABEL);
	texture_t *t = dw_create_text_texture(text, C_WHITE);
	SDL_Rect r = dm_get_code_button_wh();
	r.x = ms_get_mouse_x() - r.w/2;
	r.y = ms_get_mouse_y() - r.h/2;

	button_t *b = bt_create_button(r, true, false, false, C_BLACK, C_WHITE, t);
	check_mem(b);
	
	instruction_t *i = cl_create_instruction(LABEL, b);
	code_line_t *new = cl_create_code_line(i);

error:
	return new;

}
