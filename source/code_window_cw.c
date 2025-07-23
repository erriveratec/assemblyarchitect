#include <SDL.h>
#include <assert.h>
#include <string.h>
#include "code_window_cw.h"
#include "list.h"
#include "mouse_ms.h"
#include "dbg.h"
#include "dimensions.h"
#include "buffers_bf.h"
#include "registers_rg.h"

#define NOT_FOUND -1

static List *code_list = NULL;

static SDL_Rect code_box;
static SDL_Rect text_box;
static char *challenge_text;
static char *stage_name;

enum box_member {INVALID_MEMBER, MEMBER_X, MEMBER_Y, MEMBER_W, MEMBER_H};

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
static int get_code_box_member(int member);
static int get_text_box_member(int member);
static void code_box_height_adjust();
static void adjust_code_box_position();
static int get_first_code_line_y();
static int get_code_line_x();
static int get_label_operand_value(code_line_t *line);
List *get_code_list();
static int get_instruction_position(code_line_t *line);

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
operand_t *create_label_operand(code_line_t *line)
{
   	operand_t *b = NULL;

	b = malloc(sizeof(operand_t));

	int label = get_label_operand_value(line);
	
	texture_t *t = load_texture_from_rendered_text(rax_text, COLOR_WHITE);

	int x = 0;
	int y = 0;
	b->b = create_button(x, y, CODE_BUTTON_W, CODE_BUTTON_H, false, false, t);

	b->id = label;

	return b;
}

/* Function: cw_get_instruction_position
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

static int get_instruction_position(code_line_t *line)
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


/* Function: get_label_operand_value
 * -----------------------------------------------------------------------------
 * Traverses the code list to return the correct label position value. The
 * instruction must be already added to the code list.
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

	int label;
	bool in_code_list = cw_check_if_in_code_list(line);
	
	if (in_code_list == false){
		label = NO_VALUE;
	} if (cw_get_code_list_size() == 1){
		label = 1;
	} else {
		int i = 0;
		label = get_instruction_position(line);
		code_line_t *c;
		LIST_FOREACH(code, first, next, cur){ 
			if (i > label){
				c = cur->value;
				if (c->ins->id != LABEL){
					label = i;
					break;
				}
			} else {
				i++;
			}
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
	char *delim = " ";
	int ins_id;
	int op1_id;
	int op2_id;


	ins_text =  strtok_r(line, delim, &saveptr1);
	ins_id = cl_text_to_instruction_id(ins_text);
	
	int operand_quantity = cl_get_instruction_operand_quantity(ins_id);

	if (operand_quantity == ONE_OPERAND || operand_quantity == TWO_OPERANDS){
		op1_text =  strtok_r(NULL, delim, &saveptr1);
		op1_id = cl_text_to_operand_id(op1_text);
	}
	if (operand_quantity == TWO_OPERANDS){
		op2_text =  strtok_r(NULL, delim, &saveptr1);
		op2_id = cl_text_to_operand_id(op2_text);
	}
	
	int list_size = cw_get_code_list_size();
	ins_text = cl_get_instruction_text(ins_id);
	texture_t *instruction_tex = load_texture_from_rendered_text(
						  		  ins_text, COLOR_WHITE);
	
	int x = get_code_line_x();
	int y = get_first_code_line_y();

	for (int i = 0; i <= list_size; i++){
		y += CODE_BUTTON_H;
	}

	button_t *b = create_button(x, y, CODE_BUTTON_W, CODE_BUTTON_H,
								true, false, instruction_tex);
	check_mem(b);
	instruction_t *new_ins = cl_create_instruction(ins_id, b);
	code_line_t *new_line = cl_create_code_line(new_ins);
	
	
	if (operand_quantity == ONE_OPERAND || operand_quantity == TWO_OPERANDS){
		operand_t *op1;
		if (op1_id > REGISTERS_MIN && op1_id < REGISTERS_MAX){
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


/* Function: get_code_line_at_pos
 *------------------------------------------------------------------------------
 * Arguments:
 *	int: the position of the code_line that will be recovered.
 *
 * Return:
 *	The code line at a given position 
 */
