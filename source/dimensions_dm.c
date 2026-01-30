#include<stdio.h>
#include<stdlib.h>
#include"dimensions_dm.h"
#include"registers_rg.h"
#include <SDL.h>

#define ESC_MENU_BOX_W 600
#define ESC_MENU_BOX_H 320

#define RES_BOX_W 450
#define RES_BOX_H 270
#define RES_BOX_OFFSET 35

#define TEXT_BOX_LOWER_X 675
#define TEXT_BOX_LOWER_Y 565

#define MSG_BOX_W 270
#define MSG_BOX_H 270

// This is a new version of the dimensions set by a ration of the resolution

#define VALUE_BOX_H 40
#define VALUE_BOX_W 50
#define REG_BOX_OFS 25

#define ARROW_H 30
#define ARROW_W 30

#define AVATAR_W 50
#define AVATAR_H 50

#define TEXT_H_BIG 50
#define TEXT_H_BOTTOM_MSG 17
#define TEXT_H_MSG 25
#define TEXT_H_ERROR_MSG 35
#define TEXT_H_TOTAL_MSG 160
#define TEXT_H_STAGE_ELEMENTS_TITLES 25
#define TEXT_H_STAGE_TITLES 70
#define TEXT_H_CODE 40

#define STAGE_BUTTON_W 60
#define STAGE_BUTTON_H 60
#define STAGE_BUTTON_Y 820

#define P_BUTTON_H 200
#define P_BUTTON_W 200

#define CODE_BUTTON_W 90
#define CODE_BUTTON_H 40
#define COMMA_OFS 15

#define ESC_MENU_BUTTON_W 350
#define ESC_MENU_BUTTON_H 60

#define RST_MENU_BUTTON_W 350
#define RST_MENU_BUTTON_H 60

#define SEL_LEVEL_BUTTON_W 130
#define SEL_LEVEL_BUTTON_H 60

#define RET_BUTTON_W 90
#define RET_BUTTON_H 75

#define RET_RES_OFS 5

#define RST_BTN_W 90
#define RST_BTN_H 75

#define RST_MENU_BTNS_W 100
#define RST_MENU_BTNS_H 60

#define RES_BUTTON_H 60
#define RES_BACK_BUTTON_W 100
#define RES_CONT_BUTTON_W 200

#define RAIL_W 4
#define RAIL_END_W 16

// Padding of the text borders
#define W_PADDING 12
#define H_PADDING 10
#define BUT_PADDING 25

#define IFACE_BUTTON_BORDER_W 5 // Used for the buttons
#define IFACE_FILLED_OFS 20 // Used for Interface Messages
#define BUTTON_SHADOW_OFS 10 // The shadow of iface buttons
#define SCREEN_BORDERS_OFS 2

#define BORDERS_WIDTH 5 //Used for the interface boxes

// Intro screens
#define TITLE_W 1000
#define TITLE_H	140
#define TITLE_Y	80

#define TITLE_IMG_H 475
#define TITLE_IMG_W 475
#define TITLE_IMG_Y 275

#define PRESS_W 1000
#define PRESS_H 60
#define PRESS_Y 750

#define STUDIO_W 600
#define STUDIO_H 600

#define INS_BOX_X 0
#define INS_BOX_W 170
#define INS_BOX_H 225
#define INS_BOX_Y 225

#define CODE_BOX_W 350
#define CODE_BOX_Y 50

#define REG_BOX_W 250
#define REG_BOX_H 400
#define REG_BOX_Y 0

#define IMM_TXT_UP_Y 0

#define BTN_ANIM_MAX 15
#define BTN_ANIM_DELTA 1
#define BTN_ANIM_SPEED 1 //Lower is faster

#define MISS_OP_W 3

#define BUF_X 1400
#define BUF_H 75
#define BUF_W 1000




int g_res_id;
int g_screen_width;
int g_screen_height;

/* Function: dm_get_w_miss_op
 * -----------------------------------------------------------------------------
 * Returns the width that is going to be used for the missing op animation
 * square.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	The max level of the animation
 */
int dm_get_w_miss_op()
{
	return dm_scale_to_resolution(MISS_OP_W);
}

/* Function: dm_get_btn_anim_max
 * -----------------------------------------------------------------------------
 *	Return the max level of the button animation
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	The max level of the animation
 */
int dm_get_btn_anim_max()
{
	return dm_scale_to_resolution(BTN_ANIM_MAX);
}

/* Function: dm_get_btn_anim_delta
 * -----------------------------------------------------------------------------
 *	Return the delta change of the button animation
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	The delta level  of the animation
 */
int dm_get_btn_anim_delta()
{
	return dm_scale_to_resolution(BTN_ANIM_DELTA);
}

