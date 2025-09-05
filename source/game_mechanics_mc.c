#include <SDL.h>
#include <assert.h>
#include "game_mechanics_mc.h"
#include "aux.h"
#include "draw_dw.h"
#include "code_window_cw.h"
#include "mouse_ms.h"
#include "dimensions_dm.h"
#include "registers_rg.h" 
#include "buffers_bf.h"
#include "dbg.h"
#include "levels_lv.h"
#include "arrow_ar.h"

#define INPUT_BUFFER_EMPTY_TEXT "ERROR: A value cannot be recovered if the "\
							"Input Buffer [IB] is empty."
#define REG_VALUE_INVALID_TEXT "ERROR:Register cannot be read if a value has "\
							"been stored first."
#define INVALID_OUTPUT_VALUE_TEXT "ERROR: Incorrect value in the output buffer"
#define UNPROCESSED_IB_VALUES_TEXT "ERROR: Output is correct but only works by"\
								" that specific set of values."
#define EXCEEDS_CODE_LIMIT_TEXT "ERROR: Correct output but exceeds code size"\
								" limit."
#define OUTPUT_BUFFER_EMPTY_TEXT "ERROR: Run finished and there are no"\
								" elements in the Output Buffer [OB]."

#define AVATAR_BUFFER_OFFSET 50
#define AVATAR_W 50
#define AVATAR_H 50
#define AVATAR_VALUE_BOX_DISTANCE 10

#define IAVATAR_X BUFFER_BOX_X //(SCREEN_WIDTH*5)/8
#define IAVATAR_Y INPUT_BUFFER_BOX_Y + BUFFER_BOX_H + AVATAR_BUFFER_OFFSET

#define OAVATAR_X BUFFER_BOX_X //(SCREEN_WIDTH*5)/8
#define OAVATAR_Y OUTPUT_BUFFER_BOX_Y - 2*AVATAR_BUFFER_OFFSET

#define RAVATAR_X_POS_OFFSET (AVATAR_W + 20)

#define RAIL_W 4
#define RAIL_END_W 16

texture_array_t *ib_empty;
texture_array_t *reg_val_bad;
texture_array_t *ob_val_bad;
texture_array_t *ib_unproc_vals;
texture_array_t *exc_code_size;
texture_array_t *ob_empty;

enum avatar_id{
	NOAVATAR,
	IAVATAR,
	OAVATAR,
	RAVATAR
};

static int g_invalid_operation_flag = NO_INVALID_OPERATION;

typedef struct avatar_t{
	SDL_Rect box;
	int id;
	bool in_place;
	bool op2_retrieved;
	bool op1_delivered;
	value_box_t value;
	SDL_Color color;
} avatar_t;

static avatar_t g_iavatar;
static avatar_t g_oavatar;
static avatar_t g_ravatar; 

texture_t *g_exec_arrow_texture;
static arrow_t g_exec_arrow;

enum counter_actions{
	ADD_1,
	SUB_1,
	RESET_COUNTER,
	SET_COUNTER,
	GET_COUNTER,
};

static void execute_instruction(code_line_t *line, int line_pos);
static bool move_avatar_to_operand(avatar_t *avatar, int op_id);
static int get_operand_x_dest(int op_id);
static int get_operand_y_dest(int op_id);
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
static bool check_operand_has_value(int op_id);
static bool check_run_finished();
static bool check_correct_code_size();
static bool handle_iavatar_source_operand(int op_id);
static bool handle_oavatar_source_operand(int op_id);
static bool handle_ravatar_source_operand(int op_id);
static bool check_avatar_has_value(avatar_t *avatar);
static void draw_iavatar();
static void draw_oavatar();
static void draw_ravatar();

/* Function: mc_init_errors_texture
 *------------------------------------------------------------------------------
 * Creates the instructions texture of the instruction box
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Void.
 */
