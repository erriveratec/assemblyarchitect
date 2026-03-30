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
#include "immediates_im.h"
#include "stage_buttons_sb.h"

#define INPUT_BUFFER_EMPTY_TEXT "A value cannot be recovered if the "\
"Input Buffer [IB] is empty"
#define REG_VALUE_INVALID_TEXT "Register cannot be read if a value "\
"hasn't been stored first"
#define FLAG_VALUE_INVALID_TEXT "FLAG cannot be read if a value "\
"hasn't been stored first"

#define INVALID_OUTPUT_VALUE_TEXT "Incorrect value in the output buffer"
#define UNPROCESSED_IB_VALUES_TEXT "Output is correct but only works by"\
" that specific set of values"
#define EXCEEDS_CODE_LIMIT_TEXT "Correct output but exceeds code size"\
" limit"
#define OUTPUT_BUFFER_INCOMPLETE_TEXT "Not enough"\
"items in the Output Buffer [ob] after run"

static char *SYSTEM_ERROR_TEXT = "SYSTEM ERROR";

texture_array_t *ib_empty;
texture_array_t *reg_val_bad;
texture_array_t *flag_val_bad;
texture_array_t *ob_val_bad;
texture_array_t *ib_unproc_vals;
texture_array_t *exc_code_size;
texture_array_t *ob_incomplete;

texture_t *g_system_error;

static bool run_ended;
static bool step_ended;
static bool rst_lvl = false;

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
	bool op1_retrieved;
	bool op_delivered;
	bool flag1;
	bool valop;
	value_box_t mainval;
	value_box_t secval;
	SDL_Color color;
} avatar_t;

static avatar_t g_iavatar;
static avatar_t g_oavatar;
static avatar_t g_ravatar; 

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
static bool check_operand_has_value(int op_id);
static bool check_finishes_at_OB_correct_size();
static bool check_correct_code_size();
static bool handle_iavatar_source_operand(int op_id);
static bool handle_oavatar_source_operand(int op_id);
static bool handle_ravatar_source_operand(int op_id);
static bool check_avatar_has_value(avatar_t *avatar);
static void draw_iavatar();
static void draw_oavatar();
static void draw_ravatar();
bool cmp_substract(int op_id, value_box_t val);
static bool handle_ravatar_cmp(int op_id);
static void rflag_generator(avatar_t *avatar, int id);

/* Function: mc_set_rst_lvl
 *------------------------------------------------------------------------------
 * Sets the variable to reset the level
 *
 * Arguments:
 *	state: The state to which the variable will be set.
 *
 * Return:
 *	Void.
 */
void mc_set_rst_lvl(bool state)
{
	rst_lvl = state;
}

/* Function: mc_get_rst_lvl
 *------------------------------------------------------------------------------
 * Sets the variable to reset the level
 *
 * Arguments:
 *	state: The state to which the variable will be set.
 *
 * Return:
 *	Void.
 */