/* Function: dm_get_btn_anim_speed
 * -----------------------------------------------------------------------------
 * Returns the speed level of the animation 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	The speed level of the animation
 */
int dm_get_btn_anim_speed()
{
	return BTN_ANIM_SPEED;
}


/* Function: dm_get_h_between_code
 * -----------------------------------------------------------------------------
 *	Returns the ammount of space that will be between instructions
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Instructions sapes
 */
int dm_get_h_between_code()
{
	return dm_get_code_button_wh().h + dm_get_h_padding();
}

/* Function: dm_get_w_borders
 * -----------------------------------------------------------------------------
 *	Returns the width of the borders of each section of the game
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	The of the borders 
 */
int dm_get_w_borders()
{
	return dm_scale_to_resolution(BORDERS_WIDTH);
}

/* Function: dm_get_ofs_button_shadow
 * -----------------------------------------------------------------------------
 *	Returns the shadow offset for the buttons
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	The offset that will be used for the shadow of the buttons
 */
int dm_get_ofs_button_shadow()
{
	return dm_scale_to_resolution(BUTTON_SHADOW_OFS);
}

/* Function: dm_get_ofs_iface_filled_border
 * -----------------------------------------------------------------------------
 *	Returns the interface filled border used for messages.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	The offset that will be use of the rectangle padding of the interface
 */
int dm_get_ofs_iface_filled_border()
{
	return dm_scale_to_resolution(IFACE_FILLED_OFS);
}

/* Function: dm_get_ofs_iface_border
 * -----------------------------------------------------------------------------
 *	Returns the interfacer border space that will be use for interface buttons
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	The offset that will be use of the rectangle padding of the interface
 */
int dm_get_ofs_iface_border()
{
	return dm_scale_to_resolution(IFACE_BUTTON_BORDER_W);
}

/* Function: dm_get_screen_height
 * -----------------------------------------------------------------------------
 *	Returns the screen width
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	screen width
 */
int dm_get_screen_height()
{
	return g_screen_height;
}

/* Function: dm_get_screen_width
 * -----------------------------------------------------------------------------
 *	Returns the screen width
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	screen width
 */
int dm_get_screen_width()
{
	return g_screen_width;
}

/* Function: dm_get_ofs_space_stage_buttons
 * -----------------------------------------------------------------------------
 *	Return the offset value of the contents of the buffer. 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_ofs_space_stage_buttons()
{
	SDL_Rect sb = dm_get_stage_btns();
	return sb.w/2;
}

/* Function: dm_get_ofs_reg_value_box
 * -----------------------------------------------------------------------------
 *	Return the offset value of the contents of the buffer. 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_ofs_reg_value_box()
{
	SDL_Rect vbox = dm_get_value_box_wh();
	return vbox.h/4;
}

/* Function: dm_get_ofs_buffer_value_box
 * -----------------------------------------------------------------------------
 *	Return the offset value of the contents of the buffer. 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_ofs_between_value_box()
{
	int ofs =  dm_get_ofs_buffer_value_box();
	return (ofs*3/2);
}

/* Function: dm_get_ofs_buffer_value_box
 * -----------------------------------------------------------------------------
 *	Return the offset value of the contents of the buffer. 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_ofs_buffer_value_box()
{
	SDL_Rect vbox =  dm_get_value_box_wh();
	SDL_Rect ib = dm_get_stage_input_buffer_box();
	int ofs = (ib.h - vbox.h)/2;
	return ofs;
}

/* Function: dm_get_w_code_box_text
 * -----------------------------------------------------------------------------
 *	Return the offset value of the contents of the buffer. 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_w_code_box_text()
{
	int ofs = dm_get_w_border_padding();
	SDL_Rect cb = dm_get_stage_code_box();
	int w = cb.w - 2*ofs;
	return w;
}

/* Function: dm_get_w_border_padding
 * -----------------------------------------------------------------------------
 *	Return the value of the padding
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_w_border_padding()
{
	return dm_scale_to_resolution(W_PADDING) + dm_get_w_borders();
}

/* Function: dm_get_h_border_padding
 * -----------------------------------------------------------------------------
 *	Returns the scale value for the vertical padding
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_h_border_padding()
{
	return dm_scale_to_resolution(H_PADDING) + dm_get_w_borders();
}

/* Function: dm_get_w_padding
 * -----------------------------------------------------------------------------
 *	Return the value of the padding
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_w_padding()
{
	return dm_scale_to_resolution(W_PADDING);
}

/* Function: dm_get_h_padding
 * -----------------------------------------------------------------------------
 *	Returns the scale value for the vertical padding
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_h_padding()
{
	return dm_scale_to_resolution(H_PADDING);
}

/* Function: dm_get_w_button_padding
 * -----------------------------------------------------------------------------
 *	Returns the scale value for the horizontal padding
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_w_button_padding()
{
	return dm_scale_to_resolution(W_PADDING) + 
	       dm_scale_to_resolution(IFACE_BUTTON_BORDER_W);
}

/* Function: dm_get_h_but_padding
 * -----------------------------------------------------------------------------
 *	Returns the scale value for the vertical padding
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_h_button_padding()
{
	return dm_scale_to_resolution(H_PADDING) + 
		   dm_scale_to_resolution(IFACE_BUTTON_BORDER_W);
}


/* Function: dm_get_h_stage_titles
 * -----------------------------------------------------------------------------
 *	Return the offset value of the contents of the buffer. 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_h_stage_titles()
{
	int h = dm_scale_to_resolution(TEXT_H_STAGE_TITLES);
	return h;
}

/* Function: dm_get_h_stage_titles
 * -----------------------------------------------------------------------------
 *	Return the offset value of the contents of the buffer. 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_h_stage_elements_titles()
{
	int h = dm_scale_to_resolution(TEXT_H_STAGE_ELEMENTS_TITLES);
	return h;
	
}

/* Function: dm_get_ofs_comma
 * -----------------------------------------------------------------------------
 *	Return the offset value for the comma that is draw in instructions
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_ofs_code_comma()
{
	int ofs = dm_get_ofs_code_op2() - dm_scale_to_resolution(COMMA_OFS);
	return ofs;
}

/* Function: dm_get_ofs_code_op1
 * -----------------------------------------------------------------------------
 *	Return the offset value of the contents of the buffer. 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_ofs_code_op1()
{
	SDL_Rect cb = dm_get_code_button_wh();
	int ofs = cb.w;
	return ofs;
}

/* Function: dm_get_ofs_code_op2
 * -----------------------------------------------------------------------------
 *	Return the offset value of the contents of the buffer. 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_ofs_code_op2()
{
	SDL_Rect cb = dm_get_code_button_wh();
	int ofs = 2*cb.w + dm_get_w_padding();
	return ofs;
}
/* Function: dm_get_ofs_reg_box
 * -----------------------------------------------------------------------------
 *	Return the offset value of the contents of the buffer. 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_ofs_stage_reg_box()
{
	int offset = dm_scale_to_resolution(REG_BOX_OFS);
	return offset;
	
}

/* Function: dm_get_y_hidden_stage_buttons
 * -----------------------------------------------------------------------------
 * Returns the hidden y value for the stage buttons
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset for the sel level buttons
 */
