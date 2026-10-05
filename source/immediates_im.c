#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "immediates_im.h"
#include "code_line_cl.h"
#include "dimensions_dm.h"
#include "draw_dw.h"
#include "code_window_cw.h"
#include "aux.h"
#include "ui/button_bt.h"

#define TOTAL_IMM 20
#define IMM_CELL_BORDER_WIDTH 5


texture_t *g_imm_txt = NULL;

static bool g_imm_up = false;
static unsigned int g_imm_highlight_sources;
static bool g_imm_highlight;
static bool g_imm_highlight_rendered;
static dw_pulse_t g_imm_pulse;

static void init_imm_texture();
static void draw_imm_txt_up(float pulse);
static void draw_imm_cell(int index);

typedef struct imm_t{
	btn_t *b;
	value_box_t val;
} imm_t;

imm_t g_up_imm[TOTAL_IMM];

/* Function: im_get_buffer_value_box_x_coord_by_id
 *------------------------------------------------------------------------------
 * Arguments:
 *	op_id: The id of the operand that the x value will get calculated
 *
 * Return:
 *	x coordinate of the first value box
 */
int im_get_imm_value_box_x_coord_by_id(int id)
{
	assert(id > IMM_MIN && id < IMM_MAX && "Invalid buffer id");
	
	int index = id - IMMUP0;

	return g_up_imm[index].val.box.x;
}

/* Function: im_get_buffer_value_box_y_coord_by_id
 *------------------------------------------------------------------------------
 * Arguments:
 *	op_id: The id of the operand that the x value will get calculated
 *
 * Return:
 *	y coordinate of the first value box
 */
int im_get_imm_value_box_y_coord_by_id(int id)
{
	assert(id > IMM_MIN && id < IMM_MAX && "Invalid buffer id");
	
	int index = id - IMMUP0;

	return g_up_imm[index].val.box.y;
}

/* Function: im_create_imm_op_by_id
*------------------------------------------------------------------------------
*  creates an operand of the selected immediate
*
* Arguments:
*	Void.
*	
* Return:
*	The pointer to the created imm operand
*
*/
operand_t *im_create_imm_op_by_id(int op_id)
{
	assert(op_id > IMM_MIN && op_id < IMM_MAX && "Invalid op_id");
	operand_t *o = NULL;
	for (int i = IMMUP0; i < IMM_MAX; i++){
		if (i == op_id){
			char *num;
			if (op_id < IMMUP_1){
	   			num = ax_number_to_string(i - IMMUP0);
			} else if (op_id > IMMUP10){
	   			num = ax_number_to_string(-(i - IMMUP10));
			}	
			texture_t *t = dw_create_text_tex(num, C_WHITE);
			free(num);
			SDL_Rect cb = dm_get_code_button_wh();
			SDL_Rect r = {.x = 0, .y = 0, .w = cb.w, .h = cb.h};
			btn_t *b = bt_create_btn(r, t);
			o = malloc(sizeof(operand_t));
			o->b = b;
			o->id = op_id; 
			break;
		} 
	}
			
   return o;
}

/* Function: rg_get_imm_value_box_by_id
*------------------------------------------------------------------------------
* Arguments:
*	id: The id of the value box that will be retrieved	
*
* Return:
*	The value box
*/
value_box_t im_get_imm_value_box_by_id(int id)
{
	assert(id > IMM_MIN && id < IMM_MAX && "Invalid imm id");
	int index = id - IMMUP0;

	value_box_t val = g_up_imm[index].val;

   return val;
}


/* Function: im_create_sel_imm_op
*------------------------------------------------------------------------------
*  creates an operand of the selected immediate
*
* Arguments:
*	Void.
*	
* Return:
*	The pointer to the created imm operand
*
*/
operand_t *im_create_sel_imm_op()
{
	operand_t *o = NULL;
	int imm_id = IMMUP0;
	for (int i = 0; i < TOTAL_IMM; i++){
		if (bt_chk_rel_btn(g_up_imm[i].b, NULL) == true){
	   		char *num = ax_number_to_string(g_up_imm[i].val.value);
			texture_t *t = dw_create_text_tex(num, C_WHITE);
			free(num);
			SDL_Rect cb = dm_get_code_button_wh();
			SDL_Rect r = {.x = 0, .y = 0, .w = cb.w, .h = cb.h};
			btn_t *b = bt_create_btn(r, t);
			o = malloc(sizeof(operand_t));
			o->b = b;
			o->id = imm_id; 
		} 
		imm_id++;
	}
			
   return o;
}


/* Function: sb_set_imm_up_avail
 * ----------------------------------------------------------------------------
 * Sets the global variable that determines if the up immediates will be 
 * available
 *
 * Arguments:
 * 	state: state that the g_step_btns_avail variable will be set
 *
 * Return:
 *	Void.	
 */
void im_set_imm_up_avail(bool state)
{
	g_imm_up = state;
}

bool im_are_imm_up_available(void)
{
	return g_imm_up;
}

void im_set_highlight_source(ui_highlight_source_t source, bool enabled)
{
	g_imm_highlight_sources = ui_highlight_source_set_enabled(
	    g_imm_highlight_sources, source, enabled);
	g_imm_highlight = g_imm_highlight_sources != 0;
}

/* Function: im_init_imm_assets
 *------------------------------------------------------------------------------
 * Initializes the assets of the immediates of the level
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Void.
 */
