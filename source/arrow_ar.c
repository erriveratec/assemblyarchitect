#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include"arrow_ar.h"
#include"text_tx.h"
#include"draw_dw.h"
#include"instruction_window_iw.h"
#include"code_window_cw.h"
#include"stage_buttons_sb.h"

texture_t *g_lv_arrow;


arrow_t g_arrow_ins; 
arrow_t g_arrow_code_box;
arrow_t g_arrow_play;
arrow_t g_arrow_code_line;
arrow_t g_arrow_del;
arrow_t g_arrow_op2;
arrow_t g_arrow_error;
arrow_t g_arrow_challenge;
arrow_t g_arrow_ib;
arrow_t g_arrow_ob;


static void initialize_ins_arrow();
static void initialize_code_arrow();
static void initialize_play_arrow();
static void initialize_code_line_arrow();
static void initialize_del_arrow();
static void initialize_op2_arrow();
static void initialize_error_arrow();
static void initialize_challenge_arrow();

/* Function: initialize_ins_arrow
 * -----------------------------------------------------------------------------
 * 
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void initialize_ins_arrow()
{
	g_arrow_ins.box.x = ARROW_INS_X; 
	g_arrow_ins.box.y = tx_get_text_box_member(TX_INS_BOX, MEMBER_Y) - ARROW_H;
	g_arrow_ins.box.w = ARROW_W; 
	g_arrow_ins.box.h = ARROW_H;
	g_arrow_ins.startx = ARROW_INS_X;
	g_arrow_ins.starty = g_arrow_ins.box.y;
	g_arrow_ins.travel = g_arrow_ins.box.y - iw_get_instruction_y_by_pos(
									   iw_get_instruction_list_size() - 1);			
	g_arrow_ins.dir = AR_UP;
	g_arrow_ins.in_place = false;
	g_arrow_ins.visible =  true;
	g_arrow_ins.texture =  g_lv_arrow;
}

/* Function: initialize_code_arrow
 * -----------------------------------------------------------------------------
 * 
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void initialize_code_arrow()
{
	int y = tx_get_text_box_member(TX_INS_BOX, MEMBER_Y) +
			tx_get_text_box_member(TX_INS_BOX, MEMBER_H) + ARROW_H;
			
	g_arrow_code_box.box.x = ARROW_CODE_X;
	g_arrow_code_box.box.y = y;
	g_arrow_code_box.box.w = ARROW_W;
	g_arrow_code_box.box.h = ARROW_H;
	g_arrow_code_box.in_place = false;
	g_arrow_code_box.visible = true;
	g_arrow_code_box.startx = ARROW_CODE_X;
	g_arrow_code_box.starty = g_arrow_code_box.box.y;
	g_arrow_code_box.travel = cw_get_code_box_member(MEMBER_X) - 
										(ARROW_CODE_X + ARROW_W); 
	g_arrow_code_box.dir = AR_RIGHT;
	g_arrow_code_box.texture = g_lv_arrow;
}

/* Function: initialize_play_arrow
 * -----------------------------------------------------------------------------
 * 
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void initialize_play_arrow()
{
	g_arrow_play.box.w = ARROW_W;	
	g_arrow_play.box.h = ARROW_H;
	g_arrow_play.box.x = sb_get_play_button_member(MEMBER_X);
	g_arrow_play.box.y = STAGE_BUTTON_Y - 2*ARROW_H;
	g_arrow_play.in_place = false;
	g_arrow_play.visible = true;
	g_arrow_play.startx = STAGE_BUTTON_X;
	g_arrow_play.starty = STAGE_BUTTON_Y - 2*ARROW_H;	
	g_arrow_play.travel = STAGE_BUTTON_Y - g_arrow_play.starty - ARROW_H;	
	g_arrow_play.dir = AR_DOWN;
	g_arrow_play.texture = g_lv_arrow;
}

/* Function: initialize_code_line_arrow
 * -----------------------------------------------------------------------------
 * 
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void initialize_code_line_arrow()
{
	g_arrow_code_line.box.w = ARROW_W;	
	g_arrow_code_line.box.h = ARROW_H;
	g_arrow_code_line.box.x = cw_get_code_line_x(MOV) + ARROW_W/2;
	g_arrow_code_line.box.y = cw_get_line_y(cw_get_code_list_size()-1) + 
													    CODE_BUTTON_H;
	g_arrow_code_line.in_place = false;
	g_arrow_code_line.visible = true;
	g_arrow_code_line.startx = cw_get_code_line_x(MOV) + ARROW_W/2;
	g_arrow_code_line.starty = g_arrow_code_line.box.y;
	g_arrow_code_line.travel = g_arrow_code_line.starty - 
									   cw_get_line_y(cw_get_code_list_size()-1);	
	g_arrow_code_line.dir = AR_UP;
	g_arrow_code_line.texture = g_lv_arrow;

}

/* Function: initialize_del_arrow
 * -----------------------------------------------------------------------------
 * 
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void initialize_del_arrow()
{
	g_arrow_del.box.x = cw_get_code_box_member(MEMBER_X) +
											   cw_get_code_box_member(MEMBER_W);
	g_arrow_del.box.y = cw_get_text_box_member(MEMBER_Y);
	g_arrow_del.box.w = ARROW_W;
	g_arrow_del.box.h = ARROW_H;
	g_arrow_del.in_place = false;
	g_arrow_del.visible = true;
	g_arrow_del.startx = cw_get_code_box_member(MEMBER_X) + 
										cw_get_code_box_member(MEMBER_W);
	g_arrow_del.starty = cw_get_text_box_member(MEMBER_Y);
	g_arrow_del.travel = ARROW_W;
	g_arrow_del.dir = AR_RIGHT;
	g_arrow_del.texture = g_lv_arrow;
}

/* Function: initialize_op2_arrow
 * -----------------------------------------------------------------------------
 * 
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void initialize_op2_arrow()
{
	g_arrow_op2.box.x = cw_get_code_line_x(MOV) + OP2_X_OFFSET + ARROW_W/2;
	g_arrow_op2.box.y = cw_get_line_y(cw_get_code_list_size()-1) + 
													CODE_BUTTON_H;
	g_arrow_op2.box.w = ARROW_W;
	g_arrow_op2.box.h = ARROW_H;
	g_arrow_op2.in_place = false;
	g_arrow_op2.visible = true;
	g_arrow_op2.startx = cw_get_code_line_x(MOV) + OP2_X_OFFSET + ARROW_W/2;
	g_arrow_op2.starty = g_arrow_op2.box.y;			
	g_arrow_op2.travel = g_arrow_op2.starty	- 
									   cw_get_line_y(cw_get_code_list_size()-1);
	g_arrow_op2.dir = AR_UP;
	g_arrow_op2.texture = g_lv_arrow;
}

/* Function: initialize_op2_arrow
 * -----------------------------------------------------------------------------
 * 
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void initialize_error_arrow()
{
	g_arrow_error.box.x = RES_BOX_X + RES_BOX_W/2 - ARROW_W/2;
	g_arrow_error.box.y = RES_BOX_Y + RES_BOX_H + ERROR_BOX_Y_OFFSET - ARROW_H;	
	g_arrow_error.box.w = ARROW_W;
	g_arrow_error.box.h = ARROW_H;
	g_arrow_error.in_place = false;
	g_arrow_error.visible = true;
	g_arrow_error.startx = RES_BOX_X + RES_BOX_W/2 - BACK_BUTTON_W/2 - 2*ARROW_W;			
	g_arrow_error.starty = RES_BOX_Y + RES_BOX_H + ERROR_BOX_Y_OFFSET - ARROW_H;		
	g_arrow_error.travel = ARROW_W;
	g_arrow_error.dir = AR_UP;
	g_arrow_error.texture = g_lv_arrow;
}


/* Function: initialize_challenge_arrow
 * -----------------------------------------------------------------------------
 * 
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void initialize_challenge_arrow()
{
	g_arrow_challenge.box.x = cw_get_text_box_member(MEMBER_X) +
							  cw_get_text_box_member(MEMBER_W) + 2*ARROW_W;
	g_arrow_challenge.box.y = cw_get_text_box_member(MEMBER_Y) +
							  cw_get_text_box_member(MEMBER_H)/2;
	g_arrow_challenge.box.w = ARROW_W;
	g_arrow_challenge.box.h = ARROW_H;
	g_arrow_challenge.in_place = false;
	g_arrow_challenge.visible = true;
	g_arrow_challenge.startx = cw_get_text_box_member(MEMBER_X) +
							cw_get_text_box_member(MEMBER_W) + 2*ARROW_W;
	g_arrow_challenge.starty = cw_get_text_box_member(MEMBER_Y) +
			cw_get_text_box_member(MEMBER_H)/2;
	g_arrow_challenge.travel = ARROW_W;
	g_arrow_challenge.dir = AR_LEFT;
	g_arrow_challenge.texture = g_lv_arrow;
}

/* Function: initialize_ib_arrow
 * -----------------------------------------------------------------------------
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void initialize_ib_arrow()
{
	SDL_Rect b =  dm_get_stage_input_buffer_box();
	g_arrow_challenge.box.x = cw_get_text_box_member(MEMBER_X) +
							  cw_get_text_box_member(MEMBER_W) + 2*ARROW_W;
	g_arrow_challenge.box.y = cw_get_text_box_member(MEMBER_Y) +
							  cw_get_text_box_member(MEMBER_H)/2;
	g_arrow_challenge.box.w = ARROW_W;
	g_arrow_challenge.box.h = ARROW_H;
	g_arrow_challenge.in_place = false;
	g_arrow_challenge.visible = true;
	g_arrow_challenge.startx = cw_get_text_box_member(MEMBER_X) +
							cw_get_text_box_member(MEMBER_W) + 2*ARROW_W;
	g_arrow_challenge.starty = cw_get_text_box_member(MEMBER_Y) +
			cw_get_text_box_member(MEMBER_H)/2;
	g_arrow_challenge.travel = ARROW_W;
	g_arrow_challenge.dir = AR_LEFT;
	g_arrow_challenge.texture = g_lv_arrow;
}



/* Function: initialize_arrow
 * -----------------------------------------------------------------------------
 * Initialize all the corresponding arrows
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
void ar_initialize_arrows()
{
	initialize_ins_arrow();
	initialize_code_arrow();
	initialize_play_arrow();
	initialize_code_line_arrow();
	initialize_del_arrow();
	initialize_op2_arrow();
	initialize_error_arrow();
	initialize_challenge_arrow();
}

/* Function: display_output_arrow
 * -----------------------------------------------------------------------------
 * Animate with a moving arrow of the output buffer when available for selection
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 *
static void display_output_arrow()
{
	static arrow_t arrow;
	static bool arrow_initialized = false;
	int startx = ARROW_INPUT_X;
	if (arrow_initialized == false){
		arrow.box.x = startx;
		arrow.box.y = ARROW_OUTPUT_Y;
		arrow.box.w = ARROW_W;
		arrow.box.h = ARROW_H;
		arrow.startx = startx;
		arrow.starty = arrow.box.y;
		arrow.dir = AR_RIGHT;
		arrow.travel = BUFFER_BOX_X - startx - ARROW_W;
		arrow.in_place = false;
		arrow_initialized = true;
		arrow.texture = g_input_arrow;
	}
	SDL_SetTextureColorMod(arrow.texture->texture, 0, 255, 255);
	ar_animate_arrow(&arrow);
}

* Function: display_input_arrow_
 * -----------------------------------------------------------------------------
 * Animate with a moving arrow of the input buffer when available for selection
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 *
static void display_input_arrow()
{
	static arrow_t arrow;
	static bool arrow_initialized = false;
	int startx = ARROW_INPUT_X;
	if (arrow_initialized == false){
		arrow.box.x = startx;
		arrow.box.y = ARROW_INPUT_Y;
		arrow.box.w = ARROW_W;
		arrow.box.h = ARROW_H;
		arrow.startx = startx;
		arrow.starty = arrow.box.y;
		arrow.dir = AR_RIGHT;
		arrow.travel = BUFFER_BOX_X - startx - ARROW_W;
		arrow.in_place = false;
		arrow_initialized = true;
		arrow.texture = g_input_arrow;
	}
	SDL_SetTextureColorMod(arrow.texture->texture, 255, 0, 255);
	ar_animate_arrow(&arrow);
}*/