int dm_get_y_hidden_stage_buttons()
{
	return g_screen_height + 150;
	
}

/* Function: dm_get_w_stage_rail
 * -----------------------------------------------------------------------------
 * Returns the w value of the stage rail
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset for the sel level buttons
 */
int dm_get_w_stage_rail()
{
	int w = dm_scale_to_resolution(RAIL_W);
	return w;
	
}

/* Function: dm_get_w_stage_rail_end
 * -----------------------------------------------------------------------------
 * Returns the w value for the ends of the stage raisl
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset for the sel level buttons
 */
int dm_get_w_stage_rail_end()
{
	int w = dm_scale_to_resolution(RAIL_END_W);
	return w;
	
}
/* Function: dm_get_value_box_val_wh
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object. 
 * 
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_value_box_val_wh()
{
	SDL_Rect vb = dm_get_value_box_wh();
	SDL_Rect b;
	b.w = vb.w;
	b.h = vb.h - vb.h/10;
	b.x = 0;
	b.y = 0;
	return b;
}
/* Function: dm_get_value_box_wh
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object. 
 * 
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_value_box_wh()
{
	SDL_Rect b;
	b.w = dm_scale_to_resolution(VALUE_BOX_W);
	b.h = dm_scale_to_resolution(VALUE_BOX_H);
	b.x = 0;
	b.y = 0;
	return b;
}

/* Function: dm_get_arrow_wh
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object. 
 * 
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_arrow_wh()
{
	SDL_Rect b;
	b.w = dm_scale_to_resolution(ARROW_W);
	b.h = dm_scale_to_resolution(ARROW_H);
	b.x = 0;
	b.y = 0;
	return b;
}

/* Function: dm_get_text_box_result_but3
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object. This is for the text of the
 * result box.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_text_box_result_but3()
{
	int cont_w = dm_scale_to_resolution(RES_CONT_BUTTON_W);
	int offset = dm_scale_to_resolution(RES_BOX_OFFSET);
	SDL_Rect rb = dm_get_text_box_result();
	SDL_Rect b;
	b.w = dm_scale_to_resolution(RES_BACK_BUTTON_W);
	b.h = dm_scale_to_resolution(RES_BUTTON_H);
	b.x = rb.x + (rb.w - b.w)/2;
	b.y = rb.y + rb.h - b.h - offset;
	return b;
}

/* Function: dm_get_text_box_result_but2
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object. This is for the text of the
 * result box.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_text_box_result_but2()
{
	int back_w = dm_scale_to_resolution(RES_BACK_BUTTON_W);
	int cont_w = dm_scale_to_resolution(RES_CONT_BUTTON_W);
	int offset = dm_scale_to_resolution(RES_BOX_OFFSET);
	SDL_Rect rb = dm_get_text_box_result();
	SDL_Rect b;
	b.w = dm_scale_to_resolution(RES_CONT_BUTTON_W);
	b.h = dm_scale_to_resolution(RES_BUTTON_H);
	b.x = rb.x + back_w + 2*(rb.w - (back_w + cont_w))/3 ;
	b.y = rb.y + rb.h - b.h - offset;
	return b;
}

/* Function: dm_get_text_box_result_but1
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object. This is for the text of the
 * result box.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_text_box_result_but1()
{
	int cont_w = dm_scale_to_resolution(RES_CONT_BUTTON_W);
	int offset = dm_scale_to_resolution(RES_BOX_OFFSET);
	SDL_Rect rb = dm_get_text_box_result();
	SDL_Rect b;
	b.w = dm_scale_to_resolution(RES_BACK_BUTTON_W);
	b.h = dm_scale_to_resolution(RES_BUTTON_H);
	b.x = rb.x + (rb.w - (b.w + cont_w))/3;
	b.y = rb.y + rb.h - b.h - offset;
	return b;
}

/* Function: dm_get_text_box_result_text
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object. This is for the text of the
 * result box .
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_text_box_result_text()
{
	SDL_Rect rb = dm_get_text_box_result();
	SDL_Rect b;
	b.w = rb.w - 2*dm_scale_to_resolution(IFACE_FILLED_OFS);
	b.h = dm_scale_to_resolution(TEXT_H_TOTAL_MSG);
	b.x = rb.x + (rb.w - b.w)/2;
	b.y = rb.y + dm_scale_to_resolution(IFACE_FILLED_OFS);
	return b;
}


/* Function: dm_get_text_box_result
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_text_box_result()
{
	SDL_Rect b;
	b.w = dm_scale_to_resolution(RES_BOX_W);
	b.h = dm_scale_to_resolution(RES_BOX_H);
	b.x = dm_get_screen_width()/2 - b.w/2;
	b.y = dm_get_screen_height() - b.h -
		  dm_scale_to_resolution(IFACE_FILLED_OFS);
	return b;
}


/* Function: dm_get_box_msg_wh
 * -----------------------------------------------------------------------------
 * Returns the box dimensions of the box message
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_box_msg_wh()
{
	SDL_Rect b;
	b.w = dm_scale_to_resolution(MSG_BOX_W);
	b.h = dm_scale_to_resolution(MSG_BOX_H);
	b.x = 0;
	b.y = 0;
	return b;
}

/* Function: dm_get_text_box_error
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_text_box_error()
{
	SDL_Rect rb = dm_get_text_box_result();
	SDL_Rect mb = dm_get_box_msg_wh();
	SDL_Rect ab = dm_get_arrow_wh();
	
	SDL_Rect b;
	b.w = rb.w*2/3;
	b.h = mb.h;
	b.x = rb.x + (rb.w/3)/2;
	b.y = rb.y + rb.h + 2*ab.h;
	return b;
}

/* Function: dm_get_text_box_lower
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the text box lower
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_text_box_lower()
{
	SDL_Rect mb = dm_get_box_msg_wh();	
	SDL_Rect b;
	b.w = mb.w;
	b.h = mb.h;
	b.x = dm_get_screen_width()/2 - mb.w/2;
	b.y = dm_get_screen_height() - mb.h - 
		  dm_scale_to_resolution(IFACE_FILLED_OFS);
	return b;
}

/* Function: dm_get_text_box_big
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_text_box_big()
{
	SDL_Rect b;
	b.w = g_screen_width/5;
	b.h = g_screen_height/3;
	b.x = g_screen_width/2 - b.w/2;
	b.y = g_screen_height/4;
	return b;
}

/* Function: dm_get_text_box_stagebutton
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_text_box_stagebutton()
{
	SDL_Rect cb = dm_get_stage_code_box();	
	SDL_Rect mb = dm_get_box_msg_wh();	
	SDL_Rect b;
	b.w = mb.w;
	b.h = mb.h;
	b.x = cb.x + (cb.w - mb.w)/2;
	b.y = (cb.y + cb.h) - mb.h/2;
	return b;
}

/* Function: dm_get_text_box_code
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_text_box_code()
{
	SDL_Rect cb = dm_get_stage_code_box();	
	SDL_Rect mb = dm_get_box_msg_wh();	
	SDL_Rect b;
	b.w = mb.w;
	b.h = mb.h;
	b.x = cb.x + (cb.w - mb.w)/2;
	b.y = cb.y + cb.h - mb.h - 2*dm_get_w_borders();
	return b;
}
/* Function: dm_get_text_box_ins
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_text_box_ins()
{

	SDL_Rect ib = dm_get_stage_instruction_box();	
	SDL_Rect mb = dm_get_box_msg_wh();	
	SDL_Rect b;
	b.w = mb.w;
	b.h = mb.h;
	b.x = ib.x;
	b.y = ib.y + ib.h + dm_get_w_borders();
	return b;
}



/* Function: dm_get_text_box_center
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_text_box_center()
{
	SDL_Rect mb = dm_get_box_msg_wh();	
	SDL_Rect ib = dm_get_stage_input_buffer_box();	
	int width = dm_get_screen_width();
	SDL_Rect b;
	b.w = mb.w;
	b.h = mb.h;
	b.x = width/2 - mb.w/2;
	b.y = g_screen_height/2 - mb.h/2;
	return b;
}

/* Function: dm_get_text_box_upper
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_text_box_upper()
{
	SDL_Rect mb = dm_get_box_msg_wh();	
	SDL_Rect b;
	b.w = mb.w;
	b.h = mb.h;
	b.x = g_screen_width/2 - mb.w/2;
	b.y = dm_scale_to_resolution(IFACE_FILLED_OFS);
	return b;
}


/* Function: dm_get_stage_ib_text_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_stage_ib_text_box()
{
	SDL_Rect ib = dm_get_stage_input_buffer_box();
	SDL_Rect b;
	b.w = 0;
	b.h = dm_get_h_code_text();
	b.x = ib.x;
	b.y = 0;
	return b;
}

/* Function: dm_get_stage_ob_text_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_stage_ob_text_box()
{
	SDL_Rect ob = dm_get_stage_output_buffer_box();
	SDL_Rect b;
	b.w = 0;
	b.h = dm_get_h_code_text();
	b.x = ob.x;
	b.y = g_screen_height - b.h;
	return b;
}

/* Function: dm_get_stage_input_buffer_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_stage_input_buffer_box()
{
	SDL_Rect b;
	b.w = dm_scale_to_resolution(BUF_W);
	b.h = dm_scale_to_resolution(BUF_H);
	b.x = dm_scale_to_resolution(BUF_X);
	b.y = dm_get_h_code_text();

	return b;
}

/* Function: dm_get_stage_output_buffer_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_stage_output_buffer_box()
{
	SDL_Rect ib = dm_get_stage_ib_text_box();
	SDL_Rect b;
	b.w = dm_scale_to_resolution(BUF_W);
	b.h = dm_scale_to_resolution(BUF_H);
	b.x = dm_scale_to_resolution(BUF_X);
	b.y = g_screen_height - ib.h - b.h;
	return b;
}

/* Function: dm_get_stage_instruction_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the instruction box, x and y are initialize 
 * at 0. Instruction box is where the player selects instructions from.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_stage_instruction_box()
{
	SDL_Rect b;
	b.w = dm_scale_to_resolution(INS_BOX_W);
	b.h = dm_scale_to_resolution(INS_BOX_H);
	b.x = dm_scale_to_resolution(INS_BOX_X);
	b.y = dm_scale_to_resolution(INS_BOX_Y);
	return b;
}

/* Function: dm_get_code_stage_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_stage_code_box()
{
	SDL_Rect ib = dm_get_stage_instruction_box();
	SDL_Rect sb = dm_get_stage_btns();
	SDL_Rect b;
	
	b.x = ib.x + ib.w;
	b.y = dm_scale_to_resolution(CODE_BOX_Y);
	b.w = dm_scale_to_resolution(CODE_BOX_W);
	b.h = sb.y - 2*b.y;
	return b;
}

/* Function: dm_get_stage_ibox
 * -----------------------------------------------------------------------------
 * Returns the box for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_stage_ibox()
{
	SDL_Rect vb = dm_get_value_box_wh();
	SDL_Rect rb = rg_get_register_box();
	SDL_Rect rbi = dm_get_stage_reg_box();
	SDL_Rect b;
	b.w = vb.w;
	b.h = vb.h;
	b.x = rbi.x + rbi.w - b.w - b.w/5;
	b.y = rb.y - b.h - b.h/4;
	return b;
}

/* Function: dm_get_stage_obox
 * -----------------------------------------------------------------------------
 * Returns the box for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_stage_obox()
{
	SDL_Rect vb = dm_get_value_box_wh();
	SDL_Rect rb = rg_get_register_box();
	SDL_Rect rbi = dm_get_stage_reg_box();
	SDL_Rect b;
	b.w = vb.w;
	b.h = vb.h;
	b.x = rbi.x + rbi.w - b.w - b.w/5;
	b.y = rb.y + rb.h + b.h/4;
	return b;
}

/* Function: dm_get_stage_reg_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_stage_reg_box()
{
	SDL_Rect cb = dm_get_stage_code_box();
	SDL_Rect b;
	b.w = dm_scale_to_resolution(REG_BOX_W);
	b.h = dm_scale_to_resolution(REG_BOX_H);
	b.x = cb.x + cb.w;
	b.y = dm_scale_to_resolution(REG_BOX_Y);
	return b;
}

/* Function: dm_get_return_button_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_return_button_box()
{
	SDL_Rect b;
	b.w = dm_scale_to_resolution(RET_BUTTON_W);
	b.h = dm_scale_to_resolution(RET_BUTTON_H);
	b.x = SCREEN_BORDERS_OFS;
	b.y = g_screen_height - b.h - 2*dm_get_ofs_button_shadow();
	return b;
}

/* Function: dm_get_rst_btn_box
 * -----------------------------------------------------------------------------
 * Returns the box of the reset button of the stage
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_rst_btn_box()
{
	SDL_Rect ret_box = dm_get_return_button_box();
	SDL_Rect b;
	b.w = dm_scale_to_resolution(RST_BTN_W);
	b.h = dm_scale_to_resolution(RST_BTN_H);
	b.x = ret_box.x;
	b.y = ret_box.y - b.h - 2*dm_get_ofs_button_shadow();	
	return b;
}

/* Function: dm_get_h_bottom_msg
 * -----------------------------------------------------------------------------
 * Returns the h value for the click anywhere message
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset for the sel level buttons
 */
