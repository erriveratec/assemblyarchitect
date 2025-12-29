#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include"text_tx.h"
#include"draw_dw.h"
#include"instruction_window_iw.h"
#include"code_window_cw.h"
#include"dimensions_dm.h"

#define MSG_CLICKANY "Click anywhere to continue"
#define MSG_PRESSPLAY "Press the play button"
#define MSG_PRESSBACK "Press the back button"
#define MSG_PRESSCONT "Press the continue button"

int g_lvl_msgs_size;
texture_array_t **g_lvl_msgs = NULL;

int g_msgs_size;
texture_array_t **g_msgs = NULL;

int g_gbl_msgs_size;
texture_array_t **g_gbl_msgs = NULL;

static int get_box_member(SDL_Rect *box, int member);

/* Function: tx_set_and_allocate_msgs_array
 * -----------------------------------------------------------------------------
 * This function reserves the sapce required of the array of messages that
 * will be used on a level
 * 
 * Arguments:
 *	size: size of the array textures
 *	
 * Return:
 * 	Void.	
 */
void tx_set_and_allocate_msgs_array(int size)
{
	assert(size > 0 && "Negative size");
	g_msgs_size = size;
	g_msgs = malloc(sizeof(texture_array_t*)*size);
}

/* Function: tx_set_message_in_array
 * -----------------------------------------------------------------------------
 * Recives a message read from the file and stores it in the message array
 * on a give position
 * 
 * Arguments:
 *	pos: position in the message array
 *  msg: message that will be set
 *	
 * Return:
 * 	Void.	
 */
void tx_set_message_in_array(int pos, char *msg)
{
	assert(pos > 0 && "Invalid position");
	assert(msg != NULL && "NULL message");	

	pos--; // array starts at zero

	int h = (pos == 0) ? dm_get_h_big_text() : dm_get_h_msg();
	int w = dm_get_w_msg(dm_get_box_msg_wh());
	g_msgs[pos] = dw_new_text_texture_by_h(w, h, C_BLACK, msg);
}

/* Function: tx_init_global_msgs
 * -----------------------------------------------------------------------------
 * Creates the textures of global messages that are used across several levels.
 * This messages are used in the message boxes.
 * 
 * Arguments:
 *	Void.
 *	
 * Return:
 *	Void.	
 */
void tx_init_global_msgs()
{
	g_gbl_msgs_size = 3;
	g_gbl_msgs = malloc(sizeof(texture_array_t*)*g_gbl_msgs_size);

	int text_h = dm_get_h_bottom_msg();
	
	int w = dm_get_w_msg(dm_get_box_msg_wh());
	g_gbl_msgs[TX_MSG_CLICKANY] = dw_new_text_texture_by_h(w, text_h, C_BLACK, 
															MSG_CLICKANY);
	g_gbl_msgs[TX_MSG_PRESSPLAY] = dw_new_text_texture_by_h(w, text_h, 
														C_BLACK, MSG_PRESSPLAY);
	g_gbl_msgs[TX_MSG_PRESSBACK] = dw_new_text_texture_by_h(w, text_h, 
														C_BLACK, MSG_PRESSBACK);
}

/* Function: tx_free_level_text_texture
 * -----------------------------------------------------------------------------
 * Free the level textures when a level is finished
 * 
 * Arguments:
 *	Void.
 *	
 * Return:
 * 	Void.	
 */
void tx_free_level_text_textures()
{
	for (int i = 0; i < g_msgs_size; i++){
		dw_free_texture_array(g_msgs[i]);
	}
	g_msgs_size = 0;
}

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


/* Function: tx_bottom_msg
 * -----------------------------------------------------------------------------
 * Display a message in the bottom of a give box
 *
 * Arguments:
 *	r: The rectangle box where the message will be shown
 *	msg_id: The messages that are displayed at the botttom are global
 *	
 * Return:
 *	Void.
 */
