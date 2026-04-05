#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include <SDL_mixer.h>
#include"text_tx.h"
#include"draw_dw.h"
#include"instruction_window_iw.h"
#include"code_window_cw.h"
#include"dimensions_dm.h"
#include "sdl_config.h"

#define MSG_CLICKANY "Click Anywhere"
#define MSG_PRESSPLAY "Press the Play Button"
#define MSG_PRESSBACK "Press the Back Button"
#define MSG_PRESSCONT "Press the Continue Button"

static const Uint32 TEXT_H_BOTTOM_MSG = 17;
static const Uint32 TEXT_BOX_H = 360; //1920/5
static const Uint32 TEXT_BOX_W = 384 ;

static const Uint32 BORDER_OFS = 10 ;

char *SYSTEM_MESSAGE = "SYSTEM MESSAGE";
char *SYSTEM_NOTICE = "SYSTEM NOTICE";
char *SYSTEM_WARNING = "SYSTEM WARNING";
char *INSTRUCTION = "INSTRUCTION";

texture_t *g_system_message = NULL;
texture_t *g_system_notice = NULL;
texture_t *g_system_warning = NULL;
texture_t *g_instruction = NULL;

int g_lvl_msgs_size;
texture_array_t **g_lvl_msgs = NULL;

int g_msgs_size;
texture_array_t **g_msgs = NULL;

int g_gbl_msgs_size;
texture_array_t **g_gbl_msgs = NULL;

static int get_box_member(SDL_Rect *box, int member);
static int get_h_bottom_msg();
static SDL_Rect get_text_box_center_up();
static int get_border_ofs();
static SDL_Rect get_text_box_upper();
static SDL_Rect get_text_box_lower();
static SDL_Rect get_text_box_center();
static SDL_Rect get_text_box_ins();
static SDL_Rect get_text_box_code();
static SDL_Rect get_text_box_upper_right();

/* Function: get_border_ofs
 * -----------------------------------------------------------------------------
 * Returns a border space of messages
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	The offset value 
 */
static int get_border_ofs()
{
	return dm_scale_to_res(BORDER_OFS);
}

/* Function: tx_get_text_box_wh
 * -----------------------------------------------------------------------------
 * Return the width and heigth value of the text box
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the dimension of the object
 */
SDL_Rect tx_get_text_box_wh()
{
	SDL_Rect b;
	b.w = dm_scale_to_res(TEXT_BOX_W);
	b.h = dm_scale_to_res(TEXT_BOX_H);
	b.x = 0;
	b.y = 0;
	return b;
}

/* Function: get_text_box_upper_right
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
static SDL_Rect get_text_box_upper_right()
{
	SDL_Rect d = tx_get_text_box_wh();
	SDL_Rect b;
	b.w = d.w;
	b.h = d.h;
	b.x = dm_get_stage_imm_up().x + 12*dm_get_value_box_wh().w;
	b.y = dw_get_ofs_iface_filled_border();	return b;
	
	return b;
}

/* Function: get_text_box_code
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
static SDL_Rect get_text_box_code()
{
	SDL_Rect cb = cw_get_stage_code_box();	
	SDL_Rect d = tx_get_text_box_wh();
	SDL_Rect b;
	b.w = d.w;
	b.h = d.h;
	b.x = cb.x + (cb.w - b.w)/2;
	b.y = cb.y + cb.h - b.h;
	return b;
}
/* Function: get_text_box_ins
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
static SDL_Rect get_text_box_ins()
{

	SDL_Rect ib = dm_get_stage_instruction_box();	
	SDL_Rect d = tx_get_text_box_wh();
	SDL_Rect b;
	b.w = d.w;
	b.h = d.h;
	b.x = ib.x;
	b.y = ib.y + ib.h + dm_get_w_borders();
	return b;
}

/* Function: get_text_box_center
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
static SDL_Rect get_text_box_center()
{
	int w = dm_get_screen_width();	
	int h = dm_get_screen_height();	
	SDL_Rect d = tx_get_text_box_wh();
	
	SDL_Rect b;
	b.w = d.w;
	b.h = d.h;
	b.x = w/2 - b.w/2;
	b.y = h/2 - b.h/2;
	return b;
}

/* Function: get_text_box_lower
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the text box lower
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
static SDL_Rect get_text_box_lower()
{
	int w = dm_get_screen_width();	
	int h = dm_get_screen_height();	
	SDL_Rect d = tx_get_text_box_wh();	
	SDL_Rect b;
	b.w = d.w;
	b.h = d.h;
	b.x = w/2 - b.w/2;
	b.y = h - b.h - get_border_ofs();
	return b;
}
/* Function: get_text_box_upper
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
static SDL_Rect get_text_box_upper()
{
	int w = dm_get_screen_width();	
	SDL_Rect d = tx_get_text_box_wh();
	SDL_Rect b;
	b.w = d.w;
	b.h = d.h;
	b.x = w/2 - b.w/2;
	b.y = get_border_ofs();
	return b;
}

/* Function: get_text_box_center_up
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
static SDL_Rect get_text_box_center_up()
{
	int w = dm_get_screen_width();	
	int h = dm_get_screen_height();	
	SDL_Rect d = tx_get_text_box_wh();
	SDL_Rect b;
	b.w = d.w;
	b.h = d.h;
	b.x = w/2 - b.w/2;
	b.y = h/4;
	return b;
}

/* Function: get_h_bottom_msg
 * -----------------------------------------------------------------------------
 * Returns the h value for the click anywhere message
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset for the sel level buttons
 */