code_line_t *get_code_line_at_pos(int pos)
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

		if (true == check_mouse_click_in_button(c->ins->b)){
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
		
		if (true == check_mouse_click_in_button(c->ins->b)){
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
	int text_length = strlen(text);
	stage_name = malloc(sizeof(char)*text_length);
	check_mem(stage_name);
	strcpy(stage_name, text);
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
	free(stage_name);
	free(challenge_text);
	
	List *code = get_code_list();
	LIST_FOREACH(code, first, next, cur){
		code_line_t *line = cur->value;	
		cl_destroy_code_line(line);
	}
	List_destroy(code_list);
	code_list = NULL;
}


/* Function: get_code_box_member
 * -----------------------------------------------------------------------------
 * This function access and returns the members of the code box object.
 *
 * Arguments:
 * member: The specific member that is being requested.
 *
 * Return:
 *	The accessed member.
 */
static int get_code_box_member(int member)
{
	assert(member >= MEMBER_X && member <= MEMBER_H &&  "Member is incorrect");

	int return_value;
	switch (member){
		
		case MEMBER_X:
			return_value = code_box.x;
			break;
		case MEMBER_Y:
			return_value = code_box.y;
			break;
		case MEMBER_W:
			return_value = code_box.w;
			break;
		case MEMBER_H:
			return_value = code_box.h;
			break;
		default:
			return_value = INVALID_MEMBER;
			break;
	}
	return return_value;
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
	assert(member >= MEMBER_X && member <= MEMBER_H &&  "Member is incorrect");

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
/* Function: get_text_box_member
 * -----------------------------------------------------------------------------
 * This function access and returns the members of the text box object.
 *
 * Arguments:
 * member: The specific member that is being requested.
 *
 * Return:
 *	The accessed member.
 */
static int get_text_box_member(int member)
{
	assert(member >= MEMBER_X && member <= MEMBER_H &&  "Member is incorrect");

	int return_value;
	switch (member){
		
		case MEMBER_X:
			return_value = text_box.x;
			break;
		case MEMBER_Y:
			return_value = text_box.y;
			break;
		case MEMBER_W:
			return_value = text_box.w;
			break;
		case MEMBER_H:
			return_value = text_box.h;
			break;
		default:
			return_value = INVALID_MEMBER;
			break;
	}
	return return_value;
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
	assert(NULL != text && "The text pointer is NULL");
	
	int text_length = strlen(text);
	challenge_text = malloc(sizeof(char)*text_length);
	strcpy(challenge_text, text);
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
void cw_set_code_box(int x, int y, int w, int h)
{
	code_box.x = x;
	code_box.y = y;
	code_box.w = w;
	code_box.h = h;

	int text_box_height = get_wrapped_text_height(TEXT_BOX_WIDTH, 
												  TEXT_BOX_HEIGHT,
												  challenge_text);
	int stage_name_height = get_text_height(stage_name, STAGE_NAME_SCALE);
	set_text_box(x + CODE_BOX_OFFSET, y + CODE_BOX_OFFSET + stage_name_height
				 + CODE_BOX_OFFSET, TEXT_BOX_WIDTH, text_box_height);
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

/* Function: get_first_code_line_y
 * -----------------------------------------------------------------------------
 * This function calculates and returns the y position of the first line of 
 * code.
 *
 * Arguments:
 * 	none.
 *
 * Return:
 *	int with the coordinate of the first line of code
 */
static int get_first_code_line_y()
{
	int y = get_text_box_member(MEMBER_Y) + get_text_box_member(MEMBER_H) + 
			CODE_BOX_OFFSET - ADJUSTING_OFFSET;
	return y;
}

/* Function: get_code_line_x
 * -----------------------------------------------------------------------------
 * This function calculates and returns the x position of the lines of code
 * that will be incorporated in the code box.
 *
 * Arguments:
 * 	none.
 *
 * Return:
 *	int with the coordinate of the x coordinates for the lines of code
 */
static int get_code_line_x()
{
	int x = get_code_box_member(MEMBER_X) + CODE_BOX_NUMBER_WIDTH + 
			2*LINE_NUMBER_OFFSET;	
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

	if (CODE_LINES_SIZE < list_size){
		
		int increase_size = (list_size - CODE_LINES_SIZE)*CODE_BUTTON_H;
		
		int new_bottom_border = get_code_box_member(MEMBER_Y) + CODE_BOX_H + 
							  	increase_size;

		int cur_bottom_border = get_code_box_member(MEMBER_H) + 
								get_code_box_member(MEMBER_Y);

		int delta = get_movement_delta(cur_bottom_border, new_bottom_border, 
									   MOVEMENT_DELTA/2);
		
		if (new_bottom_border > cur_bottom_border){
			cur_bottom_border += delta;
			set_code_box_member(cur_bottom_border - 
								get_code_box_member(MEMBER_Y), MEMBER_H);
		} else if (new_bottom_border < cur_bottom_border){
			cur_bottom_border -= delta;
			set_code_box_member(cur_bottom_border - 
								get_code_box_member(MEMBER_Y), MEMBER_H);
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
	int cur_y = get_code_box_member(MEMBER_Y);
	int max_upper_border_y = CODE_BOX_Y;
	int min_lower_border_y = CODE_BOX_Y + CODE_BOX_H;

	if (cur_y >= max_upper_border_y && y_pending_scroll > 0){
		y_pending_scroll = 0;
	} 
	if ((cur_y + get_code_box_member(MEMBER_H)) <= min_lower_border_y 
			   && y_pending_scroll < 0){
		y_pending_scroll = 0;
	}

	int delta = get_movement_delta(0, y_pending_scroll, MOVEMENT_DELTA);	

	if (0 != y_pending_scroll){
		int text_box_y = get_text_box_member(MEMBER_Y);
		
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

	// Challenge text
	dw_draw_wrapped_text_fits_height(text_box.x, text_box.y, text_box.w, 
								  TEXT_BOX_HEIGHT, COLOR_WHITE, challenge_text);

	// Text rectangle
	draw_rectangle(text_box.x, text_box.y, text_box.w, text_box.h, 
	               COLOR_WHITE);
	
	// Adjust the height of the code box
	code_box_height_adjust();

	if (check_if_inside_code_window() == true){
		adjust_code_box_position();
	}
	// Draw the rectangle of the code and instructions
	draw_rectangle(code_box.x, code_box.y, code_box.w, code_box.h, 
				   COLOR_WHITE);

	// Text of the level
	draw_text(code_box.x + CODE_BOX_OFFSET, code_box.y + CODE_BOX_OFFSET, 
						   STAGE_NAME_SCALE, COLOR_WHITE, stage_name);

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
			draw_text_fits_height(line->ins->b->x + 2*CODE_BUTTON_W + 
								 LINE_NUMBER_OFFSET, line->ins->b->y, 
								 CODE_BUTTON_H, COLOR_WHITE, ",");
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
			y = c->ins->b->y;
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

	int x = get_code_box_member(MEMBER_X) + LINE_NUMBER_OFFSET;
	int y = get_text_box_member(MEMBER_Y) + get_text_box_member(MEMBER_H) + 
			CODE_BOX_OFFSET;
	int h = CODE_BUTTON_H;
	char *number = NULL;
	
	int line_number = 1;
	for(int i = 0; i < list_size; i++){
	int instruction = cw_get_instruction_at_code_pos(i);
		if (instruction != LABEL){
			if (line_number < 10){
				number = number_to_string_with_prepend_zero(line_number);
			} else {
				number = number_to_string(line_number);
			}	
				draw_text_fits_height(x, y, CODE_BUTTON_H, COLOR_WHITE, number);
				free(number);
			line_number++;
		}
		y += CODE_BUTTON_H;
	}
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
	
	int w = CODE_BOX_NUMBER_WIDTH;
	int h = CODE_BUTTON_H; 
	int x = get_code_line_x();
	int y = get_first_code_line_y();

	LIST_FOREACH(code, first, next, cur){

		code_line_t *line = cur->value;
		
		int delta = get_movement_delta(line->ins->b->x, x, MOVEMENT_DELTA);	
		
		if (line->ins->b->x < x){
			line->ins->b->x += delta;
		} else if (line->ins->b->x > x){
			line->ins->b->x -= delta;
		}
		
		delta = get_movement_delta(line->ins->b->y, y, MOVEMENT_DELTA);	
		
		if (line->ins->b->y < y){
			line->ins->b->y += delta;
		} else if (line->ins->b->y > y){
			line->ins->b->y -= delta;
		}

		if (line->op1 != NULL){
				line->op1->b->x = line->ins->b->x + OP1_X_OFFSET;
				line->op1->b->y = line->ins->b->y;
		}
		if (line->op2 != NULL){
				line->op2->b->x = line->ins->b->x + OP2_X_OFFSET;
				line->op2->b->y = line->ins->b->y;
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
	int first_y = get_first_code_line_y();

	int h = CODE_BUTTON_H;
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
	int y = first->ins->b->y;
	
	if (mouse_y < y){
		List_unshift(code, line);
		return;
	} 

	code_line_t *last = code->last->value;
	y = last->ins->b->y;
	int h = last->ins->b->h;
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
	
	int y = get_first_code_line_y();

	int mouse_y = ms_get_mouse_y();
	bool ret = false;
	int h = CODE_BUTTON_H;
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

	line->ins->b->x = ms_get_mouse_x() - line->ins->b->w/2;
	line->ins->b->y = ms_get_mouse_y() - line->ins->b->h/2;
	bt_draw_button(line->ins->b);
	
	if (line->op1 != NULL){
		line->op1->b->x = line->ins->b->x + OP1_X_OFFSET;
		line->op1->b->y = line->ins->b->y;
		bt_draw_button(line->op1->b);
	}
	if (line->op2 != NULL){
		line->op2->b->x = line->ins->b->x + OP2_X_OFFSET;
		line->op2->b->y = line->ins->b->y;
		bt_draw_button(line->op2->b);
	}

	if (check_if_inside_code_window() == true){
		
		if (cw_check_if_in_code_list(line) == true){
			if (check_selected_line_in_position(line) == false){
				ListNode *node = get_list_node_by_value(line);
				List_remove(code, node);
			}
		} 
		if (cw_check_if_in_code_list(line) == false){
			add_code_line(line);
		}
		if (line->ins->id == LABEL){
			if (line->op1 == NULL){
		//	operand_t *l = create_label_operand();
		}
		}
	} else {
		if (cw_check_if_in_code_list(line) == true){
			ListNode *node = get_list_node_by_value(line);
			List_remove(code, node);
		}
	}
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
		if (MISSING_BOTH == s || MISSING_OP1 == s || MISSING_OP2 == s ||
			CHANGING_OP1 == s || CHANGING_OP2 == s){
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

	if (MISSING_BOTH == line->state || MISSING_OP1 == line->state ||
		CHANGING_OP1 == line->state){

		draw_rectangle(line->ins->b->x+OP1_X_OFFSET, line->ins->b->y,
				   	   line->ins->b->w, line->ins->b->h, COLOR_WHITE);

	} else if (MISSING_OP2 == line->state || CHANGING_OP2 == line->state){
		draw_rectangle(line->ins->b->x+OP2_X_OFFSET, line->ins->b->y,
				   	   line->ins->b->w, line->ins->b->h, COLOR_WHITE);

	}
error:
	return;
}

/* Function: cw_check_all_code_sorted
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
bool cw_check_all_code_sorted()
{
	List *code = get_code_list();
	int list_size = List_count(code);
	if (0 == list_size){
		return true;
	}
	
	int w = CODE_BOX_NUMBER_WIDTH;
	int h = CODE_BUTTON_H;
	int x = get_code_line_x();
	int y = get_first_code_line_y();

	int retval = true;
	LIST_FOREACH(code, first, next, cur){

		code_line_t *value = cur->value;
		button_t *b = value->ins->b;
		
		if (b->x < x || b->x > x || b->y < y || b->y > y){
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
			if (true == check_mouse_click_in_button(c->op1->b)){
				clicked = true;
				break;
			}
		} 
		if (NULL != c->op2){
			if (true == check_mouse_click_in_button(c->op2->b)){
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
		if (NULL != c->op1){
			if (true == check_mouse_click_in_button(c->op1->b)){
				c->state = CHANGING_OP1;
				break;
			}
		} 
		if (NULL != c->op2){
			if (true == check_mouse_click_in_button(c->op2->b)){
				c->state = CHANGING_OP2;
				break;
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