void mc_init_errors_texture()
{
	
	ib_empty = dw_new_text_texture_by_h(RES_BOX_W, MESSAGE_TEXT_H, 
											  C_BLACK, INPUT_BUFFER_EMPTY_TEXT);
	reg_val_bad = dw_new_text_texture_by_h(RES_BOX_W, MESSAGE_TEXT_H, 
											   C_BLACK, REG_VALUE_INVALID_TEXT);
	ob_val_bad = dw_new_text_texture_by_h(RES_BOX_W, MESSAGE_TEXT_H, 
											C_BLACK, INVALID_OUTPUT_VALUE_TEXT);
	ib_unproc_vals = dw_new_text_texture_by_h(RES_BOX_W, MESSAGE_TEXT_H, 
										   C_BLACK, UNPROCESSED_IB_VALUES_TEXT);
	exc_code_size = dw_new_text_texture_by_h(RES_BOX_W, MESSAGE_TEXT_H, 
										      C_BLACK, EXCEEDS_CODE_LIMIT_TEXT);
	ob_empty = dw_new_text_texture_by_h(RES_BOX_W, MESSAGE_TEXT_H, 
										     C_BLACK, OUTPUT_BUFFER_EMPTY_TEXT);
}

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
	SDL_Rect r = {.x = RES_BOX_X, .y = RES_BOX_Y, .w = RES_BOX_W, 
				  .h = RES_BOX_H};
	dw_draw_filled_rectangle(r, C_WHITE, C_WHITE);
	texture_array_t *message;
	
	switch(id){
		case INPUT_BUFFER_EMPTY:
			message = ib_empty;
			break;
		case REG_VALUE_INVALID:
			message = reg_val_bad;
			break;
		case INVALID_OUTPUT_VALUE:
			message = ob_val_bad;
			break;
		case UNPROCESSED_IB_VALUES:
			message = ib_unproc_vals;
			break;
		case EXCEEDS_CODE_LIMIT:
			message = exc_code_size;
			break;
		case OUTPUT_BUFFER_EMPTY:
			message = ob_empty;
			break;
		default: 
			puts("ERROR: Invalid operation incorrec id");
	}
	
	SDL_Rect b = {.x = RES_BOX_X, .y = RES_BOX_Y, .w =RES_BOX_W, 
													 .h = MESSAGE_TEXT_TOTAL_H};
	dw_draw_wrapped_texture_by_h(b, MESSAGE_TEXT_H, message);
	
	static bool button_created = false;
	static button_t *ret;
	bool button_pressed = false;

	if (button_created == false){
		button_created = true;
		texture_t *ret_texture = dw_create_text_texture(STR_BACK, 
																   C_WHITE);
		check_mem(ret_texture);
		int x = RES_BOX_X + RES_BOX_W/2 - BACK_BUTTON_W/2;
		int y = RES_BOX_Y + RES_BOX_H - BACK_CONT_BUTTON_H - 
													 RES_BOX_TEXT_BORDER_OFFSET;
		SDL_Rect r = {.x = x, .y = y, .w = BACK_BUTTON_W, BACK_CONT_BUTTON_H};
		ret = bt_create_button(r, true, true, true, C_BLACK, C_WHITE,
							   ret_texture);
		check_mem(ret);
	} 
	bt_draw_button(ret);

		if (bt_check_mouse_click_button(ret) == true){
			reset_level = true;
			button_pressed = true;
		} 
		if (button_pressed == true){
			bt_destroy_button(ret);
			button_created = false;
		}

	error:
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
		
	g_iavatar.id = IAVATAR;
	g_iavatar.box.x = bf_get_buffer_value_box_x_coord_by_id(IB);	
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

	g_iavatar.color = C_MAGENTA;

	g_oavatar.id = OAVATAR;
	g_oavatar.box.x = bf_get_buffer_value_box_x_coord_by_id(OB);	
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

	g_oavatar.color = C_CYAN;

	g_ravatar.id = RAVATAR;
	g_ravatar.box.x = rg_get_register_box_member(MEMBER_X) - 3*REG_BOX_X_OFFSET 
					  + rg_get_register_box_member(MEMBER_W) - AVATAR_W;
	g_ravatar.box.y = rg_get_register_box_member(MEMBER_Y) + 
					  rg_get_register_box_member(MEMBER_H)/2 - AVATAR_H/2; 
	g_ravatar.box.w = AVATAR_W;
	g_ravatar.box.h = AVATAR_H;

	g_ravatar.value.visible_box = false;
	g_ravatar.in_place = false;
	g_ravatar.op2_retrieved = false;
	g_ravatar.op1_delivered = false;

	g_ravatar.value.value = NO_VALUE;
	g_ravatar.value.box.x = g_ravatar.box.x;
	g_ravatar.value.box.y = g_ravatar.box.y - AVATAR_H;
	g_ravatar.value.box.w = VALUE_BOX_W;
	g_ravatar.value.box.h = VALUE_BOX_H;
	
	g_ravatar.color = C_YELLOW;

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

	g_ravatar.value.visible_box = false;
	g_ravatar.in_place = false;
	g_ravatar.op2_retrieved = false;
	g_ravatar.op1_delivered = false;

	g_ravatar.value.value = NO_VALUE;
	g_ravatar.value.box.x = g_ravatar.box.x;
	g_ravatar.value.box.y = g_ravatar.box.y - AVATAR_H;
}
/* Function: draw_iavatar
 * ----------------------------------------------------------------------------
 * Draws the iavatar and its corresponding rail
 *
 * Arguments:
 * 	Void.
 *
 * Return:
 *	Void.
 */
