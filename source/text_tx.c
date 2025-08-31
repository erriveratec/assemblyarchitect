#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include"text_tx.h"
#include"draw_dw.h"
#include"instruction_window_iw.h"
#include"code_window_cw.h"
#include"dimensions_dm.h"


SDL_Rect g_ins_box = {MSG_INS_BOX_X, MSG_INS_BOX_Y, MSG_BOX_W, MSG_BOX_H};
SDL_Rect g_upper_box = {SCREEN_WIDTH/2 - MSG_BOX_W/2, 
							 SCREEN_HEIGHT/4 - MSG_BOX_H, MSG_BOX_W, MSG_BOX_H};
SDL_Rect g_lower_box = {SCREEN_WIDTH/2 - MSG_BOX_W/2, 
						   SCREEN_HEIGHT*8/9 - MSG_BOX_H, MSG_BOX_W, MSG_BOX_H};
SDL_Rect g_code_box = {CODE_BOX_X + (CODE_BOX_W - MSG_BOX_W)/2, 
						   MSG_INS_BOX_Y, MSG_BOX_W, MSG_BOX_H};
SDL_Rect g_sb_box = {CODE_BOX_X + (CODE_BOX_W - MSG_BOX_W)/2, 
			     (CODE_BOX_Y + CODE_BOX_H) - MSG_BOX_H/2, MSG_BOX_W, MSG_BOX_H};
SDL_Rect g_big_box = {BIG_MSG_BOX_X, BIG_MSG_BOX_Y, BIG_MSG_BOX_W, 
																 BIG_MSG_BOX_H};
SDL_Rect g_error_box = {ERROR_MSG_BOX_X, ERROR_MSG_BOX_Y, MSG_BOX_W, 
															 		 MSG_BOX_H};


static int get_box_member(SDL_Rect *box, int member);

/* Function: get_box_member
 * -----------------------------------------------------------------------------
 * Returns the property member of a given text box by a pointer to the bos
 * 
 * Arguments:
 *	box: pointer of the box that the member will be recovered
 *	member: the member of the box that will be recovered.
 *	
 * Return:
 *	member of the box recovered.
 */
static int get_box_member(SDL_Rect *box, int member){
	assert(box != NULL && "Box pointer is NULL");
	assert(member > MEMBER_MIN && member < MEMBER_MAX && 
		   "Invalid member");
	int retval;
	switch(member){
		case MEMBER_X:
			retval = box->x;	
			break;
		case MEMBER_Y:
			retval = box->y;
			break;
		case MEMBER_W:
			retval = box->w;
			break;
		case MEMBER_H:
			retval = box->h;
			break;
	}
	return retval;
}


/* Function: tx_get_text_box_member
 * -----------------------------------------------------------------------------
 * Returns the property member of a given text box
 * 
 * Arguments:
 *	Void.
 *	
 * Return:
 *	Void.
 */
int tx_get_text_box_member(int text_box_id, int member)
{
	assert(text_box_id > TX_BOX_MIN && text_box_id < TX_BOX_MAX && 
		   "Invalid text box");
	assert(member > MEMBER_MIN && member < MEMBER_MAX && 
		   "Invalid member");
	int retval;
	switch(text_box_id){
		case TX_INS_BOX:
			retval = get_box_member(&g_ins_box, member);
			break;
		case TX_UPPER_BOX:
			retval = get_box_member(&g_upper_box, member);
			break;
		case TX_LOWER_BOX:
			retval = get_box_member(&g_lower_box, member);
			break;
		case TX_CODE_BOX:
			retval = get_box_member(&g_code_box, member);
			break;
		case TX_SB_BOX:
			retval = get_box_member(&g_sb_box, member);
			break;
		case TX_BIG_BOX:
			retval = get_box_member(&g_big_box, member);
			break;
		case TX_ERROR_BOX:
			retval = get_box_member(&g_error_box, member);
			break;
		}
	return retval;
}

/* Function: tx_initialize_assets
 * -----------------------------------------------------------------------------
 * This function initializes the assets regarding the text box.
 * 
 *
 * Arguments:
 *	Void.
 *	
 * Return:
 *	Void.
 */
void tx_update_assets()
{
	int code_size =  cw_get_code_list_size();
	//g_code_box.y = cw_get_line_y(code_size-1) + CODE_BUTTON_H + ARROW_H;   
	g_code_box.y = cw_get_line_y(2) + CODE_BUTTON_H + ARROW_H;   
	int ins_list_size = iw_get_instruction_list_size();
	//g_ins_box.y = iw_get_instruction_y_by_pos(ins_list_size - 1) + 3*ARROW_H;
	g_ins_box.y = cw_get_line_y(2) + 3*ARROW_H;

}

/* Function: tx_text_box
 * -----------------------------------------------------------------------------
 * This function displays a message box in differents part of the screen 
 * according to an identifies
 *
 * Arguments:
 * 	msg: The message id of what is going to be displayed.
 *	pos: The position id of where is gonna be displayed.
 *	
 * Return:
 *	Void.
 */
void tx_text_box(int pos, char *msg)
{
	SDL_Rect b;
	int text_h;
	SDL_Color color;
	switch(pos){
		case TX_INS_BOX:
			b.x = g_ins_box.x; 
			b.y = g_ins_box.y; 
			b.w = g_ins_box.w; 
			b.h = g_ins_box.h;
			text_h = MSG_TEXT_H;
			color = C_WHITE;
			break;
		case TX_UPPER_BOX:
			b.x = g_upper_box.x; 
			b.y = g_upper_box.y; 
			b.w = g_upper_box.w; 
			b.h = g_upper_box.h;	
			text_h = MSG_TEXT_H;
			color = C_WHITE;
			break;
		case TX_LOWER_BOX:
			b.x = g_lower_box.x; 
			b.y = g_lower_box.y; 
			b.w = g_lower_box.w; 
			b.h = g_lower_box.h;	
			text_h = MSG_TEXT_H;
			color = C_WHITE;
			break;
		case TX_CODE_BOX:
			b.x = g_code_box.x; 
			b.y = g_code_box.y; 
			b.w = g_code_box.w; 
			b.h = g_code_box.h;	
			text_h = MSG_TEXT_H;
			color = C_WHITE;
			break;
		case TX_SB_BOX:
			b.x = g_sb_box.x; 
			b.y = g_sb_box.y; 
			b.w = g_sb_box.w; 
			b.h = g_sb_box.h;	
			text_h = MSG_TEXT_H;
			color = C_WHITE;
			break;
		case TX_BIG_BOX:
			b.x = g_big_box.x; 
			b.y = g_big_box.y; 
			b.w = g_big_box.w; 
			b.h = g_big_box.h;	
			text_h = BIG_MSG_TEXT_H;
			color = C_BLACK;
			break;
		case TX_ERROR_BOX:
			b.x = g_error_box.x; 
			b.y = g_error_box.y; 
			b.w = g_error_box.w; 
			b.h = g_error_box.h;	
			text_h = MSG_TEXT_H;
			color = C_WHITE;
			break;
	}
	dw_draw_filled_rectangle(b.x, b.y, b.w, b.h, C_WHITE, C_WHITE);
	dw_draw_wrapped_text_fits_height(b.x, b.y, b.w, b.h, text_h, color, msg);
	int tw = get_text_width_fits_height(CLICK_MESSAGE_H, MSG_CLICK_ANYWHERE);
	dw_draw_text_fits_height(b.x + b.w/2 - tw/2, b.y+b.h - 2*CLICK_MESSAGE_H, 
						     CLICK_MESSAGE_H, C_GREY, MSG_CLICK_ANYWHERE);
}






