int dm_get_h_bottom_msg()
{
	int h = dm_scale_to_resolution(TEXT_H_BOTTOM_MSG);
	return h;
	
}

/* Function: dm_get_h_error_msg
 * -----------------------------------------------------------------------------
 * Returns the h value for the click anywhere message
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset for the sel level buttons
 */
int dm_get_h_error_msg()
{
	int h = dm_scale_to_resolution(TEXT_H_ERROR_MSG);
	return h;
	
}
/* Function: dm_get_w_msg
 * -----------------------------------------------------------------------------
 * Returns the width of the messages according to the border fill offset
 * A rectangle is provided to calculate the width using the correct value
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	The width of the message
 */
int dm_get_w_msg(SDL_Rect r)
{
	int w = r.w - 2*dm_scale_to_resolution(IFACE_FILLED_OFS);
	return w;
	
}

/* Function: dm_get_h_msg
 * -----------------------------------------------------------------------------
 * Returns the h value for the click anywhere message
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset for the sel level buttons
 */
int dm_get_h_msg()
{
	int h = dm_scale_to_resolution(TEXT_H_MSG);
	return h;
	
}
/* Function: dm_get_h_line_number
 * -----------------------------------------------------------------------------
 * Returns the h value for the line numberj
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the height for the line number
 */
