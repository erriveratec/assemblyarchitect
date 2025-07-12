#include <SDL.h>
#include <assert.h>
#include "game_mechanics_mc.h"
#include "aux.h"
#include "draw_dw.h"
#include "code_window_cw.h"
#include "mouse_ms.h"
#include "dimensions.h"
#include "registers_rg.h" 
#include "buffers_bf.h"
#include "dbg.h"
#include "levels_lv.h"

#define EMPTY 0

static int g_invalid_operation_flag = NO_INVALID_OPERATION;

typedef struct avatar_t{
	SDL_Rect box;
	bool in_place;
	bool op2_retrieved;
	bool op1_delivered;
	value_box_t value;
} avatar_t;

static avatar_t g_avatar; 

enum counter_actions{
	ADD_1,
	SUB_1,
	RESET_COUNTER,
	SET_COUNTER,
	GET_COUNTER,
};

static void execute_instruction(code_line_t *line);
bool move_avatar_to_operand(int op_id);
int get_operand_x_dest(int op_id);
int get_operand_y_dest(int op_id);
static value_box_t get_operand_value_box(int op_id);
bool set_operand_value_box(int op_id, value_box_t val);
void operate_instruction(code_line_t *line);
void reset_avatar_no_pos();
static void handle_source_operand(code_line_t *line);
static void handle_destiny_operand(code_line_t *line);
static bool retrieve_operand();
static bool is_operand_retrievable(int id);
static void set_invalid_operation_flag(int flag_id);

/* Function: mc_reset_invalid_operation_flag
 * -----------------------------------------------------------------------------
 * Resets the invalid operation flag when the player has pressed stop
 * 
 * Arguments:
 * 	None.
 *
 * Return:
 *	void.
 */
void mc_reset_invalid_operation_flag()
{
	set_invalid_operation_flag(NO_INVALID_OPERATION);
}

/* Function: mc_get_invalid_operation_flag
 * -----------------------------------------------------------------------------
 * Returns the flag with the id of the invalid operation that was performed.
 * NO_INVALID_OPERATION is used if everything is ok.
 * 
 * Arguments:
 * 	void.
 *
 * Return:
 *	flag_id:  The id with the flag of the invalid operation.
 */
int mc_get_invalid_operation_flag()
{
	return g_invalid_operation_flag;
}

/* Function: mc_set_invalid_operation_flag
 * -----------------------------------------------------------------------------
 * Sets the value of the invalid_operation flag accordingly.
 * 
 * Arguments:
 * 	flag_id: The id the invalidi operation.
 *
 * Return:
 *	void.
 */
static void set_invalid_operation_flag(int flag_id)
{
	g_invalid_operation_flag = flag_id;
}

/* Function: mc_reset_avatar
 * -------------------------------------
 * Arguments:
 * 	void.
 *
 * Return:
 *	true: used for the caller of the function.
 */
void mc_reset_avatar()
{
	g_avatar.box.x = AVATAR_START_X;
	g_avatar.box.y = AVATAR_START_Y;
	g_avatar.box.w = AVATAR_W;
	g_avatar.box.h = AVATAR_H;

	g_avatar.value.visible_box = false;
	g_avatar.in_place = false;
	g_avatar.op2_retrieved = false;
	g_avatar.op1_delivered = false;

	g_avatar.value.value = NO_VALUE;
	g_avatar.value.box.x = AVATAR_START_X;
	g_avatar.value.box.y = AVATAR_START_Y - AVATAR_H;
	g_avatar.value.box.w = VALUE_BOX_W;
	g_avatar.value.box.h = VALUE_BOX_H;

}

/* Function: reset_avatar_no_pos
 * -------------------------------------
 * Arguments:
 * 	void.
 *
 * Return:
 *	None.
 */
void reset_avatar_no_pos()
{
	g_avatar.value.visible_box = false;
	g_avatar.in_place = false;
	g_avatar.op2_retrieved = false;
	g_avatar.op1_delivered = false;

	g_avatar.value.value = NO_VALUE;
	g_avatar.value.box.x = g_avatar.box.x;
	g_avatar.value.box.y = g_avatar.box.y - AVATAR_H;
}

/* Function: mc_draw_avatar
 * ----------------------------------------------------------------------------
 * Arguments:
 * 	avatar: the avatar that is going to be drawn
 *
 * Return:
 *	void.
 */
void mc_draw_avatar()
{
	if (g_avatar.value.visible_box == true){
		draw_value_box(&g_avatar.value);
	}
	dw_draw_filled_rectangle(g_avatar.box.x, g_avatar.box.y, g_avatar.box.w, 
						  	 g_avatar.box.h, COLOR_WHITE, COLOR_WHITE);
}