void tx_bottom_msg(int pos, int msg_id)
{
	assert(msg_id >= 0 && msg_id < g_gbl_msgs_size && "Invalid msg_id");
	SDL_Rect b;
	int text_h;
	SDL_Color color;
	texture_array_t *a = g_gbl_msgs[msg_id];
	int offset = dm_get_ofs_iface_filled_border();
	switch(pos){
		case TX_INS_BOX:
			b = dm_get_text_box_ins(); 
			b.y = dm_get_text_box_ins().y + dm_get_text_box_ins().h*4/6;
			b.h = dm_get_text_box_ins().h/6;
			text_h = dm_get_h_bottom_msg();
			break;
		case TX_UPPER_BOX:
			text_h = dm_get_h_bottom_msg();
			b = dm_get_text_box_upper(); 
			b.y += (dm_get_text_box_upper().h/2 - text_h - offset);
			break;
		case TX_CENTER_BOX:
			text_h = dm_get_h_bottom_msg();
			b = dm_get_text_box_center(); 
			b.y += (dm_get_text_box_center().h/2 - text_h - offset);
			break;
		case TX_LOWER_BOX:
			text_h = dm_get_h_bottom_msg();
			b = dm_get_text_box_lower(); 
			b.y += (dm_get_text_box_lower().h/2 - text_h - offset);
			break;
		case TX_CODE_BOX:
			b = dm_get_text_box_code(); 
			text_h = dm_get_h_msg();
			break;
		case TX_STAGEBUTTON_BOX:
			b = dm_get_text_box_stagebutton(); 
			text_h = dm_get_h_msg();
			break;
		case TX_BIG_BOX:
			text_h = dm_get_h_msg();
			int h = a->size*text_h;
			b = dm_get_text_box_big();
			b.y = dm_get_text_box_big().y + dm_get_text_box_big().h/2 - h;
			break;
		case TX_ERROR_BOX:
			b = dm_get_text_box_error(); 
			b.y = dm_get_text_box_error().y + dm_get_text_box_error().h*4/6;
			b.h = dm_get_text_box_error().h/6;
			text_h = dm_get_h_bottom_msg();
			break;
	}
	dw_draw_wrapped_texture_by_h(b, text_h, a);
}

/* Function: tx_text_box
 * -----------------------------------------------------------------------------
 * This function displays a message box in differents part of the screen 
 * according to an identifies
 *
 * Arguments:
 *	pos: The position id of where is gonna be displayed.
 *	msg_id: The id of the message that will be shown
 *  bootom_msg_id: id of the bottom message of the box if any
 *	
 * Return:
 *	Void.
 */
void tx_text_box(int pos, int msg_id)
{
	assert(msg_id >= 0 && msg_id < g_msgs_size && "Invalid msg_id");
	SDL_Rect b;
	int text_h;
	SDL_Color color;
	texture_array_t *a = g_msgs[msg_id];
	switch(pos){
		case TX_INS_BOX:
			b = dm_get_text_box_ins(); 
			text_h = dm_get_h_msg();
			break;
		case TX_UPPER_BOX:
			b = dm_get_text_box_upper(); 
			text_h = dm_get_h_msg();
			break;
		case TX_CENTER_BOX:
			b = dm_get_text_box_center(); 
			text_h = dm_get_h_msg();
			break;
		case TX_LOWER_BOX:
			b = dm_get_text_box_lower(); 
			text_h = dm_get_h_msg();
			break;
		case TX_CODE_BOX:
			b = dm_get_text_box_code(); 
			text_h = dm_get_h_msg();
			break;
		case TX_STAGEBUTTON_BOX:
			b = dm_get_text_box_stagebutton(); 
			text_h = dm_get_h_msg();
			break;
		case TX_BIG_BOX:
			b = dm_get_text_box_big();
			text_h = dm_get_h_big_text();
			break;
		case TX_ERROR_BOX:
			b = dm_get_text_box_error(); 
			text_h = dm_get_h_msg();
			break;
	}
	dw_draw_iface_box(b);
	dw_draw_wrapped_texture_by_h(b, text_h, a);
}