int dm_get_h_code_text()
{
	int h = dm_scale_to_resolution(TEXT_H_CODE);
	return h;
}
/* Function: dm_get_big_msg_h
 * -----------------------------------------------------------------------------
 * Returns the h value for the big messages
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset for the sel level buttons
 */
int dm_get_h_big_text()
{
	int h = dm_scale_to_resolution(TEXT_H_BIG);
	return h;
}
/* Function: dm_get_sel_level_offset_y
 * -----------------------------------------------------------------------------
 * Returns the offset for the sel level buttons
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset for the sel level buttons
 */
int get_sel_level_offset_y()
{
	int y_offset =  dm_scale_to_resolution(SEL_LEVEL_BUTTON_H + 
										0.5*SEL_LEVEL_BUTTON_H);
	return y_offset;
	
}

/* Function: dm_get_level_button_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_level_button_box()
{
	SDL_Rect b;
	b.w = dm_scale_to_resolution(SEL_LEVEL_BUTTON_W);
	b.h = dm_scale_to_resolution(SEL_LEVEL_BUTTON_H);
	b.x = (g_screen_width - (5*b.w))/6;
	b.y = g_screen_height/6;
	return b;
}


/* Function: dm_get_upper_title_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object.
 */
SDL_Rect dm_get_upper_title_box()
{
	SDL_Rect b;
	b.h = dm_scale_to_resolution(TEXT_H_STAGE_TITLES);
	b.x = 0;
	b.y = g_screen_height/36;
	b.w = g_screen_width;
	return b;
}