static int get_h_bottom_msg()
{
	int h = dm_scale_to_res(TEXT_H_BOTTOM_MSG);
	return h;
	
}

/* Function: tx_get_message_texture
 * -----------------------------------------------------------------------------
 * Retunr a given message texture loaded from a file
 * 
 * Arguments:
 *	pos: position of the texture that wants to be retrieved
 *	
 * Return:
 * 	texture_array_t pointer of the requested texture
 */
texture_array_t *tx_get_message_texture(int pos)
{
	return g_msgs[pos];
}

/* Function: tx_draw_create_typewriter_text
 * -----------------------------------------------------------------------------
 * This function is used to create the typewriter effect wheen neeeded
 * 
 * Arguments:
 *	t: pointer to the texture t of the texture
 * 	text: text that will be progresively created
 *	
 * Return:
 * 	bool indicating if the writing is complete
 */
bool tx_draw_create_typewriter_text(texture_t **t, 
									SDL_Rect r, 
									char *text, 
									size_t *index,
									SDL_Color color)
{
	bool complete = false;
	size_t full_length = strlen(text);
	if (*index < full_length){
		(*index)++;
		char buf[256];
		size_t n = (*index < sizeof(buf)-1 ? 
					*index : sizeof(buf)-1);
		memcpy(buf, text, n);
		buf[n] = '\0';

		dw_free_texture(*t);
		
		*t = dw_create_text_tex(buf, color);
		dw_draw_texture_fit_h(r, *t);

		if (g_sfx_type && buf[n-1] != ' ') {
			Mix_PlayChannel(-1, g_sfx_type, 0);
		}
	} else if (*index == full_length){
		complete = true;
	}
	return complete;
}

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
 *	h: the height of each line of the message
 *	w: the width of the container of the message
 *	
 * Return:
 * 	Void.	
 */
void tx_set_message_in_array(int pos, char *msg, int w, int h)
{
	assert(pos >= 0 && "Invalid position");
	assert(msg != NULL && "NULL message");	

	//pos--; // THIS WILL EXPLODE LATER

	//int h = (pos == 0) ? dm_get_h_big_text() : dm_get_h_msg();
	//int h = dm_get_h_msg();
//	int w = dm_get_w_msg(dm_get_box_msg_wh());
	g_msgs[pos] = dw_create_text_tex_array_by_h(w, h, C_WHITE, msg);
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

	int text_h = get_h_bottom_msg();
	
	int w = dw_get_iface_content_box(tx_get_text_box_wh()).w;
	g_gbl_msgs[TX_MSG_CLICKANY] = dw_create_text_tex_array_by_h(w, 
														   text_h, 
														   C_SHADOWGREY, 
														   MSG_CLICKANY);
	g_gbl_msgs[TX_MSG_PRESSPLAY] = dw_create_text_tex_array_by_h(w, 
															text_h, 
															C_SHADOWGREY, 
															MSG_PRESSPLAY);
	g_gbl_msgs[TX_MSG_PRESSBACK] = dw_create_text_tex_array_by_h(w, 
															text_h, 
															C_SHADOWGREY, 
															MSG_PRESSBACK);

 	text_h = dw_get_h_iface_header_txt();
	g_system_message = dw_create_text_tex(SYSTEM_MESSAGE, C_GREY);
	g_system_notice = dw_create_text_tex(SYSTEM_NOTICE, C_GREY);
	g_system_warning = dw_create_text_tex(SYSTEM_WARNING, C_GREY);
	g_instruction = dw_create_text_tex(INSTRUCTION, C_GREY);
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
	int offset = dw_get_ofs_iface_filled_border();
	switch(pos){
		case TX_INS_BOX:
			text_h = get_h_bottom_msg();
			b = get_text_box_ins(); 
			b.y += b.h/2 - 2*text_h; //Writes at the center of the box
			break;
		case TX_UPPER_BOX:
			text_h = get_h_bottom_msg();
			b = get_text_box_upper(); 
			b.y += b.h/2 - 2*text_h; //Writes at the center of the box
			break;
		case TX_UPPER_RIGHT_BOX:
			text_h = get_h_bottom_msg();
			b = get_text_box_upper_right(); 
			b.y += b.h/2 - 2*text_h; //Writes at the center of the box
			break;
		case TX_CENTER_BOX:
			text_h = get_h_bottom_msg();
			b = get_text_box_center(); 
			b.y += b.h/2 - 2*text_h; //Writes at the center of the box
			break;
		case TX_LOWER_BOX:
			text_h = get_h_bottom_msg();
			b = get_text_box_lower(); 
			b.y += b.h/2 - 2*text_h; //Writes at the center of the box
			break;
		case TX_CODE_BOX:
			text_h = get_h_bottom_msg();
			b = get_text_box_code(); 
			b.y += b.h/2 - 2*text_h; //Writes at the center of the box
			break;
		case TX_STAGEBUTTON_BOX:
			b = dm_get_text_box_stagebutton(); 
			text_h = dm_get_h_msg();
			b.y += b.h/2 - 2*text_h; //Writes at the center of the box
			break;
		case TX_BIG_BOX:
			text_h = get_h_bottom_msg();
			b = get_text_box_center_up();
			b.y += b.h/2 - 2*text_h; //Writes at the center of the box
			break;
		case TX_ERROR_BOX:
			b = dm_get_text_box_error(); 
			b.y = dm_get_text_box_error().y + dm_get_text_box_error().h*4/6;
			b.h = dm_get_text_box_error().h/6;
			text_h = get_h_bottom_msg();
			break;
	}
	dw_draw_wrapped_texture_by_h(b, text_h, a);
}