static void draw_iavatar()
{
	if (g_iavatar.value.visible_box == true){
		ax_draw_value_box(&g_iavatar.value, g_iavatar.color);
	}
	SDL_Rect r0 = {.x = g_iavatar.box.x, .y = g_iavatar.box.y, 
				  .w= g_iavatar.box.w, .h = g_iavatar.box.h};
	dw_draw_filled_rectangle(r0, g_iavatar.color, g_iavatar.color);

	int bf_y_lower = bf_get_input_buffer_box_member(MEMBER_Y) + 
					 bf_get_input_buffer_box_member(MEMBER_H);
	int medium = (rg_get_register_box_member(MEMBER_Y) - bf_y_lower)/2;

	int x = REG_VBOX_X + VALUE_BOX_W/2 - RAIL_W/2;
	int y = rg_get_register_box_member(MEMBER_Y) - medium - RAIL_W/2;
	int w = bf_get_input_buffer_box_member(MEMBER_X) + VALUE_BOX_W/2 - x + 
										BUFFER_VALUE_OFFSET_X + RAIL_W/2;
	int h = RAIL_W;

	SDL_Rect r1 = {.x = x, .y = y, .w = w, .h = h};
	dw_draw_filled_rectangle(r1, C_MAGENTA, C_MAGENTA);
	
	int rail_end_x = x - (RAIL_END_W - RAIL_W)/2;
	int rail_end_y = y - (RAIL_END_W - RAIL_W)/2;
	SDL_Rect r2 = {.x = rail_end_x, .y = rail_end_y, .w = RAIL_END_W, 
				   .h = RAIL_END_W};
	dw_draw_filled_rectangle(r2 , C_MAGENTA, C_MAGENTA);

	rail_end_x = x + w - (RAIL_END_W - RAIL_W) + RAIL_W/2;

	SDL_Rect r3 = {.x = rail_end_x, .y = rail_end_y, .w = RAIL_END_W, 
				   .h = RAIL_END_W};
	dw_draw_filled_rectangle(r3, C_MAGENTA, C_MAGENTA);

	int xc = g_iavatar.box.x + g_iavatar.box.w/2 - RAIL_W/2; 
	int yc = g_iavatar.box.y + g_iavatar.box.h/2;
	int wf = RAIL_W;
	int yf;
	int hf;
	if (yc > y){
	 	yf = y;
		hf = yc - y + RAIL_W;
	} else {
		yf = yc;
		hf = y - yc + RAIL_W;
	}
	SDL_Rect r4 = {.x = xc, .y = yf, .w = wf, .h = hf};
	dw_draw_filled_rectangle(r4, C_MAGENTA, C_MAGENTA);
}

/* Function: draw_oavatar
 * ----------------------------------------------------------------------------
 * Draws the iavatar and its corresponding rail
 *
 * Arguments:
 * 	Void.
 *
 * Return:
 *	Void.
 */