/* Function: dm_get_avatar_wh
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_avatar_wh()
{
	SDL_Rect b;
	b.w = dm_scale_to_resolution(AVATAR_W);
	b.h = dm_scale_to_resolution(AVATAR_H);
	b.x = 0;
	b.y = 0;
	return b;
}

/* Function: dm_get_stage_btns
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_stage_btns()
{
	SDL_Rect b;
	SDL_Rect ib = dm_get_stage_instruction_box();
	int sh = dm_get_screen_height();
	int shadow = dm_get_ofs_button_shadow();
	b.w = dm_scale_to_resolution(STAGE_BUTTON_W);
	b.h = dm_scale_to_resolution(STAGE_BUTTON_H);
	b.x = ib.x + ib.w;
	b.y = sh - b.h - 2*shadow;
	return b;
}

/* Function: dm_get_stage_imm_up
 * -----------------------------------------------------------------------------
 * Returns the coordinates for the stage immediates for the upper part
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_stage_imm_up()
{
	SDL_Rect b;
	SDL_Rect cb = dm_get_stage_code_box();
	SDL_Rect vb = dm_get_value_box_wh();
	b.w = 0;
	b.h = dm_get_h_stage_elements_titles();
	b.x = dm_get_stage_ibox().x;
	b.y = dm_scale_to_resolution(IMM_TXT_UP_Y);
	return b;
}

/* Function: dm_get_code_button_wh
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_code_button_wh()
{
	SDL_Rect b;
	b.w = dm_scale_to_resolution(CODE_BUTTON_W);
	b.h = dm_scale_to_resolution(CODE_BUTTON_H);
	b.x = 0;
	b.y = 0;
	return b;
}

/* Function: dm_get_center_screen_box
 * -----------------------------------------------------------------------------
 * Returns the box of the center screen for menus
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_center_screen_box()
{
	SDL_Rect b;
	b.w = dm_scale_to_resolution(ESC_MENU_BOX_W);
	b.h = dm_scale_to_resolution(ESC_MENU_BOX_H);
	b.x = (g_screen_width - b.w)/2;
	b.y = (g_screen_height - b.h)/2;
	return b;
}

/* Function: dm_get_center_screen_box_text
 * -----------------------------------------------------------------------------
 * Box used for the text of the center box
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_center_screen_box_text()
{
	SDL_Rect rb = dm_get_center_screen_box();
	SDL_Rect b;
	b.w = rb.w - 2*dm_scale_to_resolution(IFACE_FILLED_OFS);
	b.h = dm_scale_to_resolution(TEXT_H_TOTAL_MSG);
	b.x = rb.x + (rb.w - b.w)/2;
	b.y = rb.y + dm_scale_to_resolution(IFACE_FILLED_OFS);
	return b;
}

/* Function: dm_get_escape_b1_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_escape_b1_box()
{
	SDL_Rect mb = dm_get_center_screen_box();

	SDL_Rect b;
	b.w = dm_scale_to_resolution(ESC_MENU_BUTTON_W);
	b.h = dm_scale_to_resolution(ESC_MENU_BUTTON_H);
	b.x = mb.x + mb.w/2 - b.w/2;
	b.y = mb.y + (mb.h - 3*b.h)/4 ;
	return b;
}

/* Function: dm_get_escape_b1_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_escape_b2_box()
{
	SDL_Rect mb = dm_get_center_screen_box();
	SDL_Rect b1 = dm_get_escape_b1_box();
	
	SDL_Rect b;
	b.w = dm_scale_to_resolution(ESC_MENU_BUTTON_W);
	b.h = dm_scale_to_resolution(ESC_MENU_BUTTON_H);
	b.x = b1.x;
	b.y = b1.y + (mb.h - 3*b.h)/4 + b.h;
	return b;
}

/* Function: dm_get_escape_b3_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_escape_b3_box()
{
	SDL_Rect mb = dm_get_center_screen_box();
	SDL_Rect b2 = dm_get_escape_b2_box();
	
	SDL_Rect b;
	b.w = dm_scale_to_resolution(ESC_MENU_BUTTON_W);
	b.h = dm_scale_to_resolution(ESC_MENU_BUTTON_H);
	b.x = b2.x;
	b.y = b2.y + (mb.h - 3*b.h)/4 + b.h;
	return b;
}

/* Function: dm_get_rst_b1_box
 * -----------------------------------------------------------------------------
 * Box position of the button 1 of the reset menu
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_rst_b1_box()
{
	SDL_Rect cs = dm_get_center_screen_box();

	SDL_Rect b;
	b.w = dm_scale_to_resolution(RST_MENU_BTNS_W);
	b.h = dm_scale_to_resolution(RST_MENU_BTNS_H);
	b.x = cs.x + cs.w/2 - 2*b.w;
	b.y = cs.y + cs.h/2 + b.h;
	return b;
}

/* Function: dm_get_rst_b2_box
 * -----------------------------------------------------------------------------
 * Box position of the button 2 of the reset menu
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_rst_b2_box()
{
	SDL_Rect cs = dm_get_center_screen_box();
	
	SDL_Rect b;
	b.w = dm_scale_to_resolution(RST_MENU_BTNS_W);
	b.h = dm_scale_to_resolution(RST_MENU_BTNS_H);
	b.x = cs.x + cs.w/2 + b.w;
	b.y = cs.y + cs.h/2 + b.h;
	return b;
}

/* Function: dm_get_p1_button_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_p1_button_box()
{
	SDL_Rect b;
	b.w = dm_scale_to_resolution(P_BUTTON_W);
	b.h = dm_scale_to_resolution(P_BUTTON_H);
	b.x = (g_screen_width - 3*b.w)/4;
	b.y = g_screen_height/3;
	return b;
}

/* Function: dm_get_p2_button_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_p2_button_box()
{
	SDL_Rect b;
	b.w = dm_scale_to_resolution(P_BUTTON_W);
	b.h = dm_scale_to_resolution(P_BUTTON_H);
	b.x = 2*(g_screen_width - 3*b.w)/4 + b.w;
	b.y = g_screen_height/3;
	return b;
}

/* Function: dm_get_p3_button_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_p3_button_box()
{
	SDL_Rect b;
	b.w = dm_scale_to_resolution(P_BUTTON_W);
	b.h = dm_scale_to_resolution(P_BUTTON_H);
	b.x = 3*(g_screen_width - 3*b.w)/4 + 2*b.w;
	b.y = g_screen_height/3;
	return b;
}
/* Function: dm_get_press_space_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object.
 */