bool mc_get_rst_lvl()
{
	return rst_lvl;
}

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
	int text_h = dm_get_h_error_msg();		
	SDL_Rect rb = dw_get_iface_content_box(dw_get_iface_big_lower_box());

	ib_empty = dw_create_text_tex_array_by_h(rb.w, 
										text_h, 
										C_WHITE, 
										INPUT_BUFFER_EMPTY_TEXT);
	reg_val_bad = dw_create_text_tex_array_by_h(rb.w, 
										   text_h, 
										   C_WHITE, 
										   REG_VALUE_INVALID_TEXT);
	flag_val_bad = dw_create_text_tex_array_by_h(rb.w, 
										    text_h, 
										    C_WHITE, 
											FLAG_VALUE_INVALID_TEXT);
	ob_val_bad = dw_create_text_tex_array_by_h(rb.w, 
										  text_h, 
										  C_WHITE, 
										  INVALID_OUTPUT_VALUE_TEXT);
	ib_unproc_vals = dw_create_text_tex_array_by_h(rb.w, 
											  text_h,
											  C_WHITE, 
											  UNPROCESSED_IB_VALUES_TEXT);
	exc_code_size = dw_create_text_tex_array_by_h(rb.w, 
											 text_h, 
											 C_WHITE, 
											 EXCEEDS_CODE_LIMIT_TEXT);
	ob_incomplete = dw_create_text_tex_array_by_h(rb.w, 
											 text_h, 
											 C_WHITE, 
										     OUTPUT_BUFFER_INCOMPLETE_TEXT);
	g_system_error = dw_create_text_tex(SYSTEM_ERROR_TEXT, C_GREY);
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
void mc_display_invalid_operation_handler(int id)
{
	assert(id >= NO_INVALID_OPERATION && id < INVALID_OPERATION_MAX &&
		   "Incorrect id for the invalid operation handler");

	
	if (id != NO_INVALID_OPERATION){
		SDL_Rect r = dw_get_iface_big_lower_box();
		dw_draw_iface_box(r, g_system_error);
		texture_array_t *message;
		
		switch(id){
			case INPUT_BUFFER_EMPTY:
				message = ib_empty;
				break;
			case REG_VALUE_INVALID:
				message = reg_val_bad;
				break;
			case FLAG_VALUE_INVALID:
				message = flag_val_bad;
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
			case OUTPUT_BUFFER_INCOMPLETE:
				message = ob_incomplete;
				break;
			default: 
				puts("ERROR: Invalid operation incorrec id");
		}
		
		SDL_Rect b = dw_get_iface_content_box(dw_get_iface_big_lower_box());
		int text_h = dm_get_h_error_msg();		
		dw_draw_wrapped_texture_by_h(b, text_h, message);
		
		static bool button_created = false;
		static iface_btn_t *ret;
		bool button_pressed = false;

		if (button_created == false){
			button_created = true;
			texture_t *ret_texture = dw_create_text_tex(STR_BACK, C_WHITE);
			check_mem(ret_texture);
			
			SDL_Rect r = dm_get_text_box_result_but3();		
			ret = bt_create_iface_btn(r, ret_texture, true);
			check_mem(ret);
		} 
		bt_draw_iface_btn(ret, sb_get_escape_state(), NULL);

			if (bt_chk_rel_iface_btn(ret, NULL) == true){
				mc_set_rst_lvl(true);
				button_pressed = true;
			} 
			if (button_pressed == true){
				bt_destroy_iface_btn(ret);
				button_created = false;
			}

	} else {
		mc_set_rst_lvl(false);
	}
	error:
	return;
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

/* Function: mc_destroy_avatar_textures
 * -------------------------------------
 * Destroys the textures of the value boxes of the avatar.
 *
 * Arguments:
 * 	void.
 *
 * Return:
 *	void.
 */
void mc_destroy_avatar_textures()
{
	dw_free_texture(g_iavatar.mainval.t);
	dw_free_texture(g_iavatar.secval.t);
	dw_free_texture(g_oavatar.mainval.t);
	dw_free_texture(g_oavatar.secval.t);
	dw_free_texture(g_ravatar.mainval.t);
	dw_free_texture(g_ravatar.secval.t);
}

/* Function: mc_init_avatar
 * -------------------------------------
 * Arguments:
 * 	void.
 *
 * Return:
 *	void.
 */
void mc_init_avatar()
{
	SDL_Rect avatar =  dm_get_avatar_wh();
	SDL_Rect ib = dm_get_stage_input_buffer_box();
	SDL_Rect vb = dm_get_value_box_wh();
	g_iavatar.id = IAVATAR;
	g_iavatar.box.x = bf_get_buffer_value_box_x_coord_by_id(IB);	
	g_iavatar.box.y = ib.y + ib.h + avatar.h;
	g_iavatar.box.w = avatar.w;
	g_iavatar.box.h = avatar.h;

	g_iavatar.mainval.visible_box = false;
	g_iavatar.secval.visible_box = false;
	g_iavatar.in_place = false;
	g_iavatar.op2_retrieved = false;
	g_iavatar.op1_retrieved = false;
	g_iavatar.op_delivered = false;
	g_iavatar.flag1 = false;
	g_iavatar.valop = false;

	g_iavatar.mainval.value = NO_VALUE;
	g_iavatar.secval.value = NO_VALUE;
	g_iavatar.mainval.box.x = ib.x;
	g_iavatar.mainval.box.y = g_iavatar.box.y - avatar.h;
	g_iavatar.mainval.box.w = vb.w;
	g_iavatar.mainval.box.h = vb.h;
	g_iavatar.secval.box.x = ib.x;
	g_iavatar.secval.box.y = g_iavatar.box.y - avatar.h;
	g_iavatar.secval.box.w = vb.w;
	g_iavatar.secval.box.h = vb.h;

	g_iavatar.mainval.t = dw_create_text_tex(ax_char_dash, C_WHITE);
	g_iavatar.secval.t = dw_create_text_tex(ax_char_dash, C_WHITE);
	g_iavatar.color = C_MAGENTA;

	SDL_Rect ob = dm_get_stage_output_buffer_box();
	g_oavatar.id = OAVATAR;
	g_oavatar.box.x = bf_get_buffer_value_box_x_coord_by_id(OB);	
	g_oavatar.box.y = ob.y - 2*avatar.h;
	g_oavatar.box.w = avatar.w;
	g_oavatar.box.h = avatar.h;

	g_oavatar.mainval.visible_box = false;
	g_oavatar.secval.visible_box = false;
	g_oavatar.in_place = false;
	g_oavatar.op2_retrieved = false;
	g_oavatar.op1_retrieved = false;
	g_oavatar.op_delivered = false;
	g_oavatar.flag1 = false;
	g_oavatar.valop = false;

	g_oavatar.mainval.value = NO_VALUE;
	g_oavatar.secval.value = NO_VALUE;
	g_oavatar.mainval.box.x = ob.x;
	g_oavatar.mainval.box.y = g_oavatar.box.y - avatar.h;
	g_oavatar.mainval.box.w = vb.w;
	g_oavatar.mainval.box.h = vb.h;
	g_oavatar.mainval.t = dw_create_text_tex(ax_char_dash, C_WHITE);
	g_oavatar.secval.box.x = ob.x;
	g_oavatar.secval.box.y = g_oavatar.box.y - avatar.h;
	g_oavatar.secval.box.w = vb.w;
	g_oavatar.secval.box.h = vb.h;
	g_oavatar.secval.t = dw_create_text_tex(ax_char_dash, C_WHITE);
	g_oavatar.color = C_CYAN;

	int ofs = dm_get_ofs_stage_reg_box();
	SDL_Rect rb = rg_get_register_box();
	
	g_ravatar.id = RAVATAR;
	g_ravatar.box.x = rb.x + rb.w - 2*avatar.w;
	g_ravatar.box.y = rb.y + rb.h/2 - avatar.h/2; 
	g_ravatar.box.w = avatar.w;
	g_ravatar.box.h = avatar.h;

	g_ravatar.mainval.visible_box = false;
	g_ravatar.secval.visible_box = false;
	g_ravatar.in_place = false;
	g_ravatar.op2_retrieved = false;
	g_ravatar.op1_retrieved = false;
	g_ravatar.op_delivered = false;
	g_ravatar.flag1 = false;
	g_ravatar.valop = false;

	g_ravatar.mainval.value = NO_VALUE;
	g_ravatar.mainval.box.x = g_ravatar.box.x;
	g_ravatar.mainval.box.y = g_ravatar.box.y - avatar.h;
	g_ravatar.mainval.box.w = vb.w;
	g_ravatar.mainval.box.h = vb.h;
	g_ravatar.mainval.t = dw_create_text_tex(ax_char_dash, C_WHITE);
	g_ravatar.secval.value = NO_VALUE;
	g_ravatar.secval.box.x = g_ravatar.box.x;
	g_ravatar.secval.box.y = g_ravatar.box.y - avatar.h;
	g_ravatar.secval.box.w = vb.w;
	g_ravatar.secval.box.h = vb.h;
	g_ravatar.secval.t = dw_create_text_tex(ax_char_dash, C_WHITE);

	g_ravatar.color = C_YELLOW;
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
	dw_free_texture(g_iavatar.mainval.t);
	g_iavatar.mainval.t = NULL;
	dw_free_texture(g_iavatar.secval.t);
	g_iavatar.secval.t = NULL;
	
	dw_free_texture(g_oavatar.mainval.t);
	g_oavatar.mainval.t = NULL;
	dw_free_texture(g_oavatar.secval.t);
	g_oavatar.secval.t = NULL;

	dw_free_texture(g_ravatar.mainval.t);
	g_ravatar.mainval.t = NULL;
	dw_free_texture(g_ravatar.secval.t);
	g_ravatar.secval.t = NULL;

	mc_init_avatar();	
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
	SDL_Rect avatar =  dm_get_avatar_wh();
	g_iavatar.mainval.visible_box = false;
	g_iavatar.secval.visible_box = false;
	g_iavatar.in_place = false;
	g_iavatar.op2_retrieved = false;
	g_iavatar.op1_retrieved = false;
	g_iavatar.op_delivered = false;
	g_iavatar.flag1 = false;
	g_iavatar.valop = false;

	g_iavatar.mainval.value = NO_VALUE;
	g_iavatar.mainval.box.x = g_iavatar.box.x;
	g_iavatar.mainval.box.y = g_iavatar.box.y - avatar.h;
	dw_free_texture(g_iavatar.mainval.t);
	g_iavatar.mainval.t = NULL;
	g_iavatar.mainval.t = dw_create_text_tex(ax_char_dash, C_WHITE);
	
	g_iavatar.secval.value = NO_VALUE;
	g_iavatar.secval.box.x = g_iavatar.box.x;
	g_iavatar.secval.box.y = g_iavatar.box.y - avatar.h;
	dw_free_texture(g_iavatar.secval.t);
	g_iavatar.secval.t = NULL;
	g_iavatar.secval.t = dw_create_text_tex(ax_char_dash, C_WHITE);


	g_oavatar.mainval.visible_box = false;
	g_oavatar.secval.visible_box = false;
	g_oavatar.in_place = false;
	g_oavatar.op2_retrieved = false;
	g_oavatar.op1_retrieved = false;
	g_oavatar.op_delivered = false;
	g_oavatar.flag1 = false;
	g_oavatar.valop = false;

	g_oavatar.mainval.value = NO_VALUE;
	g_oavatar.mainval.box.x = g_oavatar.box.x;
	g_oavatar.mainval.box.y = g_oavatar.box.y - avatar.h;
	dw_free_texture(g_oavatar.mainval.t);
	g_oavatar.mainval.t = NULL;
	g_oavatar.mainval.t = dw_create_text_tex(ax_char_dash, C_WHITE);

	g_oavatar.secval.value = NO_VALUE;
	g_oavatar.secval.box.x = g_oavatar.box.x;
	g_oavatar.secval.box.y = g_oavatar.box.y - avatar.h;
	dw_free_texture(g_oavatar.secval.t);
	g_oavatar.secval.t = NULL;
	g_oavatar.secval.t = dw_create_text_tex(ax_char_dash, C_WHITE);

	g_ravatar.mainval.visible_box = false;
	g_ravatar.secval.visible_box = false;
	g_ravatar.in_place = false;
	g_ravatar.op2_retrieved = false;
	g_ravatar.op1_retrieved = false;
	g_ravatar.op_delivered = false;
	g_ravatar.flag1 = false;
	g_ravatar.valop = false;

	g_ravatar.mainval.value = NO_VALUE;
	g_ravatar.mainval.box.x = g_ravatar.box.x;
	g_ravatar.mainval.box.y = g_ravatar.box.y - avatar.h;
	dw_free_texture(g_ravatar.mainval.t);
	g_ravatar.mainval.t = NULL;
	g_ravatar.mainval.t = dw_create_text_tex(ax_char_dash, C_WHITE);

	g_ravatar.secval.value = NO_VALUE;
	g_ravatar.secval.box.x = g_ravatar.box.x;
	g_ravatar.secval.box.y = g_ravatar.box.y - avatar.h;
	dw_free_texture(g_ravatar.secval.t);
	g_ravatar.secval.t = NULL;
	g_ravatar.secval.t = dw_create_text_tex(ax_char_dash, C_WHITE);

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
	

	SDL_Rect r0 = {.x = g_iavatar.box.x, .y = g_iavatar.box.y, 
				  .w= g_iavatar.box.w, .h = g_iavatar.box.h};
	dw_draw_filled_rectangle(r0, g_iavatar.color, g_iavatar.color);

	SDL_Rect bf = dm_get_stage_input_buffer_box();
	int bf_y_lower = bf.y + bf.h;
	SDL_Rect rb = rg_get_register_box();
	int medium = (rb.y - bf_y_lower)/2;

	SDL_Rect ibox = dm_get_stage_ibox();
	int rail_w = dm_get_w_stage_rail();

	int offset = dm_get_ofs_buffer_value_box();
	int x = ibox.x + ibox.w/2 - rail_w/2;
	int y = rb.y - medium - rail_w/2;
	int w = bf.x + ibox.w/2 - x + offset + rail_w/2;
	int h = rail_w;

	SDL_Rect r1 = {.x = x, .y = y, .w = w, .h = h};
	dw_draw_filled_rectangle(r1, C_MAGENTA, C_MAGENTA);

	int rail_end_w = dm_get_w_stage_rail_end();

	int rail_end_x = x - (rail_end_w - rail_w)/2;
	int rail_end_y = y - (rail_end_w - rail_w)/2;
	SDL_Rect r2 = {.x = rail_end_x, .y = rail_end_y, .w = rail_end_w, 
				   .h = rail_end_w};
	dw_draw_filled_rectangle(r2 , C_MAGENTA, C_MAGENTA);

	rail_end_x = x + w - (rail_end_w - rail_w) + rail_w/2;

	SDL_Rect r3 = {.x = rail_end_x, .y = rail_end_y, .w = rail_end_w, 
				   .h = rail_end_w};
	dw_draw_filled_rectangle(r3, C_MAGENTA, C_MAGENTA);

	int xc = g_iavatar.box.x + g_iavatar.box.w/2 - rail_w/2; 
	int yc = g_iavatar.box.y + g_iavatar.box.h/2;
	int wf = rail_w;
	int yf;
	int hf;
	if (yc > y){
	 	yf = y;
		hf = yc - y + rail_w;
	} else {
		yf = yc;
		hf = y - yc + rail_w;
	}
	SDL_Rect r4 = {.x = xc, .y = yf, .w = wf, .h = hf};
	dw_draw_filled_rectangle(r4, C_MAGENTA, C_MAGENTA);

	if (g_iavatar.mainval.visible_box == true){
		ax_draw_value_box(&g_iavatar.mainval, g_iavatar.color);
	}
	if (g_iavatar.secval.visible_box == true){
		ax_draw_value_box(&g_iavatar.secval, g_iavatar.color);
	}
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
	
	SDL_Rect r0 = {.x = g_oavatar.box.x, .y = g_oavatar.box.y, 
				  .w = g_oavatar.box.w, .h = g_oavatar.box.h};
	dw_draw_filled_rectangle(r0, g_oavatar.color, g_oavatar.color);

	SDL_Rect bf = dm_get_stage_output_buffer_box();
	int bf_y_upper = bf.y;
	SDL_Rect rb = rg_get_register_box();
	int medium = (bf_y_upper - (rb.y + rb.h))/2;

	SDL_Rect obox = dm_get_stage_obox();
	int rail_w = dm_get_w_stage_rail();
	int offset = dm_get_ofs_buffer_value_box();

	int x = obox.x + obox.w/2 - rail_w/2;
	int y = bf.y - medium - rail_w/2;
	int w = bf.x + obox.w/2 - x + offset + rail_w/2;
	int h = rail_w;
	
	SDL_Rect r1 = {.x = x, .y = y, .w = w, .h = h};
	dw_draw_filled_rectangle(r1, C_CYAN, C_CYAN);

	int rail_end_w = dm_get_w_stage_rail_end();

	int rail_end_x = x - (rail_end_w - rail_w)/2;
	int rail_end_y = y - (rail_end_w - rail_w)/2;
	
	SDL_Rect r2 = {.x = rail_end_x, .y = rail_end_y, .w = rail_end_w, 
				   .h = rail_end_w};
	dw_draw_filled_rectangle(r2, C_CYAN, C_CYAN);

	rail_end_x = x + w - (rail_end_w - rail_w) + rail_w/2;
	SDL_Rect r3 = {.x = rail_end_x, .y = rail_end_y, .w = rail_end_w, 
				   .h = rail_end_w};
	dw_draw_filled_rectangle(r3, C_CYAN, C_CYAN);

	int xc = g_oavatar.box.x + g_oavatar.box.w/2 - rail_w/2; 
	int yc = g_oavatar.box.y + g_oavatar.box.h/2;
	int wf = rail_w;
	int yf;
	int hf;
	if (yc > y){
	 	yf = y;
		hf = yc - y + rail_w;
	} else {
		yf = yc;
		hf = y - yc + rail_w;
	}
	SDL_Rect r4 = {.x = xc, .y = yf, .w = wf, .h = hf};
	dw_draw_filled_rectangle(r4, C_CYAN, C_CYAN);

	if (g_oavatar.mainval.visible_box == true){
		ax_draw_value_box(&g_oavatar.mainval, g_oavatar.color);
	}
	if (g_oavatar.secval.visible_box == true){
		ax_draw_value_box(&g_oavatar.secval, g_oavatar.color);
	}
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
	

	SDL_Rect r0 = {.x = g_ravatar.box.x, .y =  g_ravatar.box.y, 
				   .w = g_ravatar.box.w, .h = g_ravatar.box.h};
	dw_draw_filled_rectangle(r0, g_ravatar.color, g_ravatar.color);

	SDL_Rect ibox = dm_get_stage_ibox();
	SDL_Rect rb = rg_get_register_box();
	int reg_ofs =  dm_get_ofs_stage_reg_box();
	int rail_w = dm_get_w_stage_rail();

	int rx = ibox.x + ibox.w/2;
	int ry = rb.y + reg_ofs;
	int rw = rail_w;
	int rh = rb.h - 2*reg_ofs;
	
	SDL_Rect r1 = {.x = rx, .y = ry, .w = rw, .h = rh};
	dw_draw_filled_rectangle(r1, C_YELLOW, C_YELLOW);
	
	int xc = g_ravatar.box.x + g_ravatar.box.w/2; 
	int yc = g_ravatar.box.y + g_ravatar.box.h/2 - rail_w/2;
	int hf = rail_w;
	int xf;
	int wf;
	if (xc < rx){
	 	xf = xc;
		wf = rx - xf + rail_w;
	} else {
		xf = rx;
		wf = xc - xf + rail_w;
	}
	SDL_Rect r2 = {.x = xf, .y = yc, .w = wf, .h = hf};
	dw_draw_filled_rectangle(r2, C_YELLOW, C_YELLOW);

	int rail_end_w = dm_get_w_stage_rail_end();

	int rail_end_x = rx - (rail_end_w - rail_w)/2;
	int rail_end_y = ry - (rail_end_w - rail_w)/2;

	SDL_Rect r3 = {.x = rail_end_x, .y = rail_end_y, .w = rail_end_w, 
				   .h = rail_end_w};
	dw_draw_filled_rectangle(r3, C_YELLOW, C_YELLOW);

	rail_end_y = ry + rh - (rail_end_w - rail_w) + rail_w/2;
	SDL_Rect r4 = {.x = rail_end_x, .y = rail_end_y, .w = rail_end_w, 
				   .h = rail_end_w};
	dw_draw_filled_rectangle(r4, C_YELLOW, C_YELLOW);

	if (g_ravatar.mainval.visible_box == true){
		ax_draw_value_box(&g_ravatar.mainval, g_ravatar.color);
	}
	if (g_ravatar.secval.visible_box == true){
		ax_draw_value_box(&g_ravatar.secval, g_ravatar.color);
	}
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
	if (op_id > REG_MIN && op_id < REG_MAX){
		y = rg_get_register_value_box_y_coord_by_id(op_id);
	} else if (op_id > FLAG_MIN && op_id < FLAG_MAX){
		y = rg_get_flag_value_box_by_id(op_id).box.y;
	} else if (op_id > BUF_MIN && op_id < BUF_MAX){
		y = bf_get_buffer_value_box_y_coord_by_id(op_id);	
	} else if (op_id > IMM_MIN && op_id < IMM_MAX){
		y = im_get_imm_value_box_y_coord_by_id(op_id) 
		    +  dm_get_avatar_wh().h;
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
	if (op_id > REG_MIN && op_id < REG_MAX){
		x = rg_get_register_value_box_x_coord_by_id(op_id);
	} else if (op_id > FLAG_MIN && op_id < FLAG_MAX){
		x = rg_get_flag_value_box_by_id(op_id).box.x;
	} else if (op_id > BUF_MIN && op_id < BUF_MAX){
		x = bf_get_buffer_value_box_x_coord_by_id(op_id);	
	} else if (op_id > IMM_MIN && op_id < IMM_MAX){
		x = im_get_imm_value_box_x_coord_by_id(op_id);	
	} else if (op_id == IBOX || op_id == OBOX){
		x = rg_get_ibox_x();
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
static bool move_avatar_to_operand(avatar_t *avatar, int op_id)
{
	assert(op_id > REG_MIN && op_id < RGBOX_MAX &&
		   "Invalid operand");

	SDL_Rect avtr = dm_get_avatar_wh();
	bool mov = false;
	int x;
	int y;

	if (op_id > REG_MIN && op_id < REG_MAX){
		x = get_operand_x_dest(op_id) + 1.5*avtr.w;
	} else if (op_id > FLAG_MIN && op_id < FLAG_MAX){
		x = get_operand_x_dest(op_id) - 1.5*avtr.w;
	} else {
		x = get_operand_x_dest(op_id);
	}

	SDL_Rect vb = dm_get_value_box_wh();
	int vbox_offset = dm_get_ofs_reg_value_box();
	if (op_id > REG_MIN && op_id < REG_MAX){
		y = get_operand_y_dest(op_id);
	} else if (op_id > FLAG_MIN && op_id < FLAG_MAX){
		y = get_operand_y_dest(op_id);
	} else if (op_id > IMM_MIN && op_id < IMM_MAX){
		y = get_operand_y_dest(op_id);
	} else if (op_id == IB){
		y = get_operand_y_dest(op_id) + 1.5*vb.h;
	} else if (op_id == OB){
		y = get_operand_y_dest(op_id) - 2*vb.h - avtr.h;
	} else if (op_id == IBOX){
		if (avatar->id == RAVATAR){
			y = get_operand_y_dest(op_id) + vbox_offset;
		} else{
			y = get_operand_y_dest(op_id) - 2.5*vb.h - vbox_offset;
		}
	} else if (op_id == OBOX){
		if (avatar->id == RAVATAR){
			y = get_operand_y_dest(op_id) - 2.5*vb.h - 2*vbox_offset;
		} else if (avatar->id == OAVATAR){
			y = get_operand_y_dest(op_id);
		}
	} else {
		return mov;
	}
	
	int mdelta = ax_get_move_delta();
//	int xd = ax_get_x_delta_div(avatar->value.box.x, x, avatar->value.box.y, y);
//	xd = mdelta/xd;
//	int yd = ax_get_y_delta_div(avatar->value.box.x, x, avatar->value.box.y, y);
//	yd = mdelta/yd;

//	printf("xd = %d, yd = %d\n", xd, yd);

	//printf("x = %d, y = %d\n", x, y);
	//printf("avatar->box.x = %d, avatar->box.y= %d\n", avatar->box.x, avatar->box.y);
	if (avatar->box.x < x){
		int delta = ax_get_movement_delta(avatar->box.x, x, mdelta);
		avatar->box.x += delta;
		avatar->mainval.box.x += delta;
		mov = true;
	//	puts("esto deberia pasar");
	//printf("avatar->box.x = %d, avatar->box.y= %d\n", avatar->box.x, avatar->box.y);
	} else if (avatar->box.x > x){
		int delta = ax_get_movement_delta(avatar->box.x, x, mdelta);
		avatar->box.x -= delta;
		avatar->mainval.box.x -= delta;
		mov = true;
	}
	
	int avatar_final_y = avatar->box.y - avtr.h;
	if (avatar_final_y < y){
		int delta = ax_get_movement_delta(avatar->mainval.box.y, y, mdelta);
		avatar->box.y += delta;
		avatar->mainval.box.y += delta;
		mov = true;
	} else if (avatar_final_y > y){
		int delta = ax_get_movement_delta(avatar->mainval.box.y, y, mdelta);
		avatar->box.y -= delta;
		avatar->mainval.box.y -= delta;
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
	assert((op_id > REG_MIN && op_id < REG_MAX) 
		   || (op_id > FLAG_MIN &&  op_id < FLAG_MAX)
		   || (op_id > BUF_MIN &&  op_id < BUF_MAX) 
		   || (op_id > RGBOX_MIN &&  op_id < RGBOX_MAX) 
		   || (op_id > IMM_MIN &&  op_id < IMM_MAX) 
		   && "The operand id is invalid");
	
	value_box_t op_value_box;

	if (op_id > REG_MIN && op_id < REG_MAX){
		op_value_box = rg_get_register_value_box_by_id(op_id);	
	} else if (op_id > FLAG_MIN && op_id < FLAG_MAX){
		op_value_box = rg_get_flag_value_box_by_id(op_id);	
	} else if (op_id > IMM_MIN && op_id < IMM_MAX){
		op_value_box = im_get_imm_value_box_by_id(op_id);	
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
	if (avatar->mainval.value == NO_VALUE){
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
	assert((op_id > REG_MIN && op_id < REG_MAX)
		   || (op_id > FLAG_MIN && op_id < FLAG_MAX)
		   || (op_id > BUF_MIN &&  op_id < BUF_MAX) 
		   || (op_id > RGBOX_MIN &&  op_id < RGBOX_MAX) && 
		   "The operand id is invalid");

	value_box_t op_value_box;

	if (op_id > REG_MIN && op_id < REG_MAX){
		op_value_box = rg_get_register_value_box_by_id(op_id);	
	} else  if (op_id > FLAG_MIN && op_id < FLAG_MAX){
		op_value_box = rg_get_flag_value_box_by_id(op_id);	
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
	assert(((op_id > REG_MIN && op_id < REG_MAX) 
			|| (op_id > FLAG_MIN && op_id < FLAG_MAX) 
			|| (op_id > BUF_MIN && op_id < BUF_MAX) 
			|| (op_id > RGBOX_MIN && op_id < RGBOX_MAX)) && 
		    "The operand id is invalid");
	
	bool operation_valid = true;

	if (op_id > REG_MIN && op_id < REG_MAX){
		rg_set_register_value_box(op_id, val);	
	} else if (op_id > FLAG_MIN && op_id < FLAG_MAX){
		rg_set_flag_value_box(op_id, val);	
	} else if (op_id == OB){
		bf_set_output_buffer_value_box(val);	
	} else if (op_id == IBOX){
		rg_set_ibox_value_box(val);	
	} else if (op_id == OBOX){
		rg_set_obox_value_box(val);	
	}
	return operation_valid;
}

/* Function: cmp_substract
 * -----------------------------------------------------------------------------
 * Substract the current value of the operand with the value provided 
 * by the avatar.
 * if the operand value is not valid, it returns an invalid operation.
 *  
 * Arguments:
 *	op_id: the id of the operand the avatar will retrieve
 *  val: the avatar value box that will modify the operand.
 *
 * Return:
 *	bool indicating if part of the retriving is pending
 */
bool cmp_substract(int op_id, value_box_t val)
{ 
	assert(op_id > REG_MIN && op_id < BUF_MAX && 
		   "The operand id is invalid");
	
	bool operation_valid = true;

	if (op_id > REG_MIN && op_id < REG_MAX){
		value_box_t cur_val = rg_get_register_value_box_by_id(op_id);
		if (cur_val.value == NO_VALUE){
			operation_valid = false;	
		} else {
			cur_val.value -= val.value;
			ax_copy_vbox(&g_ravatar.mainval, cur_val, false);
			g_ravatar.mainval.visible_box = true;
		}
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
	assert(op_id > REG_MIN && op_id < BUF_MAX && 
		   "The operand id is invalid");
	
	bool operation_valid = true;

	if (op_id > REG_MIN && op_id < REG_MAX){
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
		case CMP:
			op_status = cmp_substract(line->op1->id, value);
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

	int mdelta = ax_get_move_delta();
	if (avatar->secval.box.x < x){
		int delta = ax_get_movement_delta(avatar->secval.box.x, x, mdelta/2);
		avatar->secval.box.x += delta;
		mov = true;
	} else if (avatar->secval.box.x > x){
		int delta = ax_get_movement_delta(avatar->secval.box.x, x, mdelta/2);
		avatar->secval.box.x -= delta;
		mov = true;
	}
	if (avatar->secval.box.y < y){
		int delta = ax_get_movement_delta(avatar->secval.box.y, y, mdelta/2);
		avatar->secval.box.y += delta;
		mov = true;
	} else if (avatar->secval.box.y > y){
		int delta = ax_get_movement_delta(avatar->secval.box.y, y, mdelta/2);
		avatar->secval.box.y -= delta;
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
	SDL_Rect avtr = dm_get_avatar_wh();
	int x = avatar->box.x;
	int y = avatar->box.y - avtr.h;	

	bool mov = false;

	int mdelta = ax_get_move_delta();
	if (avatar->secval.box.x < x){
		int delta = ax_get_movement_delta(avatar->secval.box.x, x, mdelta/2);
		avatar->secval.box.x += delta;
		mov = true;
	} else if (avatar->secval.box.x > x){
		int delta = ax_get_movement_delta(avatar->secval.box.x, x, mdelta/2);
		avatar->secval.box.x -= delta;
		mov = true;
	}
	if (avatar->secval.box.y < y){
		int delta = ax_get_movement_delta(avatar->secval.box.y, y, mdelta/2);
		avatar->secval.box.y += delta;
		mov = true;
	} else if (avatar->secval.box.y > y){
		int delta = ax_get_movement_delta(avatar->secval.box.y, y, mdelta/2);
		avatar->secval.box.y -= delta;
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
	if (mov_pending == false 
		&& g_iavatar.in_place == false 
		&& g_iavatar.op2_retrieved == false){
		g_iavatar.in_place = true;
		if (op_id == IB && is_operand_retrievable(op_id) == true){
			value_box_t b = get_operand_value_box(op_id);
			ax_copy_vbox(&g_iavatar.mainval, b, false);
			g_iavatar.mainval.visible_box = false;
			ax_copy_vbox(&g_iavatar.secval, b, true);
			g_iavatar.secval.visible_box = true;
		} else if (op_id > IMM_MIN 
				   && op_id < IMM_MAX 
				   && is_operand_retrievable(op_id) == true){
			value_box_t b = get_operand_value_box(op_id);
			ax_copy_vbox(&g_iavatar.mainval, b, false);
			g_iavatar.mainval.visible_box = false;
			ax_copy_vbox(&g_iavatar.secval, b, true);
			g_iavatar.secval.visible_box = true;
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
	if (mov_pending == false 
		&& g_ravatar.in_place == false ){
	//	&& g_ravatar.op2_retrieved == false){
		g_ravatar.in_place = true;
		if (check_operand_has_value(op_id) == true){
			value_box_t b = get_operand_value_box(op_id);
			ax_copy_vbox(&g_ravatar.mainval, b, false);
			g_ravatar.mainval.visible_box = false;
			ax_copy_vbox(&g_ravatar.secval, b, true);
			g_ravatar.secval.visible_box = true;

			rg_reset_ibox();
		}
		else if (cl_is_op_reg(op_id)){
			set_invalid_operation_flag(REG_VALUE_INVALID);
		} else {
			set_invalid_operation_flag(FLAG_VALUE_INVALID);
		}
	}
	return mov_pending;
}

/* Function: handle_ravatar_cmp
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
static bool handle_ravatar_cmp_source(int op_id)
{
	bool mov_pending = true;

	mov_pending = move_avatar_to_operand(&g_ravatar, op_id);

	if (mov_pending == false
		&& g_ravatar.in_place == false 
		&& g_ravatar.op1_retrieved == false){
		g_ravatar.in_place = true;
		if (check_operand_has_value(op_id) == true){
			value_box_t b = get_operand_value_box(op_id);
			ax_copy_vbox(&g_ravatar.secval, b, true);
			g_ravatar.secval.visible_box = true;
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
			value_box_t b = get_operand_value_box(op_id);
			ax_copy_vbox(&g_oavatar.mainval, b, false);
			g_oavatar.mainval.visible_box = false;
			ax_copy_vbox(&g_oavatar.secval, b, true);
			g_oavatar.secval.visible_box = true;
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
	int op_qty = cl_get_instruction_operand_quantity(line->ins->id);
	int mov_pending = NO_VALUE;
	int avatar_id = NOAVATAR;

	if(op_qty == TWO_OPERANDS){
		if (check_operand_has_value(IBOX) == true){
			mov_pending = handle_ravatar_source_operand(IBOX);	
			avatar_id = RAVATAR;
		} else if (line->ins->id == CMP 
				   && check_avatar_has_value(&g_ravatar) == true
				   && g_ravatar.op2_retrieved == true
				   && g_ravatar.op1_retrieved == false){
			mov_pending = handle_ravatar_cmp_source(line->op1->id);	
			avatar_id = RAVATAR;
		} else if (check_avatar_has_value(&g_ravatar) == true){
			mov_pending = false;
			avatar_id = RAVATAR;
		} else if (cl_is_op_reg(line->op2->id)
				   && check_operand_has_value(OBOX) == false 
				   && check_avatar_has_value(&g_oavatar) == false){
			if (g_ravatar.op2_retrieved == false) {
				mov_pending = handle_ravatar_source_operand(line->op2->id);	
			}
			avatar_id = RAVATAR;
		} else if (line->op2->id == IB){
			if (g_iavatar.op2_retrieved == false){
				mov_pending = handle_iavatar_source_operand(line->op2->id);	
			}
			avatar_id = IAVATAR;
		} else if (line->op2->id > IMM_MIN && line->op2->id < IMM_MAX){
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

	} else if(op_qty == ONE_OPERAND){
		if ((line->ins->id == JE || line->ins->id == JNE)
			&& g_ravatar.op1_retrieved == false){
			mov_pending = handle_ravatar_source_operand(ZF);	
			avatar_id = RAVATAR;
		}
	}

	if (mov_pending == false){
		bool retrieve_pending; 
		
		if (avatar_id == RAVATAR 
			&& (line->ins->id == JE || line->ins->id == JNE)
			&& g_ravatar.op1_retrieved == false){
			retrieve_pending = retrieve_operand(&g_ravatar);
			if (retrieve_pending == false){
				g_ravatar.op1_retrieved = true;
				g_ravatar.op2_retrieved = true;
				g_ravatar.op_delivered = true;
				g_ravatar.in_place = false;
				g_ravatar.secval.visible_box = false;
				g_ravatar.mainval.visible_box = true;
				g_ravatar.mainval.box = g_ravatar.secval.box;
			}
		} else if (avatar_id == RAVATAR 
			&& line->ins->id == CMP			
			&& g_ravatar.op2_retrieved == true
			&& g_ravatar.op1_retrieved == false){
			retrieve_pending = retrieve_operand(&g_ravatar);
			if (retrieve_pending == false){
				g_ravatar.op1_retrieved = true;
				g_ravatar.in_place = false;
				g_ravatar.secval.visible_box = false;
				g_ravatar.mainval.visible_box = true;
				g_ravatar.mainval.box = g_ravatar.secval.box;
			}
		} else if (avatar_id == RAVATAR 
				   && op_qty == TWO_OPERANDS
				   && g_ravatar.op2_retrieved == false){
			retrieve_pending = retrieve_operand(&g_ravatar);
			if (retrieve_pending == false){
				g_ravatar.op2_retrieved = true;
				g_ravatar.in_place = false;
				g_ravatar.secval.visible_box = false;
				g_ravatar.mainval.visible_box = true;
				g_ravatar.mainval.box = g_ravatar.secval.box;
			}
		} else if (avatar_id == IAVATAR){
			retrieve_pending = retrieve_operand(&g_iavatar);
			if (retrieve_pending == false){
				g_iavatar.op2_retrieved = true;
				g_iavatar.in_place = false;
				g_iavatar.secval.visible_box = false;
				g_iavatar.mainval.visible_box = true;
				g_iavatar.mainval.box = g_iavatar.secval.box;
			}
		} else if (avatar_id == OAVATAR && g_oavatar.op2_retrieved == false){
			retrieve_pending = retrieve_operand(&g_oavatar);
			if (retrieve_pending == false){
				g_oavatar.op2_retrieved = true;
				g_oavatar.in_place = false;
				g_oavatar.secval.visible_box = false;
				g_oavatar.mainval.visible_box = true;
				g_oavatar.mainval.box = g_oavatar.secval.box;
				if (line->op1->id == OB){
					bf_add_output_to_list();
				}
			}
		}
	}
	return avatar_id;
}

/* Function: rflag_generator 
 * -----------------------------------------------------------------------------
 * This function sets the flag in the second value box, according to the 
 * contents of the first value box
 *
 * Arguments:
 *	avatar: The avatar that will be used to generate the flag.
 * 	id: The id of the flag that will be generated.
 *
 * Return:
 *	void.
 */
static void rflag_generator(avatar_t *avatar, int id)
{
	value_box_t flag;
	switch(id){
		case ZF:
			if (avatar->mainval.value == 0){
				flag = avatar->mainval;
				flag.value = 1;
				ax_copy_vbox(&g_ravatar.secval, flag, true);
			} else if (avatar->mainval.value != 0){
				flag = avatar->mainval;
				flag.value = 0;
				ax_copy_vbox(&g_ravatar.secval, flag, true);
			}
			break;
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
static void handle_destiny_operand(code_line_t *line, int avatar_id)
{
	assert(line != NULL && "The value of line cannot be NULL");
	int mov_pending = NO_VALUE;
	
	if (avatar_id == RAVATAR 
		&& line->ins->id == CMP 
		&& g_ravatar.op2_retrieved == true
		&& g_ravatar.op1_retrieved == true
		&& g_ravatar.op_delivered == false){
			mov_pending = move_avatar_to_operand(&g_ravatar, ZF);
		if (mov_pending == false && g_ravatar.in_place == false){
			rflag_generator(&g_ravatar, ZF);
			g_ravatar.in_place = true;
			g_ravatar.mainval.visible_box = true;
			g_ravatar.secval.visible_box = true;
			g_ravatar.secval.box = g_ravatar.mainval.box;
		}
		if (mov_pending == false){
			int deliver_pending;
			deliver_pending = deliver_operand(&g_ravatar, ZF);
			if (deliver_pending == false){
				g_ravatar.op_delivered = true;
				g_ravatar.secval.visible_box = false;
			}
		}
	} else if (avatar_id == RAVATAR 
			   && line->ins->id != CMP
			   && g_ravatar.op2_retrieved == true
			   && g_ravatar.op_delivered == false){
		if (cl_is_op_reg(line->op1->id) == true){
			mov_pending = move_avatar_to_operand(&g_ravatar, line->op1->id);
		} else {
			mov_pending = move_avatar_to_operand(&g_ravatar, OBOX);
		}
		if (mov_pending == false && g_ravatar.in_place == false){
			g_ravatar.in_place = true;
			g_ravatar.mainval.visible_box = false;
			g_ravatar.secval.visible_box = true;
			g_ravatar.secval.box = g_ravatar.mainval.box;
		}
		if (mov_pending == false){
			int deliver_pending;
			if (cl_is_op_reg(line->op1->id) == true){
				deliver_pending = deliver_operand(&g_ravatar, line->op1->id);
			} else {
				deliver_pending = deliver_operand(&g_ravatar, OBOX);
			}
			if (deliver_pending == false){
				g_ravatar.op_delivered = true;
				g_ravatar.secval.visible_box = false;
			}
		}
	}
	
	if (avatar_id == IAVATAR && g_iavatar.op2_retrieved == true){
		mov_pending = move_avatar_to_operand(&g_iavatar, IBOX);
		if (mov_pending == false && g_iavatar.in_place == false){
			g_iavatar.in_place = true;
			g_iavatar.mainval.visible_box = false;
			g_iavatar.secval.visible_box = true;
			g_iavatar.secval.box = g_iavatar.mainval.box;
		}
		if (mov_pending == false){
			int deliver_pending = deliver_operand(&g_iavatar, IBOX);
			if (deliver_pending == false){
				g_iavatar.op_delivered = true;
				g_iavatar.secval.visible_box = false;
			}
		}
	}
	if (avatar_id == OAVATAR && g_oavatar.op2_retrieved == true){
		mov_pending = move_avatar_to_operand(&g_oavatar, line->op1->id);
		if (mov_pending == false && g_oavatar.in_place == false){
			g_oavatar.in_place = true;
			g_oavatar.mainval.visible_box = false;
			g_oavatar.secval.visible_box = true;
			g_oavatar.secval.box = g_oavatar.mainval.box;
		}
		if (mov_pending == false){
			int deliver_pending = deliver_operand(&g_oavatar ,line->op1->id);
			if (deliver_pending == false){
				g_oavatar.op_delivered = true;
				g_oavatar.mainval.visible_box = false;
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


/* Function: check_finishes_at_OB_correct_size
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
static bool check_finishes_at_OB_correct_size()
{
	bool finished = false;
	
	bool win = lv_check_if_win();
	int input_buffer_size = get_input_buffer_list_size();
	bool correct_code_size = check_correct_code_size();

	if (win == true && input_buffer_size !=0) {
	//	set_invalid_operation_flag(UNPROCESSED_IB_VALUES);
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
	bool arrow_in_place = ar_move_execution_arrow(line_pos);
	mc_set_step_ended(false);

	if (line->ins->id == LABEL && arrow_in_place == true){
		line->state = EXECUTED;
		reset_avatar_no_pos();
	} else if (line->ins->id == JMP && arrow_in_place == true){
		cw_operate_jump_instruction(line);
		reset_avatar_no_pos();
	} else if (buffer_inputs != CW_EMPTY 
			  || line->state == IN_EXECUTION 
			  || line->state == COMPLETE){
		line->state = IN_EXECUTION;
		avatar_id = handle_source_operand(line);
		handle_destiny_operand(line, avatar_id);
	
		bool rst = false;
		if (avatar_id == RAVATAR 
			&& g_ravatar.op2_retrieved == true 
			&& (g_ravatar.op_delivered == true 
				|| g_ravatar.op1_retrieved == true)
			&& arrow_in_place == true){
			
			if (line->ins->id == CMP){
				if (g_ravatar.op_delivered == false 
					&& g_ravatar.valop == false){
					g_ravatar.valop = true;
					operate_instruction(line, g_ravatar.mainval);
				} else if (g_ravatar.op_delivered == true){
					set_operand_value_box(ZF, g_ravatar.secval);
					operate_instruction(line, g_ravatar.mainval);
					line->state = EXECUTED;
					mc_set_step_ended(true);
					rst = true;
				}
			} else if (line->ins->id == JE){
				if (g_ravatar.mainval.value == 1){
				cw_operate_jump_instruction(line);
				} else if (g_ravatar.mainval.value == 0){
					line->state = EXECUTED;
				}
				reset_avatar_no_pos();
			} else if (line->ins->id == JNE){
				if (g_ravatar.mainval.value == 0){
				cw_operate_jump_instruction(line);
				} else if (g_ravatar.mainval.value == 1){
					line->state = EXECUTED;
				}
				reset_avatar_no_pos();
			} else if (cl_is_op_reg(line->op1->id) == true){
				operate_instruction(line, g_ravatar.mainval);
				line->state = EXECUTED;
				mc_set_step_ended(true);
				rst = true;
			} else{
				set_operand_value_box(OBOX, g_ravatar.mainval);
				rst = true;
			}
			if (rst == true){
				reset_avatar_no_pos();
			}
		}

		if (avatar_id == IAVATAR && g_iavatar.op2_retrieved == true && 
			g_iavatar.op_delivered == true && arrow_in_place == true){
			set_operand_value_box(IBOX, g_iavatar.mainval);
			reset_avatar_no_pos();
		}
		if (avatar_id == OAVATAR && g_oavatar.op2_retrieved == true && 
			g_oavatar.op_delivered == true && arrow_in_place == true){
			operate_instruction(line, g_oavatar.mainval);
			line->state = EXECUTED;
			mc_set_step_ended(true);
			reset_avatar_no_pos();
		}	
	}
}

/* Function: mc_set_run_ended
 * -----------------------------------------------------------------------------
 * Sets the fun ended variable
 *
 * Arguments:
 *	state: state to which the variable will be set.
 *
 * Return:
 *	Void.
 */
void mc_set_run_ended(bool state)
{
	run_ended = state;
}

/* Function: mc_get_run_ended
 * -----------------------------------------------------------------------------
 * Returns the run_ended variable.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Run ended variable.
 */
bool mc_get_run_ended()
{
	return run_ended;
}

/* Function: mc_set_step_ended
 * -----------------------------------------------------------------------------
 * Sets the step ended variable
 *
 * Arguments:
 *	state: state to which the variable will be set.
 *
 * Return:
 *	Void.
 */
void mc_set_step_ended(bool state)
{
	step_ended = state;
}

/* Function: mc_get_step_ended
 * -----------------------------------------------------------------------------
 * Returns the step_ended variable.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Run ended variable.
 */
bool mc_get_step_ended()
{
	return step_ended;
}

/* Function: mc_run_code
 * -----------------------------------------------------------------------------
 * Arguments:
 *	none.
 *
 * Return:
 *	true if a step is pending, false if otherwise
 */
void mc_run_code()
{
	mc_set_run_ended(false);
	int code_size = cw_get_code_list_size();	
	
	for (int i = 0; i < code_size; i++){
		code_line_t *line = cw_get_code_line_at_pos(i);
		if (line->state != EXECUTED){
			execute_instruction(line, i);	
		if (check_finishes_at_OB_correct_size() == true){
			mc_set_run_ended(true);
		}
			return;
		}
	}
	mc_set_run_ended(true);
	int output_buffer_size = get_output_buffer_list_size();
	int win_list_size = lv_get_win_list_size();
	if (output_buffer_size < win_list_size){
		set_invalid_operation_flag(OUTPUT_BUFFER_INCOMPLETE);
	}
	return;
	
}














