/* Function: execution_counter
 * -----------------------------------------------------------------------------
 * Arguments:
 *	action: action of the execution counter
 *
 * Return:
 *	void.
 */
int execution_counter(int action, int set)
{
	static int execution_counter = 1;
	switch (action){
		case ADD_1:
			execution_counter += 1;
			break;
		case SUB_1: 
			execution_counter -= 1;
			break;
		case RESET_COUNTER:
			execution_counter = 1;
			break;
		case SET_COUNTER:
			execution_counter = set;
			break;
		case GET_COUNTER:
			break;
	}	

	return execution_counter;
}

/* Function: get_operand_y_dest
 * -----------------------------------------------------------------------------
 * Arguments:
 *	op_id: id of the operand
 *
 * Return:
 *	y coordinate destination for the avatar.
 */
int get_operand_y_dest(int op_id)
{
	int y;
	if (op_id > REGISTERS_MIN && op_id < REGISTERS_MAX){
		y = get_register_value_box_y_coord_by_id(op_id);
	} else if (op_id > BUFFERS_MIN && op_id < BUFFERS_MAX){
		y = get_buffer_value_box_y_coord_by_id(op_id);	
	}
	return y;
}

/* Function: get_operand_x_dest
 * -----------------------------------------------------------------------------
 * Arguments:
 *	op_id: id of the operand
 *
 * Return:
 *	x coordinate destination for the avatar.
 */
int get_operand_x_dest(int op_id)
{
	int x;
	if (op_id > REGISTERS_MIN && op_id < REGISTERS_MAX){
		x = get_register_value_box_x_coord_by_id(op_id);
	} else if (op_id > BUFFERS_MIN && op_id < BUFFERS_MAX){
		x = get_buffer_value_box_x_coord_by_id(op_id);	
	}
	return x;
}

/* Function: move_avatar_to_operand
 * -----------------------------------------------------------------------------
 * Arguments:
 *	operand_id: The id of the operand where the avatar will move
 *
 * Return:
 *	bool indicanting if there still movement pending
 */
bool move_avatar_to_operand(int op_id)
{
	assert(op_id > REGISTERS_MIN && op_id < BUFFERS_MAX &&
		   "Invalid operand");

	bool mov = false;
	int x;
	int y;

	if (op_id > REGISTERS_MIN && op_id < REGISTERS_MAX){
		x = get_operand_x_dest(op_id) + AVATAR_X_POS_OFFSET;
	} else{
		x = get_operand_x_dest(op_id);
	}

	if (op_id > REGISTERS_MIN && op_id < REGISTERS_MAX){
		y = get_operand_y_dest(op_id);
	} else if (op_id == IB){
		y = get_operand_y_dest(op_id) + 1.5*VALUE_BOX_H;
	} else if (op_id == OB){
		y = get_operand_y_dest(op_id) - 2.5*VALUE_BOX_H;
	}
	
	if (g_avatar.box.x < x){
		int delta = get_movement_delta(g_avatar.value.box.x, x, MOVEMENT_DELTA);
		g_avatar.box.x += delta;
		g_avatar.value.box.x += delta;
		mov = true;
	} else if (g_avatar.box.x > x){
		int delta = get_movement_delta(g_avatar.value.box.x, x, MOVEMENT_DELTA);
		g_avatar.box.x -= delta;
		g_avatar.value.box.x -= delta;
		mov = true;
	}
	
	int avatar_final_y = g_avatar.box.y - AVATAR_H;
	if (avatar_final_y < y){
		int delta = get_movement_delta(g_avatar.value.box.y, y, MOVEMENT_DELTA);
		g_avatar.box.y += delta;
		g_avatar.value.box.y += delta;
		mov = true;
	} else if (avatar_final_y > y){
		int delta = get_movement_delta(g_avatar.value.box.y, y, MOVEMENT_DELTA);
		g_avatar.box.y -= delta;
		g_avatar.value.box.y -= delta;
		mov = true;
	}

	return mov;
}

/* Function: get_operand_value_box
 * -----------------------------------------------------------------------------
 * Arguments:
 *	op_id: the id of the operand the avatar will retrieve
 *
 * Return:
 *	bool indicating if part of the retriving is pending
 */
static value_box_t get_operand_value_box(int op_id)
{
	assert((op_id > REGISTERS_MIN && op_id < REGISTERS_MAX) ||
		   (op_id > BUFFERS_MIN &&  op_id < BUFFERS_MAX) && 
		   "The operand id is invalid");

	value_box_t op_value_box;

	if (op_id > REGISTERS_MIN && op_id < REGISTERS_MAX){
		op_value_box = get_register_value_box_by_id(op_id);	
	} else if (op_id == IB){
		op_value_box = bf_get_input_buffer_value_box();
	} else if (op_id == OB){
		op_value_box = bf_get_output_buffer_value_box();
	}
	return op_value_box;
}