static void draw_oavatar()
{
	if (g_oavatar.value.visible_box == true){
		ax_draw_value_box(&g_oavatar.value, g_oavatar.color);
	}
	SDL_Rect r0 = {.x = g_oavatar.box.x, .y = g_oavatar.box.y, 
				  .w = g_oavatar.box.w, .h = g_oavatar.box.h};
	dw_draw_filled_rectangle(r0, g_oavatar.color, g_oavatar.color);

	int bf_y_upper = bf_get_output_buffer_box_member(MEMBER_Y);
	int medium = (bf_y_upper - (rg_get_register_box_member(MEMBER_Y) +
					                   rg_get_register_box_member(MEMBER_H)))/2;

	int x = REG_VBOX_X + VALUE_BOX_W/2 - RAIL_W/2;
	int y = bf_get_output_buffer_box_member(MEMBER_Y) - medium - 
																RAIL_W/2;
	
	int w = bf_get_input_buffer_box_member(MEMBER_X) + VALUE_BOX_W/2 - x + 
											   BUFFER_VALUE_OFFSET_X + RAIL_W/2;
	int h = RAIL_W;
	
	SDL_Rect r1 = {.x = x, .y = y, .w = w, .h = h};
	dw_draw_filled_rectangle(r1, C_CYAN, C_CYAN);

	int rail_end_x = x - (RAIL_END_W - RAIL_W)/2;
	int rail_end_y = y - (RAIL_END_W - RAIL_W)/2;
	
	SDL_Rect r2 = {.x = rail_end_x, .y = rail_end_y, .w = RAIL_END_W, 
				   .h = RAIL_END_W};
	dw_draw_filled_rectangle(r2, C_CYAN, C_CYAN);

	rail_end_x = x + w - (RAIL_END_W - RAIL_W) + RAIL_W/2;
	SDL_Rect r3 = {.x = rail_end_x, .y = rail_end_y, .w = RAIL_END_W, 
				   .h = RAIL_END_W};
	dw_draw_filled_rectangle(r3, C_CYAN, C_CYAN);

	int xc = g_oavatar.box.x + g_oavatar.box.w/2 - RAIL_W/2; 
	int yc = g_oavatar.box.y + g_oavatar.box.h/2;
	int wf = RAIL_W;
	int yf;
	int hf;
	if (yc > y){
	 	yf = y;
		hf = yc - y + RAIL_W;
	} else {
		yf = yc;
		hf = y - yc + RAIL_W;
	}
	SDL_Rect r4 = {.x = xc, .y = yf, .w = wf, .h = hf};
	dw_draw_filled_rectangle(r4, C_CYAN, C_CYAN);
}

/* Function: draw_ravatar
 * ----------------------------------------------------------------------------
 * Draws the iavatar and its corresponding rail
 *
 * Arguments:
 * 	Void.
 *
 * Return:
 *	Void.
 */
void draw_ravatar()
{
	
	if (g_ravatar.value.visible_box == true){
		ax_draw_value_box(&g_ravatar.value, g_ravatar.color);
	}
	SDL_Rect r0 = {.x = g_ravatar.box.x, .y =  g_ravatar.box.y, 
				   .w = g_ravatar.box.w, .h = g_ravatar.box.h};
	dw_draw_filled_rectangle(r0, g_ravatar.color, g_ravatar.color);

	int rx = rg_get_ibox_x() + VALUE_BOX_W/2;
	int ry = rg_get_register_box_member(MEMBER_Y) + REG_BOX_OFFSET;
	int rw = RAIL_W;
	int rh = rg_get_register_box_member(MEMBER_H) - 2*REG_BOX_OFFSET;
	
	SDL_Rect r1 = {.x = rx, .y = ry, .w = rw, .h = rh};
	dw_draw_filled_rectangle(r1, C_YELLOW, C_YELLOW);
	
	int xc = g_ravatar.box.x + g_ravatar.box.w/2; 
	int yc = g_ravatar.box.y + g_ravatar.box.h/2 - RAIL_W/2;
	int hf = RAIL_W;
	int xf;
	int wf;
	if (xc < rx){
	 	xf = xc;
		wf = rx - xf + RAIL_W;
	} else {
		xf = rx;
		wf = xc - xf + RAIL_W;
	}
	SDL_Rect r2 = {.x = xf, .y = yc, .w = wf, .h = hf};
	dw_draw_filled_rectangle(r2, C_YELLOW, C_YELLOW);

	int rail_end_x = rx - (RAIL_END_W - RAIL_W)/2;
	int rail_end_y = ry - (RAIL_END_W - RAIL_W)/2;

	SDL_Rect r3 = {.x = rail_end_x, .y = rail_end_y, .w = RAIL_END_W, 
				   .h = RAIL_END_W};
	dw_draw_filled_rectangle(r3, C_YELLOW, C_YELLOW);

	rail_end_y = ry + rh - (RAIL_END_W - RAIL_W) + RAIL_W/2;
	SDL_Rect r4 = {.x = rail_end_x, .y = rail_end_y, .w = RAIL_END_W, 
				   .h = RAIL_END_W};
	dw_draw_filled_rectangle(r4, C_YELLOW, C_YELLOW);
}

