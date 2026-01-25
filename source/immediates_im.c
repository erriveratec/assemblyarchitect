#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "immediates_im.h"
#include "code_line_cl.h"
#include "dimensions_dm.h"
#include "draw_dw.h"
#include "aux.h"
#include "button_bt.h"

#define IMM_TXT "Immediates"
#define TOTAL_IMM 10


texture_t *g_imm_txt = NULL;

static bool g_imm_up = false;

static void init_imm_texture();
static void draw_imm_txt_up();

typedef struct imm_t{
	btn_t *b;
	value_box_t val;
} imm_t;

imm_t g_up_imm[TOTAL_IMM];


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
	operand_t *o = NULL;
	for (int i = IMM_MIN; i < IMM_MAX; i++){
		if (i == op_id){
	   		char *num = ax_number_to_string((i-(IMM_MIN+1))%10);
			texture_t *t = dw_create_text_texture(num, C_WHITE);
			free(num);
			SDL_Rect cb = dm_get_code_button_wh();
			SDL_Rect r = {.x = 0, .y = 0, .w = cb.w, .h = cb.h};
			btn_t *b = bt_create_btn(r, t);
			o = malloc(sizeof(operand_t));
			o->b = b;
			o->id = op_id; 
		} 
	}
			
   return o;
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
		if (bt_ms_rel_btn(g_up_imm[i].b) == true){
	   		char *num = ax_number_to_string(g_up_imm[i].val.value);
			texture_t *t = dw_create_text_texture(num, C_WHITE);
			free(num);
			
			btn_t *b = bt_create_btn(g_up_imm[i].b->r, t);
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
	SDL_Rect vb = dm_get_value_box_wh();
	
	for (int i = 0; i < TOTAL_IMM; i++){
		g_up_imm[i].val.box.x = x;
		g_up_imm[i].val.box.y = y;
		g_up_imm[i].val.box.w = vb.w;
		g_up_imm[i].val.box.h = vb.h;
		g_up_imm[i].val.value = i;
		g_up_imm[i].val.visible_box = true;
		char *num =  ax_number_to_string(i);
		g_up_imm[i].val.t = dw_create_text_texture(num, C_WHITE);
		free(num);
		g_up_imm[i].b = malloc(sizeof(btn_t));
		g_up_imm[i].b->r = g_up_imm[i].val.box;

		if (i == 4){
			y += vb.h;
			x = dm_get_stage_imm_up().x;
		} else {
			x += vb.w;
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

		if (bt_ms_rel_btn(g_up_imm[i].b) == true){
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
	g_imm_txt = dw_create_text_texture(IMM_TXT, C_AMBER);
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
	if (g_imm_up == true){
		draw_imm_txt_up();
		for (int i = 0; i < TOTAL_IMM; i++){
			ax_draw_value_box(&g_up_imm[i].val, C_WHITE);
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
static void draw_imm_txt_up()
{
	SDL_Rect imm_box = dm_get_stage_imm_up();
	SDL_Rect vb = dm_get_value_box_wh();
	int text_h = dm_get_h_stage_elements_titles();
	int text_w = get_text_width_fits_height(text_h, IMM_TXT);
	int x = imm_box.x + (5*vb.w - text_w)/2;
	int y = 2*vb.h;

	SDL_Rect r = {.x = x, .y = y, .h = text_h};
	dw_draw_texture_fits_height(r, g_imm_txt);
}





