SDL_Rect dm_get_press_space_box()
{
	SDL_Rect b;
	b.w = dm_scale_to_resolution(PRESS_W);
	b.x = (g_screen_width - b.w)/2;
	b.y = dm_scale_to_resolution(PRESS_Y);
	b.h = dm_scale_to_resolution(PRESS_H);
	return b;
}

/* Function: dm_get_studio_name_msg_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object.
 */
SDL_Rect dm_get_studio_name_msg_box()
{
	SDL_Rect b;
	b.w = dm_scale_to_resolution(STUDIO_W);
	b.h = dm_scale_to_resolution(STUDIO_H);
	b.x = 0;
	b.y = (g_screen_height - b.h)/2;
	return b;
}

/* Function: dm_get_game_title_img_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object.
 */
SDL_Rect dm_get_game_title_img_box()
{
	SDL_Rect b;
	b.w = dm_scale_to_resolution(TITLE_IMG_W);
	b.h = dm_scale_to_resolution(TITLE_IMG_H);
	b.x = (g_screen_width - b.w)/2;
	b.y = dm_scale_to_resolution(TITLE_IMG_Y);
	return b;
}

/* Function: dm_get_game_title_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object.
 */
SDL_Rect dm_get_game_title_box()
{
	SDL_Rect b;
	b.w = dm_scale_to_resolution(TITLE_W);
	b.x = (g_screen_width - b.w)/2;
	b.y = dm_scale_to_resolution(TITLE_Y);
	b.h = dm_scale_to_resolution(TITLE_H);
	return b;
}