/* Function: mc_draw_avatar
 * ----------------------------------------------------------------------------
 * Draws all avatars of the screen
 *
 * Arguments:
 * 	Void.
 *
 * Return:
 *	Void.
 */
void mc_draw_avatar()
{
	draw_iavatar();
	draw_oavatar();
	draw_ravatar();
	
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
static int get_operand_y_dest(int op_id)
{
	int y;
	if (op_id > REGISTERS_MIN && op_id < REGISTERS_MAX){
		y = rg_get_register_value_box_y_coord_by_id(op_id);
	} else if (op_id > BUFFERS_MIN && op_id < BUFFERS_MAX){
		y = bf_get_buffer_value_box_y_coord_by_id(op_id);	
	} else if (op_id == IBOX){
		y = rg_get_ibox_y();
	} else if (op_id == OBOX){
		y = rg_get_obox_y();
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
static int get_operand_x_dest(int op_id)
{
	int x;
	if (op_id > REGISTERS_MIN && op_id < REGISTERS_MAX){
		x = rg_get_register_value_box_x_coord_by_id(op_id);
	} else if (op_id > BUFFERS_MIN && op_id < BUFFERS_MAX){
		x = bf_get_buffer_value_box_x_coord_by_id(op_id);	
	} else if (op_id == IBOX || op_id == OBOX){
		x = rg_get_ibox_x();
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

	g_exec_arrow.texture = g_exec_arrow_texture;

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
		dw_draw_texture_fits_height(g_exec_arrow.box, g_exec_arrow.texture);
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
	assert(op_id > REGISTERS_MIN && op_id < RGBOX_MAX &&
		   "Invalid operand");

	bool mov = false;
	int x;
	int y;

	if (op_id > REGISTERS_MIN && op_id < REGISTERS_MAX){
		x = get_operand_x_dest(op_id) + RAVATAR_X_POS_OFFSET;
	} else {
		x = get_operand_x_dest(op_id);
	}

	if (op_id > REGISTERS_MIN && op_id < REGISTERS_MAX){
		y = get_operand_y_dest(op_id);
	} else if (op_id == IB){
		y = get_operand_y_dest(op_id) + 1.5*VALUE_BOX_H;
	} else if (op_id == OB){
		y = get_operand_y_dest(op_id) - 2*VALUE_BOX_H - AVATAR_H;
	} else if (op_id == IBOX){
		if (avatar->id == RAVATAR){
			y = get_operand_y_dest(op_id) + REG_VBOX_OFFSET;
		} else{
			y = get_operand_y_dest(op_id) - 2.5*VALUE_BOX_H - REG_VBOX_OFFSET;
		}
	} else if (op_id == OBOX){
		if (avatar->id == RAVATAR){
			y = get_operand_y_dest(op_id) - 2.5*VALUE_BOX_H - 2*REG_VBOX_OFFSET;
		} else if (avatar->id == OAVATAR){
			y = get_operand_y_dest(op_id) + REG_VBOX_OFFSET;
		}

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
 *	op_id: the id of the operand the avatar will retrieve, does not contains
 *	texture
 *
 * Return:
 *	bool indicating if part of the retriving is pending
 */
static value_box_t get_operand_value_box(int op_id)
{
	assert((op_id > REGISTERS_MIN && op_id < REGISTERS_MAX) ||
		   (op_id > BUFFERS_MIN &&  op_id < BUFFERS_MAX) ||
		   (op_id > RGBOX_MIN &&  op_id < RGBOX_MAX) && 
		   "The operand id is invalid");

	value_box_t op_value_box;

	if (op_id > REGISTERS_MIN && op_id < REGISTERS_MAX){
		op_value_box = rg_get_register_value_box_by_id(op_id);	
	} else if (op_id == IB){
		op_value_box = bf_get_input_buffer_value_box();
	} else if (op_id == OB){
		op_value_box = bf_get_output_buffer_value_box();
	} else if (op_id == IBOX){
		op_value_box = rg_get_ibox_value_box();
	} else if (op_id == OBOX){
		op_value_box = rg_get_obox_value_box();
	}
	return op_value_box;
}

/* Function: check_avatar_has_value
 * -----------------------------------------------------------------------------
 * Determines if an avatar is holding a value
 * 
 * Arguments:
 *	avatar: the id of the avatar that will be checked
 *
 * Return:
 *	bool indicating if the avatar has a value
 */
static bool check_avatar_has_value(avatar_t *avatar)
{
	assert(avatar != NULL && "Avatar pointer is NULL");
	
	bool valid_value = true;
	if (avatar->value.value == NO_VALUE){
		valid_value = false;
	}
	return valid_value;
}

/* Function: check_operand_has_value
 * -----------------------------------------------------------------------------
 * Checks if an operand does not has a valid value
 * 
 *	Arguments:
 *	op_id: the id of the operand the avatar will retrieve
 *
 * Return:
 *	bool indicating if part of the retriving is pending
 */
static bool check_operand_has_value(int op_id)
{
	assert((op_id > REGISTERS_MIN && op_id < REGISTERS_MAX) ||
		   (op_id > BUFFERS_MIN &&  op_id < BUFFERS_MAX) ||
		   (op_id > RGBOX_MIN &&  op_id < RGBOX_MAX) && 
		   "The operand id is invalid");

	value_box_t op_value_box;

	if (op_id > REGISTERS_MIN && op_id < REGISTERS_MAX){
		op_value_box = rg_get_register_value_box_by_id(op_id);	
	} else if (op_id == IB){
		op_value_box = bf_get_input_buffer_value_box();
	} else if (op_id == OB){
		op_value_box = bf_get_output_buffer_value_box();
	} else if (op_id == IBOX){
		op_value_box = rg_get_ibox_value_box();
	} else if (op_id == OBOX){
		op_value_box = rg_get_obox_value_box();
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
	assert(((op_id > REGISTERS_MIN && op_id < REGISTERS_MAX) ||
	       (op_id > BUFFERS_MIN && op_id < BUFFERS_MAX) ||
		   (op_id > RGBOX_MIN && op_id < RGBOX_MAX)) && 
		   "The operand id is invalid");
	
	bool operation_valid = true;

	if (op_id > REGISTERS_MIN && op_id < REGISTERS_MAX){
		rg_set_register_value_box(op_id, val);	
	} else if (op_id == OB){
		bf_set_output_buffer_value_box(val);	
	} else if (op_id == IBOX){
		rg_set_ibox_value_box(val);	
	} else if (op_id == OBOX){
		rg_set_obox_value_box(val);	
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
		value_box_t cur_val = rg_get_register_value_box_by_id(op_id);
		if (cur_val.value == NO_VALUE){
			operation_valid = false;	
		} else {
			cur_val.value += val.value;
			rg_set_register_value_box(op_id, cur_val);	
		}
	} else if (op_id == OB){
		bf_set_output_buffer_value_box(val);	
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
			char *number = ax_number_to_string(g_iavatar.value.value);
			g_iavatar.value.t = dw_create_text_texture(number, C_WHITE);
			free(number);
			g_iavatar.value.visible_box = true;
		}
		else if (op_id == IB && is_operand_retrievable(op_id) == false){
			set_invalid_operation_flag(INPUT_BUFFER_EMPTY);
		}
	}
	return mov_pending;
}

/* Function: handle_rvatar_operand
 * -----------------------------------------------------------------------------
 * Performs the action of handling and moving the rvatar in the cases that are
 * applicable
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Void.
 */
static bool handle_ravatar_source_operand(int op_id)
{
	bool mov_pending = true;

	mov_pending = move_avatar_to_operand(&g_ravatar, op_id);
	if (mov_pending == false && g_ravatar.in_place == false && 
							  				 g_ravatar.op2_retrieved == false){
		g_ravatar.in_place = true;
		if (check_operand_has_value(op_id) == true){
			g_ravatar.value = get_operand_value_box(op_id);
			g_ravatar.value.visible_box = true;
			rg_reset_ibox();
		}
		else {
			set_invalid_operation_flag(REG_VALUE_INVALID);
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
		if (check_operand_has_value(op_id) == true){
			g_oavatar.value = get_operand_value_box(op_id);
			g_oavatar.value.visible_box = true;
			rg_reset_obox();
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

	if(operand_quantity == TWO_OPERANDS){
		if (check_operand_has_value(IBOX) == true){
			mov_pending = handle_ravatar_source_operand(IBOX);	
			avatar_id = RAVATAR;
		} else if (check_avatar_has_value(&g_ravatar) == true){
			mov_pending = false;
			avatar_id = RAVATAR;
		} else if (cl_check_op_is_register(line->op2->id)&& 
				   check_operand_has_value(OBOX) == false &&
				   check_avatar_has_value(&g_oavatar) == false){
			if (g_ravatar.op2_retrieved == false) {
				mov_pending = handle_ravatar_source_operand(line->op2->id);	
			}
			avatar_id = RAVATAR;
		} else if (line->op2->id == IB){
			if (g_iavatar.op2_retrieved == false){
				mov_pending = handle_iavatar_source_operand(line->op2->id);	
			}
			avatar_id = IAVATAR;
		} else if (check_operand_has_value(OBOX) == true){
			mov_pending = handle_oavatar_source_operand(OBOX);	
			avatar_id = OAVATAR;
		} else if (check_avatar_has_value(&g_oavatar) == true){
			mov_pending = false;
			avatar_id = OAVATAR;
		}

	}
	if (mov_pending == false){
		bool retrieve_pending; 
		if (avatar_id == RAVATAR && g_ravatar.op2_retrieved == false){
			retrieve_pending = retrieve_operand(&g_ravatar);
			if (retrieve_pending == false){
				g_ravatar.op2_retrieved = true;
				g_ravatar.in_place = false;
			}
		} else if (avatar_id == IAVATAR){
			retrieve_pending = retrieve_operand(&g_iavatar);
			if (retrieve_pending == false){
				g_iavatar.op2_retrieved = true;
				g_iavatar.in_place = false;
			}
		} else if (avatar_id == OAVATAR && g_oavatar.op2_retrieved == false){
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
	
	if (avatar_id == RAVATAR && g_ravatar.op2_retrieved == true){
		if (cl_check_op_is_register(line->op1->id) == true){
			mov_pending = move_avatar_to_operand(&g_ravatar, line->op1->id);
		} else {
			mov_pending = move_avatar_to_operand(&g_ravatar, OBOX);
		}
		if (mov_pending == false && g_ravatar.in_place == false){
			g_ravatar.in_place = true;
		}
		if (mov_pending == false){
			int deliver_pending;
			if (cl_check_op_is_register(line->op1->id) == true){
				deliver_pending = deliver_operand(&g_ravatar, line->op1->id);
			} else {
				deliver_pending = deliver_operand(&g_ravatar, OBOX);
			}
			if (deliver_pending == false){
				g_ravatar.op1_delivered = true;
				g_ravatar.value.visible_box = false;
			}
		}
	}
	if (avatar_id == IAVATAR && g_iavatar.op2_retrieved == true){
		mov_pending = move_avatar_to_operand(&g_iavatar, IBOX);
		if (mov_pending == false && g_iavatar.in_place == false){
			g_iavatar.in_place = true;
		}
		if (mov_pending == false){
			int deliver_pending = deliver_operand(&g_iavatar, IBOX);
			if (deliver_pending == false){
				g_iavatar.op1_delivered = true;
				g_iavatar.value.visible_box = false;
				dw_free_texture(g_iavatar.value.t);
				g_iavatar.value.t = NULL;
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
		
		if (avatar_id == RAVATAR && g_ravatar.op2_retrieved == true && 
			g_ravatar.op1_delivered == true && arrow_in_place == true){
			if (cl_check_op_is_register(line->op1->id) == true){
				operate_instruction(line, g_ravatar.value);
				line->state = EXECUTED;
			} else{
				set_operand_value_box(OBOX, g_ravatar.value);
			}
			reset_avatar_no_pos();
		}
		if (avatar_id == IAVATAR && g_iavatar.op2_retrieved == true && 
			g_iavatar.op1_delivered == true && arrow_in_place == true){
			set_operand_value_box(IBOX, g_iavatar.value);
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
		code_line_t *line = cw_get_code_line_at_pos(i);
		if (line->state != EXECUTED){
			execute_instruction(line, i);	
			return finished;
		}
	}
	finished = true;
	int output_buffer_size = get_input_buffer_list_size();
	if (output_buffer_size == 0){
		set_invalid_operation_flag(OUTPUT_BUFFER_EMPTY);
	}
	return finished;
}