/* Function: display_arrow
 * -----------------------------------------------------------------------------
 * Animates the arrow that points to the first instruction
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
void ar_display_arrow(int arrow_id)
{
	int x;
	int y;
	int travel;
	int dir;
	arrow_t *aptr;
	switch(arrow_id){
		case AR_INS:
			aptr = &g_arrow_ins;
			break;
		case AR_DROP:
			aptr = &g_arrow_code_box;
			break;
		case AR_PLAY:
			aptr = &g_arrow_play;
			break;
		case AR_CODE:
			aptr = &g_arrow_code_line;
			break;
		case AR_DEL:
			aptr = &g_arrow_del;
			break;
		case AR_OP2:
			aptr = &g_arrow_op2;
			break;
		case AR_ERROR:
			aptr = &g_arrow_error;
			break;
		case AR_CHALLENGE:
			aptr = &g_arrow_challenge;
			break;
		default:
			break;
	}
	ar_animate_arrow(aptr);
}

/*	SDL_SetTextureColorMod(arrow.texture->texture, 255, 0, 255);
	dw_animate_arrow(startx, starty, &arrow, AR_UP, travel);
	SDL_SetTextureColorMod(arrow.texture->texture, 255, 255, 255);
*/

/* Function: ar_animate_arrow
 * -----------------------------------------------------------------------------
 * Animates the movement of an arrow object in a given direction
 *
 * Arguments:
 *	x: starting x position.
 *	y: starting y position.
 *	arrow: arrow object that will be animated
 *  dir: direction of the movement of the arrow
 *  travel: how long the animation will move on the screen
 *
 * Return:
 *	Void.
 */