void im_init_imm_assets()
{
	init_imm_texture();

	int x = dm_get_stage_imm_up().x;
	int y = 0; 
	SDL_Rect cell = dm_get_imm_cell_wh();
	int val = 0;
	for (int i = 0; i < TOTAL_IMM; i++){
		g_up_imm[i].val.box.x = x;
		g_up_imm[i].val.box.y = y;
		g_up_imm[i].val.box.w = cell.w;
		g_up_imm[i].val.box.h = cell.h;
		g_up_imm[i].val.visible_box = true;
		
		g_up_imm[i].val.value = val;
		char *num =  ax_number_to_string(val);
		g_up_imm[i].val.t = dw_create_text_tex(num, C_WHITE);
		free(num);
		g_up_imm[i].b = malloc(sizeof(btn_t));
		g_up_imm[i].b->r = g_up_imm[i].val.box;
		g_up_imm[i].b->enabled = true;

		if (i <= 9){
			val++;
			x += cell.w;
		} else if (i == 10){
			y += cell.h;
			x = dm_get_stage_imm_up().x + cell.w;
			val = -1;
		} else {
			val --;
			x += cell.w;
		}
	}
}

/* Function: im_click_up_imm
 *------------------------------------------------------------------------------
 * Verifies if one of the upper immediates was clicked
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	True if any of the upper immediates was clicked.
 */
bool im_ms_rel_in_upimm()
{
 	bool rel = false;

	for (int i = 0; i < TOTAL_IMM; i++){

		if (bt_chk_rel_btn(g_up_imm[i].b, NULL) == true){
		   	rel = true;
		   	break;
	   	} 
	}
	return rel;
}

/* Function: init_imm_texture
 *------------------------------------------------------------------------------
 * Creates the instructions texture of the immediate text
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Void.
 */
static void init_imm_texture()
{
	g_imm_txt = dw_create_text_tex(IMM_TXT, C_AMBER);
}


/* Function: im_draw_up_imm
 * -----------------------------------------------------------------------------
 * Draws the upper immediates
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Void.
 */
void im_draw_imm()
{
	bool highlighted = g_imm_up && g_imm_highlight;
	if (highlighted != g_imm_highlight_rendered) {
		dw_pulse_reset(&g_imm_pulse);
		g_imm_highlight_rendered = highlighted;
	}
	if (g_imm_up == true){
		float anim_limit = cw_get_challenge_highlight_limit();
		float pulse = highlighted ?
		    dw_pulse_value(&g_imm_pulse, anim_limit) : 0.0f;
		draw_imm_txt_up(pulse);
		for (int i = 0; i < TOTAL_IMM; i++){
			draw_imm_cell(i);
		}
		if (highlighted) {
			dw_pulse_advance(&g_imm_pulse, anim_limit);
		}
	}	
	

}

/* Function: draw_imm_txt_up
 *------------------------------------------------------------------------------
 * Draws the immediates of the upper section of the screen 
 *
 * Arguments:
 *	Void.
 * 
 * Return:
 *	Void.
 *
 */
static void draw_imm_txt_up(float pulse)
{
	SDL_Rect imm_box = dm_get_stage_imm_up();
	SDL_Rect cell = dm_get_imm_cell_wh();
	int text_h = dm_get_h_stage_elements_titles();
	int text_w = get_text_width_fits_height(text_h, IMM_TXT);
	int x = imm_box.x + (11 * cell.w - text_w) / 2;
	int y = 2 * cell.h;

	if (g_imm_highlight) {
		SDL_FRect label = {
			.x = x,
			.y = y,
			.w = get_text_width_fits_height(text_h, IMM_TXT),
			.h = text_h
		};
		label = dw_grow_rect_height(
		    label, pulse * DW_TEXT_HIGHLIGHT_GROWTH_FACTOR);
		dw_set_texture_color_mod(g_imm_txt, C_LIGHTGREY);
		dw_draw_texture_fit_h_f(label, g_imm_txt);
		dw_set_texture_color_mod(g_imm_txt, C_AMBER);
	} else {
		SDL_Rect label = {.x = x, .y = y, .h = text_h};
		dw_draw_texture_fit_h(label, g_imm_txt);
	}
}

static void draw_imm_cell(int index)
{
	value_box_t *box = &g_up_imm[index].val;
	int border = dm_scale_to_res(IMM_CELL_BORDER_WIDTH);
	int shared_leading_border = border / 2;
	int shared_trailing_border = border - shared_leading_border;
	bool has_left = (index > 0 && index <= 10) || index >= 12;
	bool has_right = index < 10 || (index >= 11 && index < TOTAL_IMM - 1);
	bool has_top = index >= 11;
	bool has_bottom = index >= 1 && index <= 9;
	int text_h = dm_get_value_box_val_wh().h;
	int text_w = box->t != NULL ? ax_get_texture_w_fit_h(text_h, box->t) : 0;
	SDL_Rect text = {
		.x = box->box.x + (box->box.w - text_w) / 2,
		.y = box->box.y + (box->box.h - text_h) / 2,
		.h = text_h
	};
	SDL_Rect inner = box->box;
	int left_border = has_left ? shared_leading_border : border;
	int right_border = has_right ? shared_trailing_border : border;
	int top_border = has_top ? shared_leading_border : border;
	int bottom_border = has_bottom ? shared_trailing_border : border;

	dw_draw_filled_rectangle(box->box, C_GREY, C_GREY);
	inner.x += left_border;
	inner.y += top_border;
	inner.w -= left_border + right_border;
	inner.h -= top_border + bottom_border;
	dw_draw_filled_rectangle(inner, C_BLACK, C_BLACK);
	if (box->t != NULL) {
		dw_draw_texture_fit_h(text, box->t);
	}
}





























