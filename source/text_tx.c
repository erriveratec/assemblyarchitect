#include<stdio.h>
#include<stdlib.h>
#include"text_tx.h"
#include"draw_dw.h"
#include"instruction_window_iw.h"



SDL_Rect g_msg_ins_box = {MSG_INS_BOX_X, MSG_INS_BOX_Y, MSG_BOX_W, MSG_BOX_H};
SDL_Rect g_msg_upper_box = {SCREEN_WIDTH/2 - MSG_BOX_W/2, 
							 SCREEN_HEIGHT/4 - MSG_BOX_H, MSG_BOX_W, MSG_BOX_H};
SDL_Rect g_msg_lower_box = {SCREEN_WIDTH/2 - MSG_BOX_W/2, 
						   SCREEN_HEIGHT*8/9 - MSG_BOX_H, MSG_BOX_W, MSG_BOX_H};
SDL_Rect g_msg_code_box = {CODE_BOX_X + (CODE_BOX_W - MSG_BOX_W)/2, 
						   MSG_INS_BOX_Y, MSG_BOX_W, MSG_BOX_H};
SDL_Rect g_msg_sb_box = {CODE_BOX_X + (CODE_BOX_W - MSG_BOX_W)/2, 
			     (CODE_BOX_Y + CODE_BOX_H) - MSG_BOX_H/2, MSG_BOX_W, MSG_BOX_H};

SDL_Rect g_msg_big_box = {BIG_MSG_BOX_X, BIG_MSG_BOX_Y, BIG_MSG_BOX_W, 
																 BIG_MSG_BOX_H};

SDL_Rect g_msg_error_box = {ERROR_MSG_BOX_X, ERROR_MSG_BOX_Y, MSG_BOX_W, 
																 MSG_BOX_H};

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
void tx_initialize_assets()
{
	//Other assets that need updating accordin to the level
	g_msg_code_box.y = 300; //g_arrow_code_line.box.y + ARROW_H; 
	int ins_list_size = iw_get_instruction_list_size();
	g_msg_ins_box.y = iw_get_instruction_y_by_pos(ins_list_size - 1) + 2*ARROW_H;


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
		case INS_BOX:
			b.x = g_msg_ins_box.x; 
			b.y = g_msg_ins_box.y; 
			b.w = g_msg_ins_box.w; 
			b.h = g_msg_ins_box.h;
			text_h = MSG_TEXT_H;
			color = COLOR_WHITE;
			break;
		case UPPER_BOX:
			b.x = g_msg_upper_box.x; 
			b.y = g_msg_upper_box.y; 
			b.w = g_msg_upper_box.w; 
			b.h = g_msg_upper_box.h;	
			text_h = MSG_TEXT_H;
			color = COLOR_WHITE;
			break;
		case LOWER_BOX:
			b.x = g_msg_lower_box.x; 
			b.y = g_msg_lower_box.y; 
			b.w = g_msg_lower_box.w; 
			b.h = g_msg_lower_box.h;	
			text_h = MSG_TEXT_H;
			color = COLOR_WHITE;
			break;
		case CODE_BOX:
			b.x = g_msg_code_box.x; 
			b.y = g_msg_code_box.y; 
			b.w = g_msg_code_box.w; 
			b.h = g_msg_code_box.h;	
			text_h = MSG_TEXT_H;
			color = COLOR_WHITE;
			break;
		case SB_BOX:
			b.x = g_msg_sb_box.x; 
			b.y = g_msg_sb_box.y; 
			b.w = g_msg_sb_box.w; 
			b.h = g_msg_sb_box.h;	
			text_h = MSG_TEXT_H;
			color = COLOR_WHITE;
			break;
		case BIG_BOX:
			b.x = g_msg_big_box.x; 
			b.y = g_msg_big_box.y; 
			b.w = g_msg_big_box.w; 
			b.h = g_msg_big_box.h;	
			text_h = BIG_MSG_TEXT_H;
			color = COLOR_ORANGE;
			break;
		case ERROR_BOX:
			b.x = g_msg_error_box.x; 
			b.y = g_msg_error_box.y; 
			b.w = g_msg_error_box.w; 
			b.h = g_msg_error_box.h;	
			text_h = MSG_TEXT_H;
			color = COLOR_WHITE;
			break;


	}
	dw_draw_filled_rectangle(b.x, b.y, b.w, b.h, COLOR_BLACK, COLOR_WHITE);
	dw_draw_wrapped_text_fits_height(b.x, b.y, b.w, b.h, text_h, color, msg);
}
