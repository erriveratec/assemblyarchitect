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


SDL_Rect g_big_box;
SDL_Rect g_error_box;
SDL_Rect g_upper_box;
SDL_Rect g_center_box;
SDL_Rect g_lower_box;
SDL_Rect g_ins_box;
SDL_Rect g_code_box;
SDL_Rect g_stagebutton_box;

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

	SDL_Rect r = (pos == 0) ? dm_get_text_box_big() : dm_get_box_msg_wh();
	int h = (pos == 0) ? dm_get_h_big_text() : dm_get_h_msg();

	g_msgs[pos] = dw_new_text_texture_by_h(r.w, h, C_BLACK, msg);
}

/* Function: tx_init_global_msgs
 * -----------------------------------------------------------------------------
 * Creates the textures of global messages that are used across several levels.
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

	SDL_Rect r = dm_get_text_box_big();
	int text_h = dm_get_h_bottom_msg();
	
	g_gbl_msgs[TX_MSG_CLICKANY] = dw_new_text_texture_by_h(r.w, text_h, C_GREY, 
															MSG_CLICKANY);
	g_gbl_msgs[TX_MSG_PRESSPLAY] = dw_new_text_texture_by_h(r.w, text_h, C_GREY, 
															MSG_PRESSPLAY);
	g_gbl_msgs[TX_MSG_PRESSBACK] = dw_new_text_texture_by_h(r.w, text_h, C_GREY, 
															MSG_PRESSBACK);
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

/* Function: tx_init_text_boxes
 * -----------------------------------------------------------------------------
 * Set the properties of the text boxes of the game
 * 
 * Arguments:
 *	level_id: identifie of the level of which the textures will be created
 *	
 * Return:
 *	
 */
void tx_init_text_boxes()
{
	g_big_box = dm_get_text_box_big();
	g_error_box = dm_get_text_box_error();	
	g_upper_box = dm_get_text_box_upper();	
	g_center_box = dm_get_text_box_center();	
	g_lower_box = dm_get_text_box_lower();	
	g_ins_box = dm_get_text_box_ins();
	g_code_box = dm_get_text_box_code();
	g_stagebutton_box = dm_get_text_box_stagebutton();
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
		case TX_STAGEBUTTON_BOX:
			retval = get_box_member(&g_stagebutton_box, member);
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
	switch(pos){
		case TX_INS_BOX:
			b = g_ins_box; 
			b.y = g_ins_box.y + g_ins_box.h*5/6;
			b.h = g_ins_box.h/6;
			text_h = dm_get_h_bottom_msg();
			break;
		case TX_UPPER_BOX:
			b = g_upper_box; 
			b.y = g_upper_box.y + g_upper_box.h*5/6;
			b.h = g_upper_box.h/6;
			text_h = dm_get_h_bottom_msg();
			break;
		case TX_CENTER_BOX:
			b = g_center_box; 
			b.y = g_center_box.y + g_center_box.h*5/6;
			b.h = g_center_box.h/6;
			text_h = dm_get_h_bottom_msg();
			break;
		case TX_LOWER_BOX:
			b = g_lower_box; 
			b.y = g_lower_box.y + g_lower_box.h*5/6;
			b.h = g_lower_box.h/6;
			text_h = dm_get_h_bottom_msg();
			break;
		case TX_CODE_BOX:
			b = g_code_box; 
			text_h = dm_get_h_msg();
			break;
		case TX_STAGEBUTTON_BOX:
			b = g_stagebutton_box; 
			text_h = dm_get_h_msg();
			break;
		case TX_BIG_BOX:
			b = g_big_box;
			b.y = g_big_box.y + g_big_box.h*5/6;
			b.h = g_big_box.h/6;
			text_h = dm_get_h_msg();
			break;
		case TX_ERROR_BOX:
			b = g_error_box; 
			b.y = g_error_box.y + g_error_box.h*5/6;
			b.h = g_error_box.h/6;
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
			b = g_ins_box; 
			text_h = dm_get_h_msg();
			break;
		case TX_UPPER_BOX:
			b = g_upper_box; 
			text_h = dm_get_h_msg();
			break;
		case TX_CENTER_BOX:
			b = g_center_box; 
			text_h = dm_get_h_msg();
			break;
		case TX_LOWER_BOX:
			b = g_lower_box; 
			text_h = dm_get_h_msg();
			break;
		case TX_CODE_BOX:
			b = g_code_box; 
			text_h = dm_get_h_msg();
			break;
		case TX_STAGEBUTTON_BOX:
			b = g_stagebutton_box; 
			text_h = dm_get_h_msg();
			break;
		case TX_BIG_BOX:
			b = g_big_box; 
			text_h = dm_get_h_big_text();
			break;
		case TX_ERROR_BOX:
			b = g_error_box; 
			text_h = dm_get_h_msg();
			break;
	}
	dw_draw_filled_rectangle(b, C_WHITE, C_WHITE);
	dw_draw_wrapped_texture_by_h(b, text_h, a);
}

