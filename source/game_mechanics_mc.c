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

#define INPUT_BUFFER_EMPTY_TEXT "The Input Buffer is empty"
#define REG_VALUE_INVALID_TEXT "The register had an invalid value"
#define INVALID_OUTPUT_VALUE_TEXT "Incorrect value in the output buffer"
#define UNPROCESSED_IB_VALUES_TEXT "Output is correct but only works by that" \
							   "specific set of value"
#define EXCEEDS_CODE_LIMIT_TEXT "Correct output but exceeds code size limit"

#define AVATAR_BUFFER_OFFSET 50

#define IAVATAR_X BUFFER_BOX_X - 300
#define IAVATAR_Y INPUT_BUFFER_BOX_Y + BUFFER_BOX_H + AVATAR_BUFFER_OFFSET

#define OAVATAR_X BUFFER_BOX_X - 300
#define OAVATAR_Y OUTPUT_BUFFER_BOX_Y - BUFFER_BOX_H - AVATAR_BUFFER_OFFSET

#define AVATAR_START_X 900
#define AVATAR_START_Y 500
#define AVATAR_W 50
#define AVATAR_H 50
#define AVATAR_X_POS_OFFSET (AVATAR_W + 20)

enum avatar_id{
	NOAVATAR,
	IAVATAR,
	OAVATAR,
};

texture_t *right_arrow = NULL;

static int g_invalid_operation_flag = NO_INVALID_OPERATION;

typedef struct avatar_t{
	SDL_Rect box;
	bool in_place;
	bool op2_retrieved;
	bool op1_delivered;
	value_box_t value;
} avatar_t;

static avatar_t g_iavatar;
static avatar_t g_oavatar;
static avatar_t g_avatar; 

typedef struct execution_arrow_t{
	SDL_Rect box;
	bool in_place;
	bool visible;
} execution_arrow_t;

static execution_arrow_t g_exec_arrow;

enum counter_actions{
	ADD_1,
	SUB_1,
	RESET_COUNTER,
	SET_COUNTER,
	GET_COUNTER,
};

static void execute_instruction(code_line_t *line, int line_pos);
static bool move_avatar_to_operand(avatar_t *avatar, int op_id);
int get_operand_x_dest(int op_id);
int get_operand_y_dest(int op_id);
static value_box_t get_operand_value_box(int op_id);
bool set_operand_value_box(int op_id, value_box_t val);
void operate_instruction(code_line_t *line, value_box_t value);
void reset_avatar_no_pos();
static int handle_source_operand(code_line_t *line);
static void handle_destiny_operand(code_line_t *line, int avatar_id);
static bool retrieve_operand(avatar_t *avatar);
static bool deliver_operand(avatar_t *avatar, int op_id);
static bool is_operand_retrievable(int id);
static void set_invalid_operation_flag(int flag_id);
static bool move_execution_arrow(int instruction_number);
static bool check_operand_has_valid_value(int op_id);
static bool check_run_finished();
static bool check_correct_code_size();
static bool handle_iavatar_source_operand(int op_id);
static bool handle_oavatar_source_operand(int op_id);

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

/* Function: mc_get_operation_flag
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
int mc_get_operation_flag()
{
	return g_invalid_operation_flag;
}

/* Function: invalid_operation_handler
 * -----------------------------------------------------------------------------
 * This function is called in all the stages, an invalid operation message
 * the nature of the message will depend accordingly to an identifier
 * 
 * Arguments:
 *	id: The identifier of the exception that ocurred.
 *
 * Return:
 *	void.
 */