/* Function: set_operand_value_box
 * -----------------------------------------------------------------------------
 * This function sets the value of the operand with the value of the avatar.
 *
 * Arguments:
 *	op_id: the id of the operand the avatar will set
 *  val: the avatar value box that will modify the operand.
 *
 * Return:
 *	bool indicating if the operation performed was valid
 */
bool set_operand_value_box(int op_id, value_box_t val)
{
	assert(op_id > REGISTERS_MIN && op_id < BUFFERS_MAX && 
		   "The operand id is invalid");
	
	bool operation_valid = true;

	if (op_id > REGISTERS_MIN && op_id < REGISTERS_MAX){
		rg_set_register_value_box(op_id, val);	
	} else if (op_id == OB){
		set_output_buffer_value_box(val);	
	}

	return operation_valid;
}

/* Function: add_operand_value_box
 * -----------------------------------------------------------------------------
 * Adds the current value of the operand with the value provided by the avatar.
 * if the operand value is not valid, it returns an invalid operation.
 *  
 * Arguments:
 *	op_id: the id of the operand the avatar will retrieve
 *  val: the avatar value box that will modify the operand.
 *
 * Return:
 *	bool indicating if part of the retriving is pending
 */
bool add_operand_value_box(int op_id, value_box_t val)
{ 
	assert(op_id > REGISTERS_MIN && op_id < BUFFERS_MAX && 
		   "The operand id is invalid");
	
	bool operation_valid = true;

	if (op_id > REGISTERS_MIN && op_id < REGISTERS_MAX){
		value_box_t cur_val = get_register_value_box_by_id(op_id);
		if (cur_val.value == NO_VALUE){
			operation_valid = false;	
		} else {
			cur_val.value += val.value;
			rg_set_register_value_box(op_id, cur_val);	
		}
	} else if (op_id == OB){
		set_output_buffer_value_box(val);	
	}
	return operation_valid;
}
/* Function: operate_instruction
 * -----------------------------------------------------------------------------
 * Arguments:
 *	line: line of code that will be executed
 *
 * Return:
 *	bool indicating if part of the retriving is pending
 */
void operate_instruction(code_line_t *line)
{
	bool op_status;
	switch (line->ins->id){
		case MOV:
			set_operand_value_box(line->op1->id, g_avatar.value);
			break;
		case ADD:
			op_status = add_operand_value_box(line->op1->id, g_avatar.value);
			if (op_status == false){
				set_invalid_operation_flag(REG_VALUE_NOT_INITIALIZED);
			}
			break;

	}
}

/* Function: deliver_operand
 * -----------------------------------------------------------------------------
 * Arguments:
 *	op_id: the id of where the operand will be delivered
 *
 * Return:
 *	bool indicating if part of the retriving is pending
 */
bool deliver_operand(int op_id)
{
	value_box_t v = get_operand_value_box(op_id);

	int x = v.box.x;
	int y = v.box.y;	

	bool mov = false;

	if (g_avatar.value.box.x < x){
		int delta = get_movement_delta(g_avatar.value.box.x, x, 
									   MOVEMENT_DELTA/2);
		g_avatar.value.box.x += delta;
		mov = true;
	} else if (g_avatar.value.box.x > x){
		int delta = get_movement_delta(g_avatar.value.box.x, x, 
									   MOVEMENT_DELTA/2);
		g_avatar.value.box.x -= delta;
		mov = true;
	}

	if (g_avatar.value.box.y < y){
		int delta = get_movement_delta(g_avatar.value.box.y, y, 
									   MOVEMENT_DELTA/2);
		g_avatar.value.box.y += delta;
		mov = true;
	} else if (g_avatar.value.box.y > y){
		int delta = get_movement_delta(g_avatar.value.box.y, y, 
									   MOVEMENT_DELTA/2);
		g_avatar.value.box.y -= delta;
		mov = true;
	}
	return mov;
}

/* Function: retrieve_operand
 * -----------------------------------------------------------------------------
 * This function move the avatar value box.
 *
 * Arguments:
 *	void
 *
 * Return:
 *	bool indicating if part of the retriving is pending
 */