/* Function: set_screen_resolution
 * -----------------------------------------------------------------------------
 * Sets the screen resolution that will be used in the game
 *
 * Arguments:
 *	resolution_id: The id of the resolution that will be used
 *
 * Return:
 *	Void.
 */
void dm_set_screen_resolution(int resolution_id)
{
	switch(resolution_id){
		case R1920X1080:
			g_screen_width = 1920;
			g_screen_height = 1080;
			g_res_id = R1920X1080;
			break;
		case R1600X900:
			g_screen_width = 1600;
			g_screen_height = 900;
			g_res_id = R1600X900;
			break;
		default:
			g_screen_width = 1600;
			g_screen_height = 900;
			g_res_id = R1600X900;
			break;
	}	
}

/* Function: scale_to_resoution
 * -----------------------------------------------------------------------------
 * Scales the given dimension according to the scree resolution. The baseline
 * resolution is 1600x900
 *
 * Arguments:
 *	dim: The dimension that will be scaled according to the resolution
 *
 * Return:
 *	Scale variable.
 */
int dm_scale_to_resolution(int dim)
{
	int scaled_dim;

	switch(g_res_id){
		case R1920X1080:
			scaled_dim = dim*1;
			break;

		case R1600X900:
			scaled_dim = dim*1;
			break;
		default:
			scaled_dim = dim*1;
			break;
	}
	return scaled_dim;
}


