bool mc_invalid_operation_handler(int id)
{
	assert(id >= NO_INVALID_OPERATION && id < INVALID_OPERATION_MAX &&
		   "Incorrect id for the invalid operation handler");

	bool reset_level = false;

	dw_draw_filled_rectangle(MESSAGE_BOX_X, MESSAGE_BOX_Y, MESSAGE_BOX_W, 
					   		 MESSAGE_BOX_H, COLOR_BLACK, COLOR_WHITE);
	char *message = NULL;

	switch(id){
		case INPUT_BUFFER_EMPTY:
			message = malloc(sizeof(char)*strlen(INPUT_BUFFER_EMPTY_TEXT)+1);
			strcpy(message, INPUT_BUFFER_EMPTY_TEXT);	
			break;
		case REG_VALUE_INVALID:
			message = malloc(sizeof(char)*strlen(REG_VALUE_INVALID_TEXT)+1);
			strcpy(message, REG_VALUE_INVALID_TEXT);	
			break;
		case INVALID_OUTPUT_VALUE:
			message = malloc(sizeof(char)*strlen(INVALID_OUTPUT_VALUE_TEXT)+1);
			strcpy(message, INVALID_OUTPUT_VALUE_TEXT);	
			break;
		case UNPROCESSED_IB_VALUES:
			message = malloc(sizeof(char)*strlen(UNPROCESSED_IB_VALUES_TEXT)+1);
			strcpy(message, UNPROCESSED_IB_VALUES_TEXT);	
			break;
		case EXCEEDS_CODE_LIMIT:
			message = malloc(sizeof(char)*strlen(EXCEEDS_CODE_LIMIT_TEXT)+1);
			strcpy(message, EXCEEDS_CODE_LIMIT_TEXT);	
			break;
		default: 
			puts("ERROR: Invalid operation incorrec id");
	}

	dw_draw_wrapped_text_fits_height(MESSAGE_TEXT_X, MESSAGE_TEXT_Y, 
									 MESSAGE_TEXT_W, MESSAGE_TEXT_H, 
									 COLOR_WHITE, message);
	
	free(message);

	if (ms_get_mouse_left_released() == true){
		mc_reset_invalid_operation_flag();
		reset_level = true;
	}
	return reset_level;
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
 *	void.
 */
void mc_reset_avatar()
{
	g_iavatar.box.x = IAVATAR_X;
	g_iavatar.box.y = IAVATAR_Y;
	g_iavatar.box.w = AVATAR_W;
	g_iavatar.box.h = AVATAR_H;

	g_iavatar.value.visible_box = false;
	g_iavatar.in_place = false;
	g_iavatar.op2_retrieved = false;
	g_iavatar.op1_delivered = false;

	g_iavatar.value.value = NO_VALUE;
	g_iavatar.value.box.x = IAVATAR_X;
	g_iavatar.value.box.y = IAVATAR_Y - AVATAR_H;
	g_iavatar.value.box.w = VALUE_BOX_W;
	g_iavatar.value.box.h = VALUE_BOX_H;

	g_oavatar.box.x = OAVATAR_X;
	g_oavatar.box.y = OAVATAR_Y;
	g_oavatar.box.w = AVATAR_W;
	g_oavatar.box.h = AVATAR_H;

	g_oavatar.value.visible_box = false;
	g_oavatar.in_place = false;
	g_oavatar.op2_retrieved = false;
	g_oavatar.op1_delivered = false;

	g_oavatar.value.value = NO_VALUE;
	g_oavatar.value.box.x = OAVATAR_X;
	g_oavatar.value.box.y = OAVATAR_Y - AVATAR_H;
	g_oavatar.value.box.w = VALUE_BOX_W;
	g_oavatar.value.box.h = VALUE_BOX_H;

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
	g_iavatar.value.visible_box = false;
	g_iavatar.in_place = false;
	g_iavatar.op2_retrieved = false;
	g_iavatar.op1_delivered = false;

	g_iavatar.value.value = NO_VALUE;
	g_iavatar.value.box.x = g_iavatar.box.x;
	g_iavatar.value.box.y = g_iavatar.box.y - AVATAR_H;

	g_oavatar.value.visible_box = false;
	g_oavatar.in_place = false;
	g_oavatar.op2_retrieved = false;
	g_oavatar.op1_delivered = false;

	g_oavatar.value.value = NO_VALUE;
	g_oavatar.value.box.x = g_oavatar.box.x;
	g_oavatar.value.box.y = g_oavatar.box.y - AVATAR_H;

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
	if (g_iavatar.value.visible_box == true){
		draw_value_box(&g_iavatar.value);
	}
	dw_draw_filled_rectangle(g_iavatar.box.x, g_iavatar.box.y, g_iavatar.box.w, 
						  	 g_iavatar.box.h, COLOR_ORANGE, COLOR_ORANGE);

	if (g_oavatar.value.visible_box == true){
		draw_value_box(&g_oavatar.value);
	}
	dw_draw_filled_rectangle(g_oavatar.box.x, g_oavatar.box.y, g_oavatar.box.w, 
						  	 g_oavatar.box.h, COLOR_CYAN, COLOR_CYAN);

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

/* Function: mc_hide_execution_arrow
 * -------------------------------------
 * Arguments:
 * 	void.
 *
 * Return:
 *	void.
 */
void mc_hide_execution_arrow()
{
	g_exec_arrow.visible = false;

}

/* Function: mc_reset_execution_arrow
 * -------------------------------------
 * Arguments:
 * 	void.
 *
 * Return:
 *	void.
 */
void mc_reset_execution_arrow()
{
	g_exec_arrow.box.x = CODE_BOX_X + EXEC_ARROW_X_COORD_OFFSET;
	g_exec_arrow.box.y = cw_get_instruction_y_coord(0) + 
						 EXEC_ARROW_Y_COORD_OFFSET;
	g_exec_arrow.box.w = EXEC_ARROW_W;
	g_exec_arrow.box.h = EXEC_ARROW_H;

	g_exec_arrow.in_place = true;
	g_exec_arrow.visible = true;

}

/* Function: mc_draw_execution_arrow
 * ----------------------------------------------------------------------------
 * Arguments:
 * 	Void.
 *
 * Return:
 *	Void.
 */
void mc_draw_execution_arrow()
{
	if (g_exec_arrow.visible == true){
	draw_texture_fits_height(g_exec_arrow.box.x, g_exec_arrow.box.y, 
							 g_exec_arrow.box.h, right_arrow);
	}
}

/* Function: check_execution_arrow_in_place
 * -----------------------------------------------------------------------------
 * Arguments:
 *	Evaluates if the execution arrow is it it's final posiition
 *
 * Return:
 *	bool if the exectution arrow arrived to its final position
 */
static void check_execution_arrow_in_place(int instruction_number)
{
	int code_size = cw_get_code_list_size();	
	assert(code_size > 0  && "Code size is invalid");
	assert(instruction_number <= code_size && 
		   "Instruction number is incorrect");

	bool in_final_position = false;
	
	int y = cw_get_instruction_y_coord(instruction_number) + 
			EXEC_ARROW_Y_COORD_OFFSET;
	
	if (g_exec_arrow.box.y == y){
		int delta = get_movement_delta(g_exec_arrow.box.y, y, MOVEMENT_DELTA/6);
		g_exec_arrow.box.y += delta;
	} 
}


/* Function: move_execution_arrow
 * -----------------------------------------------------------------------------
 * Arguments:
 *	instruction_number: The number of the current instruction being executed
 *
 * Return:
 * 	Bool indicating if arrow is in final position
 */
static bool move_execution_arrow(int instruction_number)
{
	int code_size = cw_get_code_list_size();	
	assert(code_size > 0  && "Code size is invalid");
	assert(instruction_number <= code_size && 
		   "Instruction number is incorrect");
	
	bool in_final_position = false;

	int y = cw_get_instruction_y_coord(instruction_number) + 
			EXEC_ARROW_Y_COORD_OFFSET;
	
	if (g_exec_arrow.box.y < y){
		int delta = get_movement_delta(g_exec_arrow.box.y, y, MOVEMENT_DELTA/3);
		g_exec_arrow.box.y += delta;
	} else if (g_exec_arrow.box.y > y){
		int delta = get_movement_delta(g_exec_arrow.box.y, y, MOVEMENT_DELTA/3);
		g_exec_arrow.box.y -= delta;
	}
	if (g_exec_arrow.box.y == y){
		in_final_position = true;
	} 
	return in_final_position;
}

/* Function: move_avatar_to_operand
 * -----------------------------------------------------------------------------
 * Arguments:
 *	operand_id: The id of the operand where the avatar will move
 *
 * Return:
 *	bool indicanting if there still movement pending
 */
static bool move_avatar_to_operand(avatar_t *avatar, int op_id)
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
	
	if (avatar->box.x < x){
		int delta = get_movement_delta(avatar->value.box.x, x, MOVEMENT_DELTA);
		avatar->box.x += delta;
		avatar->value.box.x += delta;
		mov = true;
	} else if (avatar->box.x > x){
		int delta = get_movement_delta(avatar->value.box.x, x, MOVEMENT_DELTA);
		avatar->box.x -= delta;
		avatar->value.box.x -= delta;
		mov = true;
	}
	
	int avatar_final_y = avatar->box.y - AVATAR_H;
	if (avatar_final_y < y){
		int delta = get_movement_delta(avatar->value.box.y, y, MOVEMENT_DELTA);
		avatar->box.y += delta;
		avatar->value.box.y += delta;
		mov = true;
	} else if (avatar_final_y > y){
		int delta = get_movement_delta(avatar->value.box.y, y, MOVEMENT_DELTA);
		avatar->box.y -= delta;
		avatar->value.box.y -= delta;
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

/* Function: check_operand_has_valid_value
 * -----------------------------------------------------------------------------
 * Checks if an operand does not has a valid value
 * 
 *	Arguments:
 *	op_id: the id of the operand the avatar will retrieve
 *
 * Return:
 *	bool indicating if part of the retriving is pending
 */
static bool check_operand_has_valid_value(int op_id)
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
	bool value = true;
	if (op_value_box.value == NO_VALUE){
		value = false;
	}
	return value;
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
void operate_instruction(code_line_t *line, value_box_t value)
{
	bool op_status;
	switch (line->ins->id){
		case MOV:
			op_status = set_operand_value_box(line->op1->id, value);
			if (op_status == false){
				set_invalid_operation_flag(REG_VALUE_INVALID);
			}
			break;
		case ADD:
			op_status = add_operand_value_box(line->op1->id, value);
			if (op_status == false){
				set_invalid_operation_flag(REG_VALUE_INVALID);
			}
			break;
	}
	if (lv_evaluate_output_correctness() == false){
		set_invalid_operation_flag(INVALID_OUTPUT_VALUE);
	}
}

/* Function: deliver_operand
 * -----------------------------------------------------------------------------
 * This function is merely aesthetic, and it makes the delivery of the operand
 *
 * Arguments:
 *	op_id: the id of where the operand will be delivered
 *
 * Return:
 *	bool indicating if part of the retriving is pending
 */
static bool deliver_operand(avatar_t *avatar, int op_id)
{
	value_box_t v = get_operand_value_box(op_id);

	int x = v.box.x;
	int y = v.box.y;	

	bool mov = false;

	if (avatar->value.box.x < x){
		int delta = get_movement_delta(avatar->value.box.x, x, 
									   MOVEMENT_DELTA/2);
		avatar->value.box.x += delta;
		mov = true;
	} else if (avatar->value.box.x > x){
		int delta = get_movement_delta(avatar->value.box.x, x, 
									   MOVEMENT_DELTA/2);
		avatar->value.box.x -= delta;
		mov = true;
	}

	if (avatar->value.box.y < y){
		int delta = get_movement_delta(avatar->value.box.y, y, 
									   MOVEMENT_DELTA/2);
		avatar->value.box.y += delta;
		mov = true;
	} else if (avatar->value.box.y > y){
		int delta = get_movement_delta(avatar->value.box.y, y, 
									   MOVEMENT_DELTA/2);
		avatar->value.box.y -= delta;
		mov = true;
	}
	return mov;
}

/* Function: retrieve_operand
 * -----------------------------------------------------------------------------
 * This function moves the avatar value box. Is an aesthetic functions
 * as it does not changes the avatar box value
 *
 * Arguments:
 *	avatar: the avatar that will be animated
 *
 * Return:
 *	bool indicating if part of the retriving is pending
 */
static bool retrieve_operand(avatar_t *avatar)
{
	int x = avatar->box.x;
	int y = avatar->box.y - AVATAR_H;	

	bool mov = false;

	if (avatar->value.box.x < x){
		int delta = get_movement_delta(avatar->value.box.x, x, 
									   MOVEMENT_DELTA/2);
		avatar->value.box.x += delta;
		mov = true;
	} else if (avatar->value.box.x > x){
		int delta = get_movement_delta(avatar->value.box.x, x, 
									   MOVEMENT_DELTA/2);
		avatar->value.box.x -= delta;
		mov = true;
	}

	if (avatar->value.box.y < y){
		int delta = get_movement_delta(avatar->value.box.y, y, 
									   MOVEMENT_DELTA/2);
		avatar->value.box.y += delta;
		mov = true;
	} else if (avatar->value.box.y > y){
		int delta = get_movement_delta(avatar->value.box.y, y, 
									   MOVEMENT_DELTA/2);
		avatar->value.box.y -= delta;
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

/* Function: handle_ivatar_operand
 * -----------------------------------------------------------------------------
 * Performs the action of handling and moving the ivatar in the cases that are
 * applicable
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Void.
 */
static bool handle_iavatar_source_operand(int op_id)
{
	bool mov_pending = true;

	mov_pending = move_avatar_to_operand(&g_iavatar, op_id);
	if (mov_pending == false && g_iavatar.in_place == false && 
							  g_iavatar.op2_retrieved == false){
		g_iavatar.in_place = true;
		if (op_id == IB && is_operand_retrievable(op_id) == true){
			g_iavatar.value = get_operand_value_box(op_id);
			g_iavatar.value.visible_box = true;
		}
		else if (op_id == IB && is_operand_retrievable(op_id) == false){
			set_invalid_operation_flag(INPUT_BUFFER_EMPTY);
		}
	}
	return mov_pending;
}

/* Function: handle_ovatar_operand
 * -----------------------------------------------------------------------------
 * Performs the action of handling and moving the ovatar in the cases that are
 * applicable
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Void.
 */
static bool handle_oavatar_source_operand(int op_id)
{
	int mov_pending = true;

	mov_pending = move_avatar_to_operand(&g_oavatar, op_id);
	
	if (mov_pending == false && g_oavatar.in_place == false &&
								g_oavatar.op2_retrieved == false){

		g_oavatar.in_place = true;
		if (check_operand_has_valid_value(op_id)){
			g_oavatar.value = get_operand_value_box(op_id);
			g_oavatar.value.visible_box = true;
		}
		else {
			set_invalid_operation_flag(REG_VALUE_INVALID);
		}
	}
	return mov_pending;
}
/* Function: handle_source_operand
 * -----------------------------------------------------------------------------
 * Moves the corresponding avatar depending of the kind of source operand that
 * the instruction is using.
 *
 * Arguments:
 *	line: the code line that will be executed
 *
 * Return:
 *	Avatar id, the id of the avatar that is under movement
 */
static int handle_source_operand(code_line_t *line)
{
	assert(line != NULL && "The value of line cannot be NULL");
	int operand_quantity = cl_get_instruction_operand_quantity(line->ins->id);
	int mov_pending = NO_VALUE;
	int avatar_id = NOAVATAR;

	if (operand_quantity == TWO_OPERANDS && line->op2->id == IB){
		if (g_iavatar.op2_retrieved == false){
			mov_pending = handle_iavatar_source_operand(line->op2->id);	
		}
		avatar_id = IAVATAR;
	} else if (operand_quantity == TWO_OPERANDS){
		if (g_oavatar.op2_retrieved == false){
			mov_pending = handle_oavatar_source_operand(line->op2->id);	
		}
		avatar_id = OAVATAR;
	}
	if (mov_pending == false){
		bool retrieve_pending; 
		if (avatar_id == IAVATAR){
			retrieve_pending = retrieve_operand(&g_iavatar);
			if (retrieve_pending == false){
				g_iavatar.op2_retrieved = true;
				g_iavatar.in_place = false;
			}
		} else if (avatar_id == OAVATAR){
			retrieve_pending = retrieve_operand(&g_oavatar);
			if (retrieve_pending == false){
				g_oavatar.op2_retrieved = true;
				g_oavatar.in_place = false;
				if (line->op1->id == OB){
					bf_add_output_to_list();
				}
			}
		}
	}
	return avatar_id;
}
/* Function: handle_source_operand
 * -----------------------------------------------------------------------------
 * Moves the corresponding avatar depending of the kind of source operand that
 * the instruction is using.
 *
 * Arguments:
 *	line: the code line that will be executed
 *
 * Return:
 *	void.
 *
static void handle_source_operand(code_line_t *line)
{
	assert(line != NULL && "The value of line cannot be NULL");
	int operand_quantity = cl_get_instruction_operand_quantity(line->ins->id);
	int mov_pending = NO_VALUE;

	if (operand_quantity == TWO_OPERANDS && g_avatar.op2_retrieved == false){
		mov_pending = move_avatar_to_operand(&g_avatar, line->op2->id);
		if (mov_pending == false && g_avatar.in_place == false){
			g_avatar.in_place = true;
			int op_id = line->op2->id;
			if ((op_id == IB && is_operand_retrievable(op_id) == true) || 
				(op_id != IB && check_operand_has_valid_value(op_id))){	
				g_avatar.value = get_operand_value_box(line->op2->id);
				g_avatar.value.visible_box = true;
			} else if (op_id == IB && is_operand_retrievable(op_id) == false){
					set_invalid_operation_flag(INPUT_BUFFER_EMPTY);
			} else {
					set_invalid_operation_flag(REG_VALUE_INVALID);
			}
		} 
		if (mov_pending == false){
			int retrieve_pending = retrieve_operand();
			if (retrieve_pending == false){
				g_avatar.op2_retrieved = true;
				g_avatar.in_place = false;

				if (line->op1->id == OB){
					bf_add_output_to_list();
				}
			}
		}
	}
}*/

/* Function: handle_destiny_operand
 * -----------------------------------------------------------------------------
 * Arguments:
 *	line: the code line that will be executed
 *
 * Return:
 *	void.
 */
static void handle_destiny_operand(code_line_t *line, int avatar_id)
{
	assert(line != NULL && "The value of line cannot be NULL");
	int mov_pending = NO_VALUE;
	if (avatar_id == IAVATAR && g_iavatar.op2_retrieved == true){
		mov_pending = move_avatar_to_operand(&g_iavatar, line->op1->id);
		if (mov_pending == false && g_avatar.in_place == false){
			g_iavatar.in_place = true;
		}
		if (mov_pending == false){
			int deliver_pending = deliver_operand(&g_iavatar ,line->op1->id);
			if (deliver_pending == false){
				g_iavatar.op1_delivered = true;
				g_iavatar.value.visible_box = false;
			}
		}
	}
	if (avatar_id == OAVATAR && g_oavatar.op2_retrieved == true){
		mov_pending = move_avatar_to_operand(&g_oavatar, line->op1->id);
		if (mov_pending == false && g_oavatar.in_place == false){
			g_oavatar.in_place = true;
		}
		if (mov_pending == false){
			int deliver_pending = deliver_operand(&g_oavatar ,line->op1->id);
			if (deliver_pending == false){
				g_oavatar.op1_delivered = true;
				g_oavatar.value.visible_box = false;
			}
		}
	}
}

/* Function: check_code_size
 * -----------------------------------------------------------------------------
 * Verifies if the code develped by the player complies with the code size set
 * for that given level
 *
 * Arguments:
 *	void.
 *
 * Return:
 *	bool with the state if the level complies with the code size.
 */
static bool check_correct_code_size()
{
	bool within_limit = false;
	
	int code_size = cw_get_code_list_size();
	int instructions_limit = lv_get_level_instructions_limit();

	if (code_size <= instructions_limit){
		within_limit = true;
	}
	
	return within_limit;
}


/* Function: check_run_finished
 * -----------------------------------------------------------------------------
 * Verifies if the elements in the output list are the same as the output list
 * and if the input buffer still have values and if there are any values
 * pending processing in the input buffer. Checks if code complies with 
 * the limit of instructions set for the level
 *
 * Arguments:
 *	void.
 *
 * Return:
 *	void.
 */
static bool check_run_finished()
{
	bool finished = false;
	
	int input_buffer_size = get_input_buffer_list_size();
	bool win = lv_check_if_win();
	bool correct_code_size = check_correct_code_size();

	if (win == true && input_buffer_size !=0) {
		set_invalid_operation_flag(UNPROCESSED_IB_VALUES);
	} else if (win == true && correct_code_size == false) {
		set_invalid_operation_flag(EXCEEDS_CODE_LIMIT);
	} else if (win == true && input_buffer_size == 0){
		finished = true;
	} 

	return finished;	
}

/* Function: execute_instruction
 * -----------------------------------------------------------------------------
 * Arguments:
 *	The code line that will be executed
 *
 * Return:
 *	void.
 */
static void execute_instruction(code_line_t *line, int line_pos)
{
	assert(line != NULL && "The value of line cannot be NULL");
	int buffer_inputs = get_input_buffer_list_size();
	int avatar_id = NOAVATAR;	
	bool arrow_in_place = move_execution_arrow(line_pos);

	if (line->ins->id == LABEL && arrow_in_place == true){
		line->state = EXECUTED;
		reset_avatar_no_pos();
	} else if (line->ins->id == JMP && arrow_in_place == true){
		cw_operate_jump_instruction(line);
		reset_avatar_no_pos();
	} else if (buffer_inputs != CW_EMPTY || line->state == IN_EXECUTION || 
													 line->state == COMPLETE){
		line->state = IN_EXECUTION;
		avatar_id = handle_source_operand(line);
		handle_destiny_operand(line, avatar_id);
	
		if (avatar_id == IAVATAR && g_iavatar.op2_retrieved == true && 
			g_iavatar.op1_delivered == true && arrow_in_place == true){
			operate_instruction(line, g_iavatar.value);
			line->state = EXECUTED;
			reset_avatar_no_pos();
		}
		if (avatar_id == OAVATAR && g_oavatar.op2_retrieved == true && 
			g_oavatar.op1_delivered == true && arrow_in_place == true){
			operate_instruction(line, g_oavatar.value);
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
	
	if (check_run_finished() == true){
		finished = true;	
		return finished;
	}
	for (int i = 0; i < code_size; i++){
		code_line_t *line = get_code_line_at_pos(i);
		if (line->state != EXECUTED){
			execute_instruction(line, i);	
			return finished;
		}
	}
	finished = true;
	return finished;
}