static bool retrieve_operand()
{
	int x = g_avatar.box.x;
	int y = g_avatar.box.y - AVATAR_H;	

	bool mov = false;

	if (g_avatar.value.box.x < x){
		int delta = get_movement_delta(g_avatar.value.box.x, x, 
									   MOVEMENT_DELTA/2);
		g_avatar.value.box.x += delta;
		mov = true;
	} else if (g_avatar.value.box.x > x){
		int delta = get_movement_delta(g_avatar.value.box.x, x, 
									   MOVEMENT_DELTA/2);
		g_avatar.value.box.x -= delta;
		mov = true;
	}

	if (g_avatar.value.box.y < y){
		int delta = get_movement_delta(g_avatar.value.box.y, y, 
									   MOVEMENT_DELTA/2);
		g_avatar.value.box.y += delta;
		mov = true;
	} else if (g_avatar.value.box.y > y){
		int delta = get_movement_delta(g_avatar.value.box.y, y, 
									   MOVEMENT_DELTA/2);
		g_avatar.value.box.y -= delta;
		mov = true;
	}
	return mov;
}
/* Function: is_operand_retrievable
 * -----------------------------------------------------------------------------
 * Arguments:
 *	id: The id of the operand that will be verified if it is retrivable
 *
 * Return:
 *	bool inticating if the operand is retrievable or not.
 */
static bool is_operand_retrievable(int id)
{
	bool retrievable = true;
	
	if (id == IB){
		int list_size = get_input_buffer_list_size();
		if (list_size == 0){
			retrievable = false;
		}
	}
	return retrievable;
}

/* Function: handle_source_operand
 * -----------------------------------------------------------------------------
 * Arguments:
 *	line: the code line that will be executed
 *
 * Return:
 *	void.
 */
static void handle_source_operand(code_line_t *line)
{
	assert(line != NULL && "The value of line cannot be NULL");
	int operand_quantity = get_instruction_operand_quantity(line->ins->id);
	int mov_pending = NO_VALUE;

	if (operand_quantity == TWO_OPERANDS && g_avatar.op2_retrieved == false){
		mov_pending = move_avatar_to_operand(line->op2->id);
		if (mov_pending == false && g_avatar.in_place == false){
			g_avatar.in_place = true;
			if (is_operand_retrievable(line->op2->id) == true){	
				g_avatar.value = get_operand_value_box(line->op2->id);
				g_avatar.value.visible_box = true;
			}
		} 
		if (mov_pending == false){
			int retrieve_pending = retrieve_operand();
			if (retrieve_pending == false){
				g_avatar.op2_retrieved = true;
				g_avatar.in_place = false;
				if (line->op1->id == OB){
					add_output_to_list();
				}
			}
		}
	}
}

/* Function: handle_destiny_operand
 * -----------------------------------------------------------------------------
 * Arguments:
 *	line: the code line that will be executed
 *
 * Return:
 *	void.
 */
static void handle_destiny_operand(code_line_t *line)
{
	assert(line != NULL && "The value of line cannot be NULL");
	int mov_pending = NO_VALUE;
	if (g_avatar.op2_retrieved == true){
		mov_pending = move_avatar_to_operand(line->op1->id);
		if (mov_pending == false && g_avatar.in_place == false){
			g_avatar.in_place = true;
		}
		if (mov_pending == false){
			int deliver_pending = deliver_operand(line->op1->id);
			if (deliver_pending == false){
				g_avatar.op1_delivered = true;
				g_avatar.value.visible_box = false;
			}
		}
	}
}


/* Function: execute_instruction
 * -----------------------------------------------------------------------------
 * Arguments:
 *	The code line that will be executed
 *
 * Return:
 *	void.
 */
static void execute_instruction(code_line_t *line)
{
	assert(line != NULL && "The value of line cannot be NULL");
	int buffer_inputs = get_input_buffer_list_size();


	if (buffer_inputs != EMPTY || line->state == IN_EXECUTION || 
													 line->state == COMPLETE){
		line->state = IN_EXECUTION;
		handle_source_operand(line);
		handle_destiny_operand(line);
		if (g_avatar.op2_retrieved == true && g_avatar.op1_delivered == true){
			operate_instruction(line);
			line->state = EXECUTED;
			reset_avatar_no_pos();
		}
	}
}

/* Function: mc_run_code
 * -----------------------------------------------------------------------------
 * Arguments:
 *	none.
 *
 * Return:
 *	true if finished running code, false otherwise.
 */
bool mc_run_code()
{
	bool finished = false;
	int code_size = cw_get_code_list_size();	

	for (int i = 0; i < code_size; i++){
		code_line_t *line = get_code_line_at_pos(i);
		if (line->state != EXECUTED){
			execute_instruction(line);	
			return finished;
		}
	}
	finished = true;
	return finished;
}