/* Function: tx_text_box
 * -----------------------------------------------------------------------------
 * This function displays a message box in differents part of the screen 
 * according to an identifies.
 * Important to notice that the width of the message is defined when te text
 * texture is created.
 *
 * Arguments:
 *	pos: The position id of where is gonna be displayed.
 *	msg_id: The id of the message that will be shown
 *  header: The header that will accompany the text box
 *	
 * Return:
 *	Void.
 */
void tx_text_box(int pos, int msg_id, int header)
{
	assert(msg_id >= 0 && msg_id < g_msgs_size && "Invalid msg_id");
	SDL_Rect b;
	SDL_Rect content;
	int text_h;
	SDL_Color color;
	texture_array_t *a = g_msgs[msg_id];
	switch(pos){
		case TX_INS_BOX:
			b = get_text_box_ins(); 
			content = dw_get_iface_content_box(b);
			text_h = dm_get_h_msg();
			break;
		case TX_UPPER_BOX:
			b = get_text_box_upper(); 
			content = dw_get_iface_content_box(b);
			text_h = dm_get_h_msg();
			break;
		case TX_UPPER_RIGHT_BOX:
			b = get_text_box_upper_right(); 
			content = dw_get_iface_content_box(b);
			text_h = dm_get_h_msg();
			break;
		case TX_CENTER_BOX:
			b = get_text_box_center(); 
			content = dw_get_iface_content_box(b);
			text_h = dm_get_h_msg();
			break;
		case TX_LOWER_BOX:
			b = get_text_box_lower(); 
			content = dw_get_iface_content_box(b);
			text_h = dm_get_h_msg();
			break;
		case TX_CODE_BOX:
			b = get_text_box_code(); 
			content = dw_get_iface_content_box(b);
			text_h = dm_get_h_msg();
			break;
		case TX_STAGEBUTTON_BOX:
			b = dm_get_text_box_stagebutton(); 
			content = dw_get_iface_content_box(b);
			text_h = dm_get_h_msg();
			break;
		case TX_BIG_BOX:
			b = get_text_box_center_up();
			content = dw_get_iface_content_box(b);
			text_h = dm_get_h_big_text();
			break;
		case TX_ERROR_BOX:
			b = dm_get_text_box_error(); 
			content = dw_get_iface_content_box(b);
			text_h = dm_get_h_big_text();
			break;
	}
	texture_t *header_tex = NULL;
	switch(header){
		case TX_NONE:
			header_tex = NULL;
			break;
		case TX_SYSMES:
			header_tex = g_system_message;
			break;
		case TX_SYSNOT:
			header_tex = g_system_notice;
			break;
		case TX_SYSWAR:
			header_tex = g_system_warning;
			break;
		case TX_INS:
			header_tex = g_instruction;
			break;
	}
	dw_draw_iface_box(b, header_tex);
	dw_draw_wrapped_texture_by_h(content, text_h, a);
}