void ar_animate_arrow(arrow_t *arrow) 
{
	assert(arrow !=NULL && "arrow object is NULL");
	
	switch(arrow->dir){
		case AR_UP:
			if (arrow->box.y <= arrow->starty - arrow->travel){
				arrow->in_place = true;	
			} else if (arrow->box.y >= arrow->starty){
				arrow->in_place = false;
			}
			if (arrow->in_place == false){
				arrow->box.y--;
			}else if (arrow->in_place == true){
				arrow->box.y++;
			}
			dw_draw_rotated_texture_fits_h(arrow->box.x, arrow->box.y, 
	     								   arrow->box.h, -90.0, arrow->texture);
			break;

		case AR_DOWN:
			if (arrow->box.y >= arrow->starty + arrow->travel){
				arrow->in_place = true;	
			} else if (arrow->box.y <= arrow->starty){
				arrow->in_place = false;
			}
			if (arrow->in_place == false){
				arrow->box.y++;
			}else if (arrow->in_place == true){
				arrow->box.y--;
			}
			dw_draw_rotated_texture_fits_h(arrow->box.x, arrow->box.y, 
	     								   arrow->box.h, 90.0, arrow->texture);
			break;

		case AR_RIGHT:
			if (arrow->box.x >= arrow->startx + arrow->travel){
				arrow->in_place = true;	
			} else if (arrow->box.x <= arrow->startx){
				arrow->in_place = false;
			}
			if (arrow->in_place == false){
				arrow->box.x++;
			}else if (arrow->in_place == true){
				arrow->box.x--;
			}
			dw_draw_rotated_texture_fits_h(arrow->box.x, arrow->box.y, 
										     arrow->box.h, 0.0, arrow->texture);
			break;
		case AR_LEFT:
			if (arrow->box.x <= arrow->startx - arrow->travel){
				arrow->in_place = true;	
			} else if (arrow->box.x >= arrow->startx){
				arrow->in_place = false;
			}
			if (arrow->in_place == false){
				arrow->box.x--;
			}else if (arrow->in_place == true){
				arrow->box.x++;
			}
			dw_draw_rotated_texture_fits_h(arrow->box.x, arrow->box.y, 
										  arrow->box.h, 180.00, arrow->texture);
			break;

		default:
			break;
	}
	return;
}

