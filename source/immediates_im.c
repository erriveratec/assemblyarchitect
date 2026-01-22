#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "immediates_im.h"
#include "dimensions_dm.h"
#include "draw_dw.h"
#include "aux.h"

#define IMM_TXT "Immediates"

texture_t *g_imm_txt = NULL;

static bool g_imm_up = false;

static void draw_imm_txt_up();



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

/* Function: im_init_imm_texture
 *------------------------------------------------------------------------------
 * Creates the instructions texture of the immediate text
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Void.
 */
void im_init_imm_texture()
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
	int text_h = dm_get_h_stage_elements_titles();
	int text_w = get_text_width_fits_height(text_h, IMM_TXT);
	int x = imm_box.x;
	int y = imm_box.y;

	SDL_Rect r = {.x = x, .y = y, .h = text_h};
	dw_draw_texture_fits_height(r, g_imm_txt);
}
